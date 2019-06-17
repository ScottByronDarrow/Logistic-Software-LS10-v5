/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: fe_invallc.c,v 5.14 2002/11/12 07:38:18 kaarlo Exp $
|  Program Name  : (fe_invallc.c)
|  Program Desc  : (Allocate/De-allocate Forward Exchange Contracts)
|                 (to/from Selected Invoices and Credit Notes and)
|                 (Update Invoice Exchange Rates Accordingly)
|---------------------------------------------------------------------|
|  Author        : Dirk Heinsius   | Date Written  : 18/08/94         |
|---------------------------------------------------------------------|
| $Log: fe_invallc.c,v $
| Revision 5.14  2002/11/12 07:38:18  kaarlo
| Updated to correct the computation on local_rec.value by checking first the envDbNettUsed.
|
| Revision 5.13  2002/09/06 01:19:51  scott
| S/C 4041 - There is misalignment in the report details.
|
| Revision 5.12  2002/07/24 08:38:51  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.11  2002/07/18 06:35:34  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.10  2002/07/17 08:42:30  scott
| S/C 004037
|
| Revision 5.9  2002/07/16 07:00:46  scott
| Updated from service calls and general maintenance.
|
| Revision 5.8  2002/07/16 02:41:52  scott
| Updated from service calls and general maintenance.
|
| Revision 5.7  2002/07/09 04:11:05  scott
| S/C 004041 - Lineup of report fixed.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: fe_invallc.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/FE/fe_invallc/fe_invallc.c,v 5.14 2002/11/12 07:38:18 kaarlo Exp $";

#define MAXWIDTH	150
#define MAXLINES	100
#define PAGELINES	65
#define	MOD  		1	/* alters frequency for dsp_process */

#include <pslscr.h>
#include <GlUtils.h>
#include <dsp_screen.h>
#include <dsp_process.h>
#include <twodec.h>
#include <ml_std_mess.h>
#include <ml_fe_mess.h>
#include <arralloc.h>

#define	BY_INVOICE	 (selectType [0] == 'I')
#define	BY_CURRENCY	 (selectType [0] == 'C')
#define	BY_ALL		 (selectType [0] == 'A')

#define	EXCH_OK		 (cuin_rec.er_fixed [0] != 'Y')

#define	CURR_OK 	 (!strcmp (cuin_rec.currency, pocrRec.code))

#define	INVOICE		 (cuin_rec.type [0] == '1')
#define	CREDIT 		 (cuin_rec.type [0] == '2')

#define	SR			store [line_cnt]
#define	SI			store [local_rec.storeIdx]

	/*
	 * Special fields and flags. 
	 */
   	int		processID		= 0,
   			found_data 		= 0,
   			printerNo 		= 1,
			envDbCo 		= 0,
			storeMax 		= 0,
			envDbFind 		= 0,
			envDbNettUsed 	= TRUE,
			envDbMcurr 		= FALSE,
			envFeInstall 	= FALSE,
			envGlByClass 	= TRUE;

	char	locCurr 	[4],
			dataStr 	[31],
	    	branchNo 	[3],
	    	selectType 	[2];

	char	*sptr;

   	double 	er_variance 	= 0.00,
   	       	exch_variance 	= 0.00,
   			exch_var_tot 	= 0.00,
   			old_doc_bal 	= 0.00,
			new_doc_bal 	= 0.00;

	FILE	*fin,
			*fout;

/*
 *	Structure for dynamic array.
 */
struct SortStruct
{
	char	sortCode [MAXLEVEL + 1];
	double	exchVariance;
}	*sortRec;
	DArray sortDetails;
	int	sortCnt = 0;

#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct esmrRecord	esmr_rec;
struct crbkRecord	crbk_rec;
struct cumrRecord	cumr_rec;
struct cuinRecord	cuin_rec;
struct wfbcRecord	wfbc_rec;
struct fehrRecord	fehr_rec;
struct fehrRecord	fehr2_rec;
struct felnRecord	feln_rec;
struct fetrRecord	fetr_rec;
struct cudtRecord	cudt_rec;

POCR_STRUCT	pocr2Rec;

	char	*data  = "data",
			*cuin2 = "cuin2",
			*fehr2 = "fehr2";

	struct	storeRec 
	{
		char	branchNo [3];	/* Branch Number 						*/
		char	invoiceNo [9];	/* Invoice Number						*/
		double	value;			/* Invoice Value						*/
		int		assign;			/* 0 = Assign, 1 = Unassign				*/
		long	hhciHash;		/* Link to cuin record					*/
		long	hhfeHash;		/* Link to ORIGINAL fehr for invoice	*/
	} store [MAXLINES];


/*============================
| Local & Screen Structures. |
============================*/
struct {
	char 	dummy [11];
	int		storeIdx;
	char 	branchNo [3];
	char 	invoiceNo [sizeof cuin_rec.inv_no];
	double	balance;
	double	value;
	double	invoiceBalance;
	double	oldRate;
	double	newRate;
	char 	systemDate [11];
	long	lsystemDate;
} local_rec;

extern	int	TruePosition;
static	struct	var	vars [] =
{
	{1, LIN, "currCode",	4, 2, CHARTYPE,
		"UUU", "          ",
		" ", " ", "Currency Code   ", "Enter Currency, [SEARCH] available.",
		NE, NO,  JUSTLEFT, "", "", pocrRec.code},
	{1, LIN, "currDesc",	4, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", pocrRec.description},
	{1, LIN, "cont_code",	 5, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", "A", "Contract No.    ", "Enter Contract No., [SEARCH] available.",
		 NE, NO,  JUSTLEFT, "", "", fehr_rec.cont_no},
	{1, LIN, "balance",	 6, 2, MONEYTYPE,
		"NNNNNNNNNNN.NN", "          ",
		" ", "",  "Balance         ", " ",
		 NA, NO,  JUSTRIGHT, "", "", (char *)&local_rec.balance},

	{2, TAB, "branchNo",	 MAXLINES, 1, CHARTYPE,
		"NN", "          ",
		" ", comm_rec.est_no, "Br No", "Enter Branch number.",
		 NE, NO, JUSTRIGHT, "", "", local_rec.branchNo},
	{2, TAB, "invoiceNo",	 0, 0, CHARTYPE,
		"UUUUUUUU", "          ",
		" ", "      ", "  Invoice Number.  ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.invoiceNo},
	{2, TAB, "invoiceAmt",	 0, 0, MONEYTYPE,
		"NNNNNNNNNNN.NN", "          ",
		" ", "0", "  Invoice Value  ", " ",
		 NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.value},
	{2, TAB, "store_idx",	 0, 0, INTTYPE,
		"NNNN", "          ",
		" ", "-1", "", " ",
		 ND, NO, JUSTRIGHT, "", "", (char *)&local_rec.storeIdx},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*======================
| Function Prototypes. |
======================*/
int 	ChkInvNo 			(char *);
int 	DeleteLine 			(int);
int 	GetGlmr 			(char *);
int 	heading 			(int);
int 	LoadInvoices 		(void);
int 	spec_valid 			(int);
int		SortFunc			(const	void *,	const void *);
void 	CloseDB 			(void);
void 	Heading1Output 		(void);
void 	Heading2Output 		(void);
void 	OpenDB 				(void);
void 	PrintControlAcc 	(char *, double, char *);
void 	ProcessInvoice 		(int);
void 	ProcessInvTran 		(void);
void 	ProcessSortedList 	(void);
void 	shutdown_prog 		(void);
void 	SrchCuin 			(char *);
void 	SrchEsmr 			(void);
void 	SrchFehr 			(char *);
void 	StoreAccount 		(void);
void 	UpdateWfbc 			(void);
void 	WriteGlTrans 		(char *, double, char *);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
	int		argc, 
	char 	*argv [])
{
	int		i;
	char	*sptr;

	if (argc != 3)
	{
		print_at (0,0, ML (mlFeMess017), argv [0]);
		return (EXIT_FAILURE);
	}
	printerNo  	= atoi (argv [1]);
	processID   = atoi (argv [2]);

	sptr = chk_env ("FE_INSTALL");
	envFeInstall = (sptr == (char *) 0) ? FALSE : atoi (sptr);
	
	sptr = chk_env ("DB_MCURR");
	envDbMcurr = (sptr == (char *) 0) ? FALSE : atoi (sptr);
	
	sptr = chk_env ("DB_NETT_USED");
	envDbNettUsed = (sptr == (char *) 0) ? TRUE : atoi (sptr);

	sptr = chk_env ("GL_BYCLASS");
	envGlByClass = (sptr == (char *)0) ? TRUE : atoi (sptr);

	init_scr ();

	if (!envDbMcurr) 
	{
		no_option ("DB_MCURR (Multi Currency Customer.)");
		return (EXIT_FAILURE);
	}

	if (!envFeInstall) 
	{
		no_option ("FE_INSTALL (Forward Exchange Contracts.)");
		return (EXIT_FAILURE);
	}

	tab_col = 20;
	tab_row = 8;

	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	SETUP_SCR (vars);

	TruePosition	=	TRUE;


	set_tty 	();
	set_masks 	();
/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (2, store, sizeof (struct storeRec));
#endif
	init_vars 	(1);

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));
	local_rec.lsystemDate = TodaysDate ();

	envDbCo 	= atoi (get_env ("DB_CO"));
	envDbFind  	= atoi (get_env ("DB_FIND"));

	OpenDB ();

	strcpy (branchNo, (envDbCo) ? comm_rec.est_no : " 0");

	while (prog_exit == 0) 
	{
		abc_unlock (fehr);
		abc_unlock (cuin);
		prog_status = ENTRY;
		entry_exit	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;
		init_vars (1);
		init_vars (2);
		lcount [2] 	= 0;

		/*------------------
		| Clear Store Array |
		-------------------*/
		for (i = 0; i < MAXLINES; i++)
		{
			strcpy (store [i].branchNo, "  ");
			strcpy (store [i].invoiceNo, "        ");
			store [i].value = 0.00;
			store [i].assign = FALSE;
			store [i].hhciHash = 0L;
			store [i].hhfeHash = 0L;
			storeMax = 0;
		}
		/*------------------------------
		| Enter screen 1 linear input. |
		------------------------------*/
		heading (1);
		entry (1);
		if (prog_exit || restart) 
			continue;

		cc = LoadInvoices ();
		if (!cc)
		{
			heading (2);
			scn_display (2);
			edit (2);
		}
		else
		{
			heading (2);
			entry (2);
		}

		if (restart) 
			continue;

		edit_all ();
		if (restart) 
			continue;

		/*--------------------------------
		| Update invoice & glwk records. |
		--------------------------------*/
		ProcessInvTran ();
	}	
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*=================================
| Load any invoices for contract. |
=================================*/
int
LoadInvoices (void)
{
	scn_set (2);
	line_cnt 	= 0;
	lcount [2] 	= 0;
	storeMax	= 0;

	abc_selfield (feln, "feln_hhfe_hash");
	feln_rec.hhfe_hash = fehr_rec.hhfe_hash;
	cc = find_rec (feln, &feln_rec, GTEQ, "r");
	while (!cc && feln_rec.hhfe_hash == fehr_rec.hhfe_hash)
	{
		if (feln_rec.index_by [0] == 'C')
		{
			cuin_rec.hhci_hash = feln_rec.index_hash;
			cc = find_rec (cuin2, &cuin_rec, EQUAL, "r");
			if (!cc)
			{
				strcpy (SR.branchNo, cuin_rec.est);
				strcpy (SR.invoiceNo, cuin_rec.inv_no);
//				SR.value 		= cuin_rec.amt;
				SR.value 		= (envDbNettUsed) ? cuin_rec.amt - cuin_rec.disc 
												  : cuin_rec.amt;
				SR.assign 		= TRUE;
				SR.hhciHash 	= feln_rec.index_hash;
				SR.hhfeHash 	= feln_rec.hhfe_hash;
//				local_rec.value = cuin_rec.amt;
				local_rec.value = (envDbNettUsed) ? cuin_rec.amt - cuin_rec.disc 
												  : cuin_rec.amt;
				local_rec.storeIdx = line_cnt;
				strcpy (local_rec.branchNo, cuin_rec.est);
				strcpy (local_rec.invoiceNo, cuin_rec.inv_no);
				putval (line_cnt++);
				lcount [2]++;
				storeMax++;
			}
		}
		local_rec.balance -= feln_rec.value;
		
		cc = find_rec (feln, &feln_rec, NEXT, "r");
	}
	abc_selfield (feln, "feln_id_no");

	fetr_rec.hhfe_hash = fehr_rec.hhfe_hash;
	cc = find_rec (fetr, &fetr_rec, GTEQ, "r");
	while (!cc && fetr_rec.hhfe_hash == fehr_rec.hhfe_hash)
	{
		local_rec.balance -= fetr_rec.value;
		cc = find_rec (fetr, &fetr_rec, NEXT, "r");
	}
	scn_set (1);
	if (lcount [2] > 0)
		return (EXIT_SUCCESS);
	return (EXIT_FAILURE);
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
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (comr,  comr_list, COMR_NO_FIELDS, "comr_co_no");
	strcpy (comr_rec.co_no, comm_rec.co_no);
	cc = find_rec (comr, &comr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, comr, "DBFIND");

	if (!strcmp (comr_rec.base_curr, "   "))
		sprintf (locCurr, "%-3.3s", get_env ("CURR_CODE"));
	else
		sprintf (locCurr, "%-3.3s", comr_rec.base_curr);

	abc_fclose (comr);

	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");

	open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (crbk, crbk_list, CRBK_NO_FIELDS, "crbk_id_no");
	open_rec (cudt, cudt_list, CUDT_NO_FIELDS, "cudt_hhci_hash");
	open_rec (cuin, cuin_list, CUIN_NO_FIELDS, "cuin_id_no3");
	open_rec (wfbc, wfbc_list, WFBC_NO_FIELDS, "wfbc_id_no");
	open_rec (fehr, fehr_list, FEHR_NO_FIELDS, "fehr_id_no");
	open_rec (feln, feln_list, FELN_NO_FIELDS, "feln_id_no");
	open_rec (fetr, fetr_list, FETR_NO_FIELDS, "fetr_hhfe_hash");

	abc_alias (cuin2, cuin);
	open_rec (cuin2, cuin_list, CUIN_NO_FIELDS, "cuin_hhci_hash");
	abc_alias (fehr2, fehr);
	open_rec (fehr2, fehr_list, FEHR_NO_FIELDS, "fehr_hhfe_hash");

	OpenPocr ();
	OpenGlmr ();
	GL_OpenBatch (comm_rec.co_no, comm_rec.est_no);
}



/*========================
| Close data base files. |
========================*/
void
CloseDB (
 void)
{
	abc_fclose (cumr);
	abc_fclose (esmr);
	abc_fclose (crbk);
	abc_fclose (cudt);
	abc_fclose (cuin);
	abc_fclose (wfbc);
	abc_fclose (fehr);
	abc_fclose (feln);
	abc_fclose (fetr);

	abc_fclose (cuin2);
	abc_fclose (fehr2);

	GL_CloseBatch (printerNo);
	GL_Close ();
	abc_dbclose (data);

}

/*============================
| Read gl account master.    |
============================*/
int
GetGlmr (
	char	*account)
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
 int field)
{
	int		i;

	/*--------------------------------------------------
	| Validate Forward Exchange Contract Number Input. |
	--------------------------------------------------*/
	if (LCHECK ("cont_code"))
	{
		if (SRCH_KEY)
		{
			SrchFehr (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (fehr_rec.co_no, comm_rec.co_no);
		cc = find_rec (fehr, &fehr_rec, EQUAL, "w");
		if (cc) 
		{
			print_mess (ML (mlStdMess075));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE); 
		}

		if (strcmp (fehr_rec.curr_code, pocrRec.code))
		{
			/*--------------------------------------------------
			| Contract %s is on file for a diffenent currency. |
			--------------------------------------------------*/
			sprintf (err_str, ML (mlFeMess005), fehr_rec.cont_no);
			print_mess (err_str);
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}

		if (fehr_rec.stat_flag [0] != 'A')
		{
			/*------------------------------------------
			| Forward Exchange Contract is not active. |
			------------------------------------------*/
			print_mess (ML (mlFeMess006));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
	
		/*-----------------------------------------
		| now see if contract is still current.    
		------------------------------------------*/
		if (fehr_rec.date_exp < local_rec.lsystemDate)
		{
			sprintf (err_str,ML (mlFeMess007),DateToString (fehr_rec.date_exp));
			print_mess (err_str);
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
	
		if (fehr_rec.date_wef > local_rec.lsystemDate)
		{
			/*-----------------------------------------
			| Contract Not Yet Current - Effective %s |
			-----------------------------------------*/
			sprintf (err_str,ML (mlFeMess008),DateToString (fehr_rec.date_wef));
			print_mess (err_str);
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		strcpy (crbk_rec.co_no,   comm_rec.co_no);
		strcpy (crbk_rec.bank_id, fehr_rec.bank_id);
		cc = find_rec (crbk, &crbk_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess010)); 
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		strcpy (pocr2Rec.co_no, comm_rec.co_no);
		strcpy (pocr2Rec.code,  crbk_rec.curr_code);
		cc = find_rec (pocr, &pocr2Rec, EQUAL, "r");
		if (cc)
			file_err (cc, pocr, "DBFIND");
		
		local_rec.balance = fehr_rec.val_orig;
		local_rec.newRate = fehr_rec.exch_rate * pocr2Rec.ex1_factor;

		return (EXIT_SUCCESS);
	}

	/*-------------------------------
	| Validate Branch Number Input. |
	-------------------------------*/
	if (LCHECK ("branchNo"))
	{
		if (FLD ("branchNo") == NA || FLD ("branchNo") == ND)
		{
			strcpy (local_rec.branchNo, comm_rec.est_no);
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchEsmr ();
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
			strcpy (local_rec.branchNo, comm_rec.est_no);

		strcpy (esmr_rec.co_no, comm_rec.co_no);
		strcpy (esmr_rec.est_no, local_rec.branchNo);
	
		cc = find_rec (esmr, &esmr_rec, EQUAL, "r");
		if (cc) 
		{
			/*---------------------------
			| Branch %s is not on File. |
			---------------------------*/
			print_mess (ML (mlStdMess073));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE); 
		}
		return (EXIT_SUCCESS);
	}

	/*--------------------------------
	| Validate Invoice Number Input. |
	--------------------------------*/
	if (LCHECK ("invoiceNo"))
	{
		if (dflt_used || last_char == DELLINE)
		{
			if (prog_status == ENTRY)
			{
				print_mess (ML (mlStdMess005));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		
			cc = DeleteLine (TRUE);	
			if (!cc)
			{
				scn_set (1);
				DSP_FLD ("balance");
				scn_set (2);
			}
			return (EXIT_SUCCESS);
		}

		if (line_cnt < lcount [2] && strcmp (local_rec.invoiceNo, SI.invoiceNo))
		{
			/*------------------------------------
			| Invoice numbers cannot be changed. |
			------------------------------------*/
			print_mess (ML (mlFeMess009));
			sleep (sleepTime);
			strcpy (local_rec.invoiceNo, SI.invoiceNo);
			return (EXIT_SUCCESS); 
		}

		if (SRCH_KEY)
		{
			SrchCuin (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (cuin_rec.co_no,  comm_rec.co_no);
		strcpy (cuin_rec.est,    local_rec.branchNo);
		strcpy (cuin_rec.inv_no, local_rec.invoiceNo);
		cc = find_rec (cuin, &cuin_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess115));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}

		if (strcmp (cuin_rec.currency, fehr_rec.curr_code))
		{
			/*-------------------------------------------------
			| Invoice %s is on file for a different currency. |
			-------------------------------------------------*/
			sprintf (err_str, ML (mlFeMess010), cuin_rec.inv_no);
			print_mess (err_str);
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}

		cudt_rec.hhci_hash = cuin_rec.hhci_hash;
		cc = find_rec (cudt, &cudt_rec, EQUAL, "r");
		if (!cc)
		{
			/*--------------------------------------------------
			| Invoice %s has already been paid or partly paid. |
			--------------------------------------------------*/
			sprintf (err_str, ML (mlFeMess011), cuin_rec.inv_no);
			print_mess (err_str);
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}

		local_rec.value = (envDbNettUsed) ? cuin_rec.amt - cuin_rec.disc 
										  : cuin_rec.amt;
		if (local_rec.value == 0.00)
		{
			sprintf (err_str, ML (mlFeMess012), cuin_rec.inv_no);
			print_mess (err_str);
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}

		if (local_rec.value > local_rec.balance)
		{
			/*-------------------------------------------------------------
			| Invoice amount is greater than amount remaining on contract.|
			-------------------------------------------------------------*/
			print_mess (ML (mlFeMess013));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE); 
		}

		i = ChkInvNo (local_rec.invoiceNo);

		if (i < storeMax)
		{
			if (store [i].assign)
			{
				/*----------------------------------------------
				| Invoice is already assigned to this contract.|
				----------------------------------------------*/
				print_mess (ML (mlFeMess014));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE); 
			}
			local_rec.storeIdx = i;
		}
		else
			local_rec.storeIdx = storeMax++;

		strcpy (feln_rec.index_by, "C");
		feln_rec.index_hash = cuin_rec.hhci_hash;
		cc = find_rec (feln, &feln_rec, EQUAL, "r");
		if (!cc)
		{
			fehr2_rec.hhfe_hash = feln_rec.hhfe_hash;
			cc = find_rec (fehr2, &fehr2_rec, EQUAL, "r");
			if (cc)
				file_err (cc, fehr2, "DBFIND");

			if (fehr2_rec.hhfe_hash != fehr_rec.hhfe_hash)
			{
				i = prmptmsg (ML (mlFeMess018), "YyNn", 1, 2);
				print_at (2, 0, "%80.80s", " ");
				if (i != 'y' && i != 'Y')
					return (EXIT_FAILURE);
			}
			SI.hhfeHash = feln_rec.hhfe_hash;
		}
		else
		{
			if (!EXCH_OK)
			{
				print_mess (ML (mlFeMess016));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE); 
			}
			SI.hhfeHash = 0L;
		}

		DSP_FLD ("branchNo");
		DSP_FLD ("invoiceNo");
		DSP_FLD ("invoiceAmt");
		
		local_rec.oldRate = cuin_rec.exch_rate;
		local_rec.balance -= local_rec.value;
		scn_set (1);
		DSP_FLD ("balance");
		scn_set (2);
		strcpy (selectType, "I");
		strcpy (SI.branchNo, local_rec.branchNo);
		strcpy (SI.invoiceNo, local_rec.invoiceNo);
		SI.value     = local_rec.value;
		SI.assign    = TRUE;
		SI.hhciHash = cuin_rec.hhci_hash;

		return (EXIT_SUCCESS);
	}

	/*-------------------------------
	| Validate Currency Code.       |
	-------------------------------*/
	if (LCHECK ("currCode"))
	{
		if (SRCH_KEY)
		{
        	SearchPocr (comm_rec.co_no, temp_str);
			return (EXIT_SUCCESS);
		}

		/*-----------------------
		| Read Currency Record. |
		-----------------------*/
		strcpy (pocrRec.co_no, comm_rec.co_no);
		cc = find_rec (pocr, &pocrRec, EQUAL, "r");
		if (cc) 
		{
			/*----------------------------
			| Currency %s is not on file.|
			----------------------------*/
			print_mess (ML (mlStdMess040));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE); 
		}

		DSP_FLD ("currDesc");
	   	return (EXIT_SUCCESS);
	}
    return (EXIT_SUCCESS);
}


int
ChkInvNo (
 char *invoiceNo)
{
	int		i;

	i = 0;
	while (i < storeMax)
	{
		if (!strcmp (invoiceNo, store [i].invoiceNo))
			break;
		else
			i++;
	}
	return (i);
}


/*==========================
| Search for branch number |
==========================*/
void
SrchEsmr (
 void)
{
	_work_open (2,0,40);
	strcpy (esmr_rec.co_no,  comm_rec.co_no);
	sprintf (esmr_rec.est_no, "%-2.2s", " ");
	save_rec ("#No","#Branch number description.");
	cc = find_rec (esmr, &esmr_rec, GTEQ, "r");
    while (!cc 
	&&     !strcmp (esmr_rec.co_no, comm_rec.co_no))
   	{                        
        cc = save_rec (esmr_rec.est_no, esmr_rec.est_name);
		if (cc)
			break;
		cc = find_rec (esmr, &esmr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (esmr_rec.co_no,  comm_rec.co_no);
	sprintf (esmr_rec.est_no, "%-2.2s", temp_str);
	cc = find_rec (esmr, &esmr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, esmr, "DBFIND");
}

/*================
| Search for FEC |
================*/
void
SrchFehr (
 char *key_val)
{
    _work_open (3,0,40);
	save_rec ("#No ","#Bank currency description.");                       
	strcpy (fehr_rec.co_no,   comm_rec.co_no);
	sprintf (fehr_rec.cont_no, "%-3.3s", key_val);
	cc = find_rec (fehr, &fehr_rec, GTEQ, "r");
    while (!cc 
	&&     !strcmp (fehr_rec.co_no, comm_rec.co_no) 
	&&     !strncmp (fehr_rec.cont_no, key_val, strlen (key_val)))
    {
		if (!strcmp (fehr_rec.curr_code, pocrRec.code))
		{
			sprintf (err_str, "%-5.5s   %-3.3s", fehr_rec.bank_id, 
												 fehr_rec.curr_code);
		    cc = save_rec (fehr_rec.cont_no, err_str);                       
			if (cc)
				break;
		}
		cc = find_rec (fehr, &fehr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (fehr_rec.co_no,   comm_rec.co_no);
	sprintf (fehr_rec.cont_no, "%-6.6s", temp_str);
	cc = find_rec (fehr, &fehr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, fehr, "DBFIND");
}
/*===============================================
| Search routine for supplier invoice file.     |
===============================================*/
void
SrchCuin (
 char *key_val)
{
	char	disp_amt [51];

	work_open ();
	strcpy (cuin_rec.co_no,  comm_rec.co_no);
	strcpy (cuin_rec.est,    local_rec.branchNo);
	strcpy (cuin_rec.inv_no, key_val);
	save_rec ("#Invoice","#    Amount.           ");
	cc = find_rec (cuin, &cuin_rec, GTEQ, "r");
	while (!cc 
	&&     !strcmp (cuin_rec.co_no,  comm_rec.co_no)
	&&     !strcmp (cuin_rec.est,    local_rec.branchNo)
	&&     !strncmp (cuin_rec.inv_no, key_val, strlen (key_val)))
	{
		if (INVOICE 
		&& !strcmp (cuin_rec.currency, fehr_rec.curr_code))
		{
			local_rec.invoiceBalance = (envDbNettUsed) 
					? cuin_rec.amt - cuin_rec.disc : cuin_rec.amt;

			if (local_rec.invoiceBalance != 0.00)
			{
				sprintf (disp_amt, " %-14.2f ",
						DOLLARS (local_rec.invoiceBalance));
	
				cc = save_rec (cuin_rec.inv_no, disp_amt);
				if (cc)
					break;
			}
		}
		cc = find_rec (cuin, &cuin_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (cuin_rec.est,    local_rec.branchNo);
	strcpy (cuin_rec.inv_no, temp_str);
	strcpy (cuin_rec.co_no,  comm_rec.co_no);
	cc = find_rec (cuin, &cuin_rec, EQUAL, "r");
	if (cc)
		file_err (cc, cuin, "DBFIND");
}


/*================================
| Update Invoice & GL Work File. |
================================*/
void
ProcessInvTran (void)
{
	int		idx;
	/*----------------------------------------
	| Start audit report & open sort file.   |
	----------------------------------------*/

	Heading1Output ();
	/*
	 * Allocate the initial array.
	 */
	ArrAlloc (&sortDetails, &sortRec, sizeof (struct SortStruct), 10);
	sortCnt = 0;

	for (idx = 0; idx < storeMax; idx++)
	{
		/*----------------------------------------
		| Update selection on inv/cn document.   |
		----------------------------------------*/
		strcpy (cuin_rec.co_no,  comm_rec.co_no);
		strcpy (cuin_rec.est,    store [idx].branchNo);
		strcpy (cuin_rec.inv_no, store [idx].invoiceNo);
		cc = find_rec (cuin, &cuin_rec, EQUAL, "u");
		if (cc)
		{
			sprintf (err_str,"DBFIND [%d][%s][%s]", idx, store [idx].branchNo, store [idx].invoiceNo);
			file_err (cc, cuin, err_str);
		}

		cumr_rec.hhcu_hash = cuin_rec.hhcu_hash;
		cc = find_rec (cumr, &cumr_rec, EQUAL, "r");
		if (cc)
			file_err (cc, cumr, "DBFIND");

		if (CURR_OK)
			ProcessInvoice (idx);
	}

	cc = abc_update (fehr, &fehr_rec);
	if (cc)
		file_err (cc, fehr, "DBUPDATE");

	fprintf (fout, ".EOF\n");
	pclose (fout);

	if (sortCnt != 0)
	{
		ProcessSortedList ();
		UpdateWfbc ();
	}
}

/*=====================================
| Process Invoice / Cn Document.      |
=====================================*/
void
ProcessInvoice (
	int		idx)
{
	double	sav_rate;
	double	dbcr_value = 0.00;

	sav_rate = local_rec.newRate;

	if (store [idx].assign == FALSE)
	{
		if (store [idx].hhfeHash == fehr_rec.hhfe_hash)
			local_rec.newRate = pocrRec.ex1_factor;
		else
		{
			abc_unlock (cuin);
			local_rec.newRate = sav_rate;
			return;
		}
	}

	if (local_rec.newRate == 0.00 || cuin_rec.exch_rate == 0.00)
	{
		er_variance = 0.00;	
	}
	else
	{
		er_variance = (1.0 / local_rec.newRate) - (1.0 / cuin_rec.exch_rate);
	}

	local_rec.oldRate = psl_round (cuin_rec.exch_rate,8);
	new_doc_bal = (envDbNettUsed) ? cuin_rec.amt - cuin_rec.disc
				            	  : cuin_rec.amt;

	exch_variance = er_variance * new_doc_bal;
	exch_variance = exch_variance;

	if (store [idx].assign)
	{
		if (store [idx].hhfeHash != fehr_rec.hhfe_hash)
		{
			if (store [idx].hhfeHash > 0)
			{
				fehr2_rec.hhfe_hash = feln_rec.hhfe_hash;
				cc = find_rec (fehr2, &fehr2_rec, EQUAL, "u");
				if (cc)
					file_err (cc, fehr2, "DBFIND");
	
				strcpy (feln_rec.index_by, "C");
				feln_rec.index_hash = cuin_rec.hhci_hash;
				cc = find_rec (feln, &feln_rec, EQUAL, "u");
				if (!cc)
				{
					fehr2_rec.val_avail += feln_rec.value;
					cc = abc_delete (feln);
					if (cc)
						file_err (cc, feln, "DBDELETE");
					cc = abc_update (fehr2, &fehr2_rec);
					if (cc)
						file_err (cc, fehr2, "DBUPDATE");
				}
				else
					abc_unlock (fehr2);
			}
			feln_rec.hhfe_hash = fehr_rec.hhfe_hash;
			strcpy (feln_rec.index_by, "C");
			feln_rec.index_hash = cuin_rec.hhci_hash;
			feln_rec.value = store [idx].value;
			cc = abc_add (feln, &feln_rec);
			if (cc)
				file_err (cc, feln, "DBADD");

			fehr_rec.val_avail -= store [idx].value;
		}
	}
	else
	{
		if (store [idx].hhfeHash == fehr_rec.hhfe_hash)
		{
			strcpy (cuin_rec.er_fixed, "N");
			strcpy (feln_rec.index_by, "C");
			feln_rec.index_hash = cuin_rec.hhci_hash;
			cc = find_rec (feln, &feln_rec, EQUAL, "u");
			if (!cc)
			{
				fehr_rec.val_avail += feln_rec.value;
				cc = abc_delete (feln);
				if (cc)
					file_err (cc, feln, "DBDELETE");
			}
		}
	}

	if (exch_variance == 0.0)
	{
		abc_unlock (cuin);
		local_rec.newRate = sav_rate;
		return;
	}

	/*--------------------
	| Update Inv/Cn      |
	--------------------*/
	cuin_rec.exch_rate = local_rec.newRate;
	cc = abc_update (cuin, &cuin_rec);
	if (cc)
		file_err (cc, cuin, "DBUPDATE");

	/*--------------------
	| Store In Sort File |
	--------------------*/
	StoreAccount ();

	fprintf (fout, "| %s | %s|%s| %14.14s ",
			 cumr_rec.dbt_no,
			 cumr_rec.dbt_name,
			 cuin_rec.inv_no,
			 comma_fmt (DOLLARS (new_doc_bal), "NNN,NNN,NNN.NN"));

	fprintf (fout, "|%11.8f|%11.8f|", local_rec.oldRate, local_rec.newRate);

	fprintf (fout, "%14.14s ", comma_fmt (DOLLARS (exch_variance),
										  "NNN,NNN,NNN.NN"));

	/*--------------------
	| Credit Value       |
	--------------------*/
	if (exch_variance < 0)
	{
		dbcr_value = exch_variance * -1;
		dbcr_value = dbcr_value;
		fprintf (fout, "|               |%14.14s |\n",
			    comma_fmt (DOLLARS (dbcr_value), "NNN,NNN,NNN.NN"));
	}
	/*--------------------
	| Debit Value        |
	--------------------*/
	else
	{
		fprintf (fout, "|%14.14s |               |\n",
				comma_fmt (DOLLARS (exch_variance), "NNN,NNN,NNN.NN"));
	}
	local_rec.newRate = sav_rate;
}


/*====================================
| Store Account Detail In Sort File. |
====================================*/
void
StoreAccount (void)
{
	char	account_no [MAXLEVEL + 1];

	sprintf (account_no,"%-*.*s", MAXLEVEL,MAXLEVEL,cumr_rec.gl_ctrl_acct);
	if (GetGlmr (account_no))
	{
		sprintf (account_no, "%-*.*s",MAXLEVEL,MAXLEVEL, 
												pocrRec.gl_exch_var);
		if (GetGlmr (account_no)) 
		{
			GL_GLI 
			(
				comm_rec.co_no,
				comm_rec.est_no,	
				"  ",			
				"SUSPENSE  ",
				(envGlByClass) ? cumr_rec.class_type : cumr_rec.sman_code,
				" "
			);
			strcpy (account_no, glmrRec.acc_no);
		}
		if (GetGlmr (account_no)) 
			return;
	}
	/*
	 * Check the array size before adding new element.
	 */
	if (!ArrChkLimit (&sortDetails, sortRec, sortCnt))
		sys_err ("ArrChkLimit (sortRec)", ENOMEM, PNAME);

	/*
	 * Load values into array element sortCnt.
	 */
	sprintf 
	(
		sortRec [sortCnt].sortCode, 
		"%-*.*s", 
		MAXLEVEL, 
		MAXLEVEL, 
		account_no
	);
	sortRec [sortCnt].exchVariance	= DOLLARS (exch_variance);
	/*
	 * Increment array counter.
	 */
	sortCnt++;
}

/*========================================
| Process Account Detail From Sort File. |
========================================*/
void
ProcessSortedList (void)
{
	char	previousAcc	[MAXLEVEL + 1];
	char	currentAcc 	[MAXLEVEL + 1];
	double	cur_val = 0.00;
	double	acctTotal = 0.00;
	int		first_record = TRUE;
	int		i;

	Heading2Output ();

	exch_var_tot = 0.0;

	/*
	 * Sort the array in item description order.
	 */
	qsort (sortRec, sortCnt, sizeof (struct SortStruct), SortFunc);
	
	for (i = 0; i < sortCnt; i++)
	{
		strcpy (currentAcc, sortRec [i].sortCode);
		cur_val = sortRec [i].exchVariance;
		if (first_record == TRUE)
		{
			first_record = FALSE;
			sprintf (previousAcc, "%-*.*s", MAXLEVEL,MAXLEVEL,currentAcc);
			sprintf (temp_str, ML (mlFeMess019), currentAcc);
			us_pr (temp_str,20,18,1);
		}

		if (strncmp (previousAcc, currentAcc, MAXLEVEL))
		{
			PrintControlAcc (previousAcc,acctTotal,"Ctrl Acct");
			WriteGlTrans (previousAcc,acctTotal,"Ctrl Acct");
			exch_var_tot -= acctTotal;
			acctTotal = cur_val;
			strcpy (previousAcc,currentAcc);

			sprintf (temp_str,"Processing Account Summary: %s", currentAcc);
			us_pr (temp_str,20,18,1);
		}
		else
			acctTotal += cur_val;
	}
	PrintControlAcc (previousAcc, acctTotal, " Control Account.   ");
	WriteGlTrans (previousAcc, acctTotal, " Control Account.   ");

	exch_var_tot -= acctTotal;

	PrintControlAcc (pocrRec.gl_exch_var, exch_var_tot, "Exchange variation ");
	WriteGlTrans (pocrRec.gl_exch_var, exch_var_tot, "Exchange variation ");

	/*
	 *	Free up the array memory
	 */
	ArrDelete (&sortDetails);
	
	fprintf (fout, ".EOF\n");
	pclose (fout);
}

/*==========================================
| Print Summarised Control Account Detail. |
==========================================*/
void
PrintControlAcc (
	char   *acct, 
	double value, 
	char   *type)
{
	double	dbcr_value;

	if (!value) return;

	fprintf (fout, "| %20.20s |  %-16.16s  ", type, acct);
				
	/*-------------------
	| Debit Value       |
	-------------------*/
	if (value > 0)
	{
		fprintf (fout, "|%14.14s |               |\n",
			comma_fmt (value, "NNN,NNN,NNN.NN"));
	}
	/*---------------------
	| Credit Value        |
	---------------------*/
	else
	{
		dbcr_value = value * -1;
		dbcr_value = dbcr_value;
		fprintf (fout, "|               |%14.14s |\n",
			comma_fmt (dbcr_value, "NNN,NNN,NNN.NN"));
	}
}


/*===========================
| Write glwk total records. |
===========================*/
void
WriteGlTrans (
 char   *acct, 
 double value, 
 char   *type)
{
	int		monthPeriod;

	if (value == 0.00) return;

	/*-------------------------------------------------
	| Post control total debits & credits .           |
	-------------------------------------------------*/
	strcpy (glwkRec.jnl_type, (value > 0.00) ? "1" : "2");
	glwkRec.amount = (value > 0.00) 
					? CENTS (value) : CENTS (value * -1);

	glwkRec.loc_amount 	= 	glwkRec.amount;
	glwkRec.exch_rate 	=	1.00;
	strcpy (glwkRec.currency, locCurr);

	strcpy (glmrRec.co_no,comm_rec.co_no);
	sprintf (glmrRec.acc_no, "%-16.16s", acct);
	cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
	if (cc)
		file_err (cc, glmr, "DBFIND");
		
	/*-------------------------------------------
	| Add transaction for account .             |
	-------------------------------------------*/
	strcpy (glwkRec.acc_no,acct);
	strcpy (glwkRec.co_no,comm_rec.co_no);
	strcpy (glwkRec.est_no,comm_rec.est_no);
	strcpy (glwkRec.acronym,"         ");
	strcpy (glwkRec.name,"                              ");
	sprintf (glwkRec.chq_inv_no, "%-8.8s", " ");
	glwkRec.ci_amt = 0;
	glwkRec.o1_amt = 0;
	glwkRec.o2_amt = 0;
	glwkRec.o3_amt = 0;
	glwkRec.o4_amt = 0;
	glwkRec.hhgl_hash = glmrRec.hhmr_hash;
	strcpy (glwkRec.tran_type,"15");
	sprintf (glwkRec.sys_ref, "%010ld", (long) comm_rec.term);
	glwkRec.tran_date = comm_rec.dbt_date;
	
	DateToDMY (glwkRec.tran_date, NULL, &monthPeriod,NULL);
	sprintf (glwkRec.period_no, "%02d", monthPeriod);

	glwkRec.post_date = StringToDate (local_rec.systemDate);
	strcpy (glwkRec.narrative,"Inv/cn exch rate adj");
	strcpy (glwkRec.alt_desc1, " ");
	strcpy (glwkRec.alt_desc2, " ");
	strcpy (glwkRec.alt_desc3, " ");
	strcpy (glwkRec.batch_no, " ");
	strcpy (glwkRec.user_ref,"Exch rate Adj. ");
	strcpy (glwkRec.stat_flag,"2");

	GL_AddBatch ();
}


/*====================================
| Update GLBC Batch Control Records. |
====================================*/
void
UpdateWfbc (void)
{

	/*---------------------------------------------
   	| Add general ledger batch control record.    |
	---------------------------------------------*/
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
			file_err (cc, wfbc, "DBADD");

		abc_unlock (wfbc);
	}
	else
	{
		wfbc_rec.batch_tot_1 += CENTS (exch_var_tot);
		cc = abc_update (wfbc,&wfbc_rec);
		if (cc)
			file_err (cc, wfbc, "DBUPDATE");
	}
}


/*==============================
| Start output for first part. |
===============================*/
void
Heading1Output (
 void)
{

	if ( (fout = popen ("pformat","w")) == NULL)
		file_err (errno, "pformat", "POPEN");

	sprintf (err_str, "%s <%s>",local_rec.systemDate,PNAME);
	fprintf (fout, ".START%s\n",clip (err_str));
	fprintf (fout, ".LP%d\n",printerNo);

	fprintf (fout, ".10\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L158\n");
	fprintf (fout, ".E%s\n", clip (comm_rec.co_name));
	fprintf (fout, ".B1\n");
	fprintf (fout, ".EEXCHANGE VARIANCE AUDIT REPORT FOR CURRENCY: %s\n",
						pocrRec.code);

	fprintf (fout, ".R===================================================");
	fprintf (fout, "==========================");
	fprintf (fout, "========================================");
	fprintf (fout, "=================================\n");

	fprintf (fout, "===================================================");
	fprintf (fout, "==========================");
	fprintf (fout, "========================================");
	fprintf (fout, "=================================\n");

	fprintf (fout, "|CUSTOMER|            CUSTOMER NAME                ");
	fprintf (fout, "| INVOICE|   FGN VALUE    ");
	fprintf (fout, "| OLD EXCH. | NEW EXCH. |  EXCH. VAR.   ");
	fprintf (fout, "|     DEBIT     |     CREDIT    |\n");

	fprintf (fout, "| NUMBER |                                         ");
	fprintf (fout, "| NUMBER |     AMOUNT     ");
	fprintf (fout, "|   RATE    |   RATE    |    AMOUNT     ");
	fprintf (fout, "|               |               |\n");

	fprintf (fout, "|--------|-----------------------------------------");
	fprintf (fout, "|--------|----------------");
	fprintf (fout, "|-----------|-----------|---------------");
	fprintf (fout, "|---------------|---------------|\n");
	fflush (fout);
}



/*===============================
| Start output for second part. |
================================*/
void
Heading2Output (
 void)
{
	if ( (fout = popen ("pformat","w")) == NULL)
		file_err (errno, "pformat", "POPEN");

	sprintf (err_str, "%s <%s>",local_rec.systemDate,PNAME);
	fprintf (fout, ".START%s\n",clip (err_str));
	fprintf (fout, ".LP%d\n",printerNo);

	fprintf (fout, ".10\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L120\n");
	fprintf (fout, ".E%s\n", clip (comm_rec.co_name));
	fprintf (fout, ".B1\n");
	fprintf (fout, ".EEXCHANGE VARIANCE AUDIT REPORT FOR CURRENCY: %s\n",
						pocrRec.code);

	fprintf (fout, ".EACCOUNT SUMMARY : NOTE VARIANCE POSTED IN %s\n",locCurr);

	fprintf (fout, ".R============================================");
	fprintf (fout, "=================================\n");

	fprintf (fout, "============================================");
	fprintf (fout, "=================================\n");

	fprintf (fout, "|   DETAIL NARRATIVE   |       ACCOUNT      ");
	fprintf (fout, "|     DEBIT     |     CREDIT    |\n");

	fprintf (fout, "|----------------------|--------------------");
	fprintf (fout, "|---------------|---------------|\n");
	fflush (fout);
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
		clear ();
		sprintf (err_str, " %s ", ML (mlFeMess004));
		rv_pr (err_str, 22, 0, 1);

		fflush (stdout);
		line_at (1,0,80);

		box (0,3,80,3);
		scn_set (1);
		scn_write (1);
		if (scn == 2 || prog_status != ENTRY)
		{
			scn_display (1);
			scn_set (2);
			scn_write (2);
			scn_display (2);
		}
		if (scn != cur_screen)
			scn_set (scn);

		line_at (20,1,80);
		print_at (21,0, ML (mlStdMess038),comm_rec.co_no,comm_rec.co_name);
		print_at (22,0, ML (mlStdMess039),comm_rec.est_no,comm_rec.est_name);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
	}
    return 0;
}


/*==============
| Delete line. |
==============*/
int
DeleteLine (
 int dispLine)
{
	int		i;
	int		this_page;
	int		delta;

	if (prog_status == ENTRY)
	{
		/*------------------------------
		| Cannot Delete Lines on Entry |
		------------------------------*/
		print_mess (ML (mlStdMess005));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}

	getval (line_cnt);

	strcpy (cuin_rec.est,    local_rec.branchNo);
	strcpy (cuin_rec.inv_no, local_rec.invoiceNo);
	cc = find_rec (cuin, &cuin_rec, EQUAL, "r");
	if (cc)
	{
		/*---------------------------
		| Invoice %s is not on file.|
		---------------------------*/
		print_mess (ML (mlStdMess115));
		sleep (sleepTime);
		clear_mess ();
		strcpy (local_rec.invoiceNo, SI.invoiceNo);
		return (EXIT_FAILURE); 
	}

	cudt_rec.hhci_hash = cuin_rec.hhci_hash;
	cc = find_rec (cudt, &cudt_rec, EQUAL, "r");
	if (!cc)
	{
		/*----------------------------------------------------------------------
		| Invoice has already been paid or partly paid and cannot be unassigned|
		----------------------------------------------------------------------*/
		print_mess (ML (mlFeMess011));
		sleep (sleepTime);
		clear_mess ();
		strcpy (local_rec.invoiceNo, SI.invoiceNo);
		return (EXIT_FAILURE); 
	}
	local_rec.balance += local_rec.value;

	this_page = line_cnt / TABLINES;

	delta = 1;

	i = ChkInvNo (local_rec.invoiceNo);
	if (i < storeMax)
	{
		local_rec.storeIdx = i;
		SI.assign = FALSE;
	}

	for (i = line_cnt; line_cnt < lcount [2] - delta; line_cnt++)
	{
		getval (line_cnt + delta);
		putval (line_cnt);

		if (dispLine)
		{
			if (this_page == line_cnt / TABLINES)
				line_display ();
		}
	}

	while (line_cnt < lcount [2])
	{
		sprintf (local_rec.branchNo, "%-2.2s", " ");
		sprintf (local_rec.invoiceNo, "%-8.8s", " ");
		local_rec.value = 0.00;
		local_rec.storeIdx = -1;
		putval (line_cnt);

		if (dispLine)
		{
			if (this_page == line_cnt / TABLINES)
				blank_display ();
		}

		line_cnt++;
	}

	lcount [2] -= delta;

	line_cnt = i;
	getval (line_cnt);
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

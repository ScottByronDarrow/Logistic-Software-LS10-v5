/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: cr_chqsdry.c,v 5.5 2002/07/24 08:38:43 scott Exp $
|  Program Name  : (cr_chqsdry.c )                                   |
|  Program Desc  : (Input Suppliers Sundry Cheque Details.     )     |
|---------------------------------------------------------------------|
|  Author        : Terry Keillor.  | Date Written  : 11/05/89         |
|---------------------------------------------------------------------|
| $Log: cr_chqsdry.c,v $
| Revision 5.5  2002/07/24 08:38:43  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.4  2002/07/18 06:17:36  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.3  2002/06/21 04:10:23  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.2  2001/08/09 08:51:40  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:01:16  scott
| RELEASE 5.0
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cr_chqsdry.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CR/cr_chqsdry/cr_chqsdry.c,v 5.5 2002/07/24 08:38:43 scott Exp $";

#define MAXLINES	200 
#define MCURR		 (multiCurrency [0] == 'Y')
#include <ml_std_mess.h>
#include <ml_cr_mess.h>
#include <pslscr.h>
#include <GlUtils.h>
#include <pslscr.h>
#include <twodec.h>

#include "schema"

	/*===============================================================
	| Special fields and flags  ##################################. |
	===============================================================*/
   	int		chequeProof = TRUE,
			pidNumber,
			numberCheques = 0,
			printerNumber,
   			currentGlPeriod = 0;

	char	multiCurrency [2],	/* Multi currency control flag (Y/N) */
	    	terminalNumber [sizeof glblRec.sys_ref];

   	double  batchTotal = 0.0;

   	long 	suspen_hash;

	struct	storeRec {
		double 	GlAllocation;
	} store [MAXLINES];

	char	*fifteenSpaces	=	"               ";

	struct	commRecord	comm_rec;
	struct	comrRecord	comr_rec;
	struct	suhpRecord	suhp_rec;
	struct	suhtRecord	suht_rec;
	struct	crbkRecord	crbk_rec;
	struct	wfbcRecord	wfbc_rec;

/*=============================
| Local & Screen Structures . |
=============================*/
struct {
	char 	dummy [11];
	char 	prev_cheque [sizeof glwkRec.user_ref];
	char 	loc_curr [4];
	char 	systemDate [11];
	double	orig_chq_amt;
	double	exch_rate;
	double	loc_chq_amt;
	char 	cheque_no [sizeof glwkRec.user_ref];
	char 	name [31];
	long	cheque_date;
	char 	narrative [sizeof glwkRec.narrative];
	char 	acc_no [MAXLEVEL + 1];
	double	gl_amt;
	double	gl_amt_loc;
	char 	gl_narrative [sizeof glwkRec.narrative];
	char 	chq_approved [2];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "bank_id",	 4, 20, CHARTYPE,
		"UUUUU", "          ",
		" ", "", "Bank Code", "enter code orgSEARCH] key.",
		 NE, NO,  JUSTLEFT, "", "", crbk_rec.bank_id},
	{1, LIN, "bk_name",	 4, 30, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", crbk_rec.bank_name},
	{1, LIN, "br_name",	 5, 20, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Branch Name ", " ",
		 NA, NO,  JUSTLEFT, "", "", crbk_rec.branch_name},
	{1, LIN, "acct_name",	 6, 20, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Account Name", " ",
		 NA, NO,  JUSTLEFT, "", "", crbk_rec.acct_name},
	{1, LIN, "bank_no",	 7, 20, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", "", "Bank No.", " ",
		 NA, NO,  JUSTLEFT, "", "", crbk_rec.bank_no},
	{1, LIN, "bk_acno",	 7, 60, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", "", "     Account No.", " ",
		 NA, NO,  JUSTLEFT, "", "", crbk_rec.bank_acct_no},
	{1, LIN, "bk_curr",	 8, 20, CHARTYPE,
		"UUU", "          ",
		" ", " ", "Currency Code", " ",
		 NA, NO,  JUSTLEFT, "", "", crbk_rec.curr_code},
	{1, LIN, "curr_desc",	 8, 30, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", pocrRec.description},
	{1, LIN, "orig_chq",	10, 20, MONEYTYPE,
		"NNNNNNNNNNN.NN", "          ",
		" ", " ", "Origin Amount", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.orig_chq_amt},
	{1, LIN, "exch_rate",	11, 20, DOUBLETYPE,
		"NNNN.NNNNNNNN", "          ",
		" ", ".0001 ", "Exchange Rate", "<retn> defaults to current rate for currency ",
		YES, NO, JUSTRIGHT, ".0001", "9999", (char *)&local_rec.exch_rate},
	{1, LIN, "loc_chq",	12, 20, MONEYTYPE,
		"NNNNNNNNNNN.NN", "          ",
		" ", " ", "Local Amount ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.loc_chq_amt},
	{1, LIN, "chq_no",	13, 20, CHARTYPE,
		"UUUUUUUUUUUUUUU", "          ",
		" ", " ", "Cheque No.", " ",
		YES, NO, JUSTLEFT, "", "", local_rec.cheque_no},
	{1, LIN, "name",	14, 20, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Name", " ",
		 NO, NO,  JUSTLEFT, "", "", local_rec.name},
	{1, LIN, "chqdate",	15, 20, EDATETYPE,
		"DD/DD/DD", "        ",
		" ", local_rec.systemDate, "Date of Cheque", "<retn> defaults to system (todays) date ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.cheque_date},
	{1, LIN, "narr",	16, 20, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "G/L Narrative", "",
		 NO, NO,  JUSTLEFT, "", "", local_rec.narrative},
	{2, TAB, "glacct",	MAXLINES, 0, CHARTYPE,
		GlMask, "          ",
		"0", "0", GlDesc, "Enter account or usergSEARCH] key ",
		YES, NO,  JUSTLEFT, "", "", local_rec.acc_no},
	{2, TAB, "gl_amt",	 0, 1, MONEYTYPE,
		"NNNNNNNNNNN.NN", "          ",
		" ", "0", " Base Curr Amt ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.gl_amt},
	{2, TAB, "gl_amt_loc",	 0, 1, MONEYTYPE,
		"NNNNNNNNNN.NN", "          ",
		" ", "0", " Equiv Loc Curr ", " ",
		 NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.gl_amt_loc},
	{2, TAB, "gl_narr",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", local_rec.narrative, "      Narrative.    ", "<retn> defaults to screen 1 narrative ",
		YES, NO,  JUSTLEFT, "", "", local_rec.gl_narrative},
	{3, LIN, "app",	 4, 20, CHARTYPE,
		"U", "          ",
		" ", "", "Process Cheque ", "Enter Y (es) to approve, N (o) to edit, F1 to restart ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.chq_approved},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", "",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

#include	<CashBook.h>

/*===========================
| Local function prototypes |
===========================*/
void	shutdown_prog	 (void);
void	OpenDB			 (void);
void	CloseDB			 (void);
void	ReadComr		 (void);
int		spec_valid		 (int);
int		InsertLine		 (void);
int		DeleteLine		 (void);
void	RecalcTab		 (void);
void	ClearAlloc		 (void);
void	DisplayAlloc	 (void);
void	CheckAlloc 		 (void);
double	TotalAlloc		 (int);
void	SrchCrbk		 (char *);
int		FindGlmr		 (char *);
void	SrchGlmr		 (char *);
int		heading			 (int);
int		Update			 (void);
void	UpdateWfbc		 (void);
int		CheckClass		 (void);
void	CreateSuhp		 (void);
void	CreateSuht		 (void);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char	*argv [])
{
	int	i;

	if (argc != 3)
	{
		print_at (0,0,mlStdMess072,argv [0]);
		return (EXIT_FAILURE);
	}
	pidNumber   	= atoi (argv [1]);
	printerNumber  	= atoi (argv [2]);

	SETUP_SCR (vars);


	init_scr	 ();
	set_tty		 ();
	set_masks	 ();
/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (2, store, sizeof (struct storeRec));
#endif
	init_vars	 (1);

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));

	sprintf (multiCurrency, "%-1.1s", get_env ("CR_MCURR"));

	/*------------------------------------------------------
	| Reset screen control if not multi-currency suppliers.|
	------------------------------------------------------*/
	if (!MCURR)
	{
		FLD ("bk_curr")    = ND;
		FLD ("curr_desc")  = ND;
		FLD ("exch_rate")  = ND;
		FLD ("loc_chq")    = ND;
		FLD ("gl_amt_loc") = ND;
	 }
	
	OpenDB ();
	ReadComr ();

	sprintf (local_rec.loc_curr, "%-3.3s", comr_rec.base_curr);

	GL_SetMask (GlFormat);

	while (prog_exit == 0) 
	{	
		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		search_ok	= TRUE;
		init_vars (1);
		lcount [2] 	= 0;
		ClearAlloc ();
		local_rec.exch_rate = 0.0000;
		chequeProof = TRUE;

		for (i = 0; i < MAXLINES; i++)
			store [i].GlAllocation = 0.00;

		/*------------------------------
		| Enter screen 1 linear input. |
		------------------------------*/
		heading (1);
		entry (1);
		if (prog_exit || restart) 
			continue;

		no_edit (3);

		/*-------------------------------
		| Enter screen 2 tabular input. |
		-------------------------------*/
		heading (2);
		entry (2);
		if (restart) 
			continue;

		/*-------------------------------------------
		| Re-edit tabular if proof total incorrect. |
		-------------------------------------------*/
		do
		{
			edit_all ();
			if (restart)
				 break;

			heading (3);
			scn_display (3);
			entry (3);
			if (restart)
				 break;
		} while (chequeProof) ;

		if (restart)
			 continue;

		/*----------------------------------
		| Update cheque detail records .   |
		----------------------------------*/
		if (chequeProof == FALSE)
			Update ();
		
	}
	if (numberCheques > 0)
	{
		UpdateWfbc ();
	 	shutdown_prog ();
		return (EXIT_SUCCESS);
	}
	shutdown_prog ();
	return (EXIT_FAILURE);
}

/*========================
| Program exit sequence. |
========================*/
void
shutdown_prog (
 void)
{
	clear ();
	print_at (0, 0, ML (mlCrMess039),DOLLARS (batchTotal));
	print_at (1, 0, ML (mlStdMess042));
    PauseForKey (3, 0, ML ("Press Any Key to Continue."), 0);
	CloseDB (); 
	FinishProgram ();
}

/*======================
| Open Database Files. |
======================*/
void
OpenDB (
 void)
{
	abc_dbopen ("data");
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (wfbc, wfbc_list, WFBC_NO_FIELDS, "wfbc_id_no");
	open_rec (crbk, crbk_list, CRBK_NO_FIELDS, "crbk_id_no");
	open_rec (pocr, pocr_list, POCR_NO_FIELDS, "pocr_id_no");
	open_rec (glmr, glmr_list, GLMR_NO_FIELDS, "glmr_id_no");
	open_rec (suhp, suhp_list, SUHP_NO_FIELDS, "suhp_id_no2");
	open_rec (suht, suht_list, SUHT_NO_FIELDS, "suht_id_no");

	OpenGlmr ();
	OpenPocr ();
	OpenCashBook ();
	GL_OpenBatch (comm_rec.co_no, comm_rec.est_no);
}

/*=======================
| Close Database Files. |
=======================*/
void
CloseDB (
 void)
{
	abc_fclose (crbk);
	abc_fclose (wfbc);
	abc_fclose (suhp);
	abc_fclose (suht);
	CloseCashBook ();
	GL_CloseBatch (printerNumber);
	GL_Close ();

	abc_dbclose ("data");
}

/*============================================
| Get common info from commom database file. |
============================================*/
void
ReadComr (
 void)
{
	sprintf (terminalNumber, "%010ld", (long) comm_rec.term);

	open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");

	strcpy (comr_rec.co_no, comm_rec.co_no);
	cc = find_rec (comr, &comr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, comr, "DBFIND");

	abc_fclose (comr);

	DateToDMY (comr_rec.crd_date, NULL, &currentGlPeriod, NULL);
}

/*============================
| Special Validation Section.|
============================*/
int
spec_valid (
 int field)
{
	double 	totalGl = 0.00;

	/*-----------------------------------------
	| Validate Bank Id Code And Allow Search. |
	-----------------------------------------*/
	if (LCHECK ("bank_id"))
	{
		if (end_input)
		{
			prog_exit = 1;
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchCrbk (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (crbk_rec.co_no,comm_rec.co_no);
		cc = find_rec (crbk, &crbk_rec, COMPARISON, "r");
		if (cc) 
		{
			errmess (ML (mlStdMess043));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		DSP_FLD ("bk_name");
		DSP_FLD ("br_name");
		DSP_FLD ("acct_name");
		DSP_FLD ("bank_no");
		DSP_FLD ("bk_acno");

		/*--------------------------------
		| Read Bank Currency Record.     |
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
	| Validate Origin Cheque Amount.|
	-------------------------------*/
	if (LCHECK ("orig_chq"))
	{
		if (!MCURR)
			return (EXIT_SUCCESS);

		if (local_rec.exch_rate > 0.0000)
		{
			local_rec.loc_chq_amt = no_dec (local_rec.orig_chq_amt / 
										   local_rec.exch_rate);
			DSP_FLD ("loc_chq");
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("chq_no"))
	{
		if (dflt_used || !strcmp (local_rec.cheque_no, fifteenSpaces))
		{
			print_mess (ML ("Sorry, Cannot be blank."));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		strcpy (suhp_rec.co_no, comm_rec.co_no);
		strcpy (suhp_rec.bank_id, crbk_rec.bank_id);
		sprintf (suhp_rec.cheq_no, "%-15.15s", local_rec.cheque_no);
		cc = find_rec (suhp, &suhp_rec, COMPARISON, "r");
		if (!cc)
		{
			print_mess ("Duplicate Cheque No. for bank entered");
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
	}
	 
	/*-------------------------------
	| Validate Exchange Rate.       |
	-------------------------------*/
	if (LCHECK ("exch_rate"))
	{
		if (!MCURR)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			local_rec.exch_rate = pocrRec.ex1_factor;
			DSP_FLD ("exch_rate");
		}
		local_rec.loc_chq_amt = no_dec (local_rec.orig_chq_amt / 
					 					local_rec.exch_rate);

		/*-------------------------------------
		| Recalculate Tabular screen on Edit. |
		-------------------------------------*/
		if (prog_status != ENTRY)
			RecalcTab ();

		DSP_FLD ("loc_chq");
	    	return (EXIT_SUCCESS);
	}

	/*--------------------------------
	| Validate Local Cheque Amount.  |
	--------------------------------*/
	if (LCHECK ("loc_chq"))
	{	
		if (!MCURR)
			return (EXIT_SUCCESS);

		if (local_rec.exch_rate <= 0.00)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			local_rec.loc_chq_amt = no_dec (local_rec.orig_chq_amt / 
										   local_rec.exch_rate);
			DSP_FLD ("loc_chq");
			return (EXIT_SUCCESS);
		}
		local_rec.orig_chq_amt = no_dec (local_rec.loc_chq_amt *
										 local_rec.exch_rate);

		DSP_FLD ("orig_chq");
	
		return (EXIT_SUCCESS);
	}

	/*-------------------------------
	| Validate Cheque Date.         |	
	-------------------------------*/
	if (LCHECK ("chqdate"))
	{
		if (dflt_used) 
			DSP_FLD ("chqdate");

		if (local_rec.cheque_date < MonthStart (comm_rec.gl_date))
			return print_err (ML (mlStdMess248),
				DateToString (MonthStart (comm_rec.gl_date)));

		if (local_rec.cheque_date > MonthEnd (TodaysDate ()))
		    return print_err (ML (mlStdMess248), DateToString (MonthEnd (TodaysDate ())));

		return (EXIT_SUCCESS);
	}
		
	/*-----------------------------------------
	| Validate General Ledger Account Number. |
	-----------------------------------------*/
	if (LCHECK ("glacct"))
	{
		if (SRCH_KEY)
		{
			SrchGlmr (temp_str);
			return (EXIT_SUCCESS);
		}

		if (last_char == DELLINE)
			return (DeleteLine ());

		if (last_char == INSLINE)
			return (InsertLine ());

		strcpy (glmrRec.co_no,comm_rec.co_no);
		strcpy (glmrRec.acc_no, "");

		GL_FormAccNo (local_rec.acc_no, glmrRec.acc_no, 0);
		cc = find_rec (glmr, &glmrRec, COMPARISON,"r");
		if (cc) 
		{
			errmess (ML (mlStdMess024));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		if (CheckClass ())
			return (EXIT_FAILURE);
		
		sprintf (err_str, ML (mlCrMess037), glmrRec.desc);
		rv_pr (err_str, 2,5,1);
		return (EXIT_SUCCESS);
	}

	/*--------------------------------------------
	| Validate General Ledger Allocation Amount. |
	--------------------------------------------*/
	if (LCHECK ("gl_amt"))
	{
		if (MCURR)
		{
			if (local_rec.exch_rate == 0.00)
			  	 local_rec.exch_rate = 1.00;

			local_rec.gl_amt_loc = no_dec (local_rec.gl_amt / 
										   local_rec.exch_rate); 
			DSP_FLD ("gl_amt_loc");
		}
		store [line_cnt].GlAllocation = no_dec (local_rec.gl_amt);
		DisplayAlloc ();
		return (EXIT_SUCCESS);
	}

	/*-------------------------
	| Validate Approved Flag. |
	-------------------------*/
	if (LCHECK ("app"))
	{
		chequeProof = TRUE;
		if (local_rec.chq_approved [0] == 'N')
			return (EXIT_SUCCESS);

		CheckAlloc ();

		totalGl = TotalAlloc (FALSE);
		local_rec.orig_chq_amt = no_dec (local_rec.orig_chq_amt);

		if (local_rec.chq_approved [0] == 'Y' && 
			totalGl != local_rec.orig_chq_amt)
		{
			sprintf (err_str,ML (mlCrMess041), DOLLARS (totalGl),DOLLARS (local_rec.orig_chq_amt));
			errmess (err_str);
			sleep (sleepTime);
			chequeProof = TRUE;
			return (EXIT_FAILURE);
		}
		chequeProof = FALSE;
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*==============
| Insert Line. |
==============*/
int
InsertLine (
 void)
{
	int	i;
	int	this_page = line_cnt / TABLINES;

	if (prog_status == ENTRY)
	{
		print_mess (ML (mlStdMess005));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}

	if (lcount [2] >= vars [label ("glacct")].row)
	{
		print_mess (ML (mlStdMess076));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}
	for (i = line_cnt,line_cnt = lcount [2];line_cnt > i;line_cnt--)
	{
		getval (line_cnt - 1);
		putval (line_cnt);
		store [line_cnt].GlAllocation = store [line_cnt -1].GlAllocation;
		if (this_page == line_cnt / TABLINES)
			line_display ();
	}
	lcount [2]++;
	line_cnt = i;

	sprintf (local_rec.acc_no,"%*.*s",MAXLEVEL, MAXLEVEL, " ");
	local_rec.gl_amt = 0.00;
	store [line_cnt].GlAllocation = 0.00;
	putval (line_cnt);

	if (this_page == line_cnt / TABLINES)
		blank_display ();

	init_ok = 0;
	prog_status = ENTRY;
	scn_entry (cur_screen);
	prog_status = !ENTRY;
	init_ok = 1;
	line_cnt = i;
	getval (line_cnt);
	DisplayAlloc ();
	return (EXIT_SUCCESS);
}

/*==============
| Delete line. |
==============*/
int
DeleteLine (
 void)
{
	int	i;
	int	this_page;

	if (prog_status == ENTRY)
	{
		print_mess (ML (mlStdMess005));
		return (EXIT_FAILURE);
	}

	lcount [2]--;

	this_page = line_cnt / TABLINES;

	for (i = line_cnt;line_cnt < lcount [2];line_cnt++)
	{
		getval (line_cnt + 1);
		putval (line_cnt);
		store [line_cnt].GlAllocation = store [line_cnt + 1].GlAllocation;

		if (this_page == line_cnt / TABLINES)
			line_display ();
	}

	sprintf (local_rec.acc_no,"%*.*s",MAXLEVEL, MAXLEVEL," ");
	store [line_cnt].GlAllocation = 0.00;
	local_rec.gl_amt = 0.00;
	putval (line_cnt);

	if (this_page == line_cnt / TABLINES)
		blank_display ();
	
	line_cnt = i;
	getval (line_cnt);
	DisplayAlloc ();
	return (EXIT_SUCCESS);
}
/*==============================================
| Recalculate Exchange Rate on tabular screen. |
==============================================*/
void
RecalcTab (
 void)
{
	int	i = line_cnt;

	if (!MCURR)
		return;

	scn_set (2);

	for (line_cnt = 0; line_cnt < lcount [2]; line_cnt++) 
	{
		getval (line_cnt);
		if (local_rec.exch_rate == 0.00)
		  	 local_rec.exch_rate = 1.00;

		local_rec.gl_amt_loc = no_dec (local_rec.gl_amt / 
									   local_rec.exch_rate); 
		putval (line_cnt); 
	    store [line_cnt].GlAllocation = no_dec (local_rec.gl_amt);
	}
	scn_set (1);
	line_cnt = i;
}

/*----------------------------------
| Clear Allocation Array.          |
----------------------------------*/
void
ClearAlloc (
 void)
{
	int	i;

	for (i = 0; i < MAXLINES; i++)
		store [i].GlAllocation = 0.00;
}

/*----------------------------------
| Display Allocation Balance.      |
----------------------------------*/
void
DisplayAlloc (
 void)
{
	move (15, 19);
	cl_line ();
	print_at (19,15,ML (mlCrMess042), DOLLARS (TotalAlloc (TRUE)));
	fflush (stdout);
}

void
CheckAlloc (
 void)
{
	int	i;

	scn_set (2);

	for (i = 0; i < MAXLINES; i++)
		store [i].GlAllocation = 0.00;

	for (i = 0; i < lcount [2]; i++) 
	{
		getval (i);
		store [i].GlAllocation = no_dec (local_rec.gl_amt);
	}
	scn_set (3);
}

/*----------------------------------
| Total GL Allocations.            |
----------------------------------*/
double
TotalAlloc (
 int bal_orig)
{
	int	i;

	double	total_alloc = 0.00;

	for (i = 0;i < MAXLINES ;i++)
		total_alloc += no_dec (store [i].GlAllocation);
	
	if (bal_orig)
		return (no_dec (local_rec.orig_chq_amt - total_alloc));
	else
		return (no_dec (total_alloc));
}

/*=========================================
| Search routine for Suppliers Bank File. |
=========================================*/
void
SrchCrbk (
 char *	key_val)
{
	_work_open (5,0,40);
	save_rec ("#Code.","#Bank Code Description ");
	strcpy (crbk_rec.co_no, comm_rec.co_no);
	strcpy (crbk_rec.bank_id, key_val);
	cc = find_rec (crbk, &crbk_rec, GTEQ, "r");
	while (!cc && !strncmp (crbk_rec.bank_id,key_val,strlen (key_val)) && 
		      !strcmp (crbk_rec.co_no, comm_rec.co_no))
	{
		cc = save_rec (crbk_rec.bank_id,crbk_rec.bank_name);
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

/*============================
| Read gl account master.    |
============================*/
int
FindGlmr (
 char *	account)
{
	strcpy (glmrRec.co_no,comm_rec.co_no);
	sprintf (glmrRec.acc_no,"%-*.*s", MAXLEVEL, MAXLEVEL, account);
	cc = find_rec (glmr, &glmrRec, COMPARISON,"r");
	if (cc) 
		return (EXIT_FAILURE);

	if (glmrRec.glmr_class [2][0] != 'P') 
	    return (2);
	
	return (EXIT_SUCCESS);
}

/*===================================
| Search routine for Account Number |
===================================*/
void
SrchGlmr (
 char *	key_val)
{
	_work_open (16,0,40);
	strcpy (glmrRec.co_no,comm_rec.co_no);
	sprintf (glmrRec.acc_no, "%-*.*s", MAXLEVEL, MAXLEVEL, key_val);
	sprintf (err_str, "%-*.*s", MAXLEVEL, MAXLEVEL, "# Account Number ");
	save_rec (err_str,"#Account Description ");
	cc = find_rec (glmr, &glmrRec, GTEQ,"r");
	while (!cc && !strncmp (glmrRec.acc_no, key_val,strlen (key_val)) && 
		      !strcmp (glmrRec.co_no, comm_rec.co_no))
	{
		if (glmrRec.glmr_class [2][0] == 'P')
		{
			cc = save_rec (glmrRec.acc_no, glmrRec.desc);
			if (cc)
				break;
		}
		cc = find_rec (glmr, &glmrRec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (glmrRec.co_no,comm_rec.co_no);
	sprintf (glmrRec.acc_no, "%-*.*s", MAXLEVEL, MAXLEVEL, temp_str);
	cc = find_rec (glmr, &glmrRec, COMPARISON,"r");
	if (cc)
		file_err (cc, glmr, "DBFIND");
}

/*====================================================
| Screen Heading Routine.                            |
====================================================*/
int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();
		rv_pr (ML (mlCrMess038),20,0,1);
		move (45,0);
		print_at (0,45,ML (mlCrMess062),local_rec.prev_cheque, " ");
		move (0,1);
		line (80);

		move (1,input_row);
		switch (scn)
		{
		case 1 :
			box (0,3,79,13);
			move (1,9);
			line (78);
			break;
		
		case 2 :
			if (MCURR)
				print_at (4,15,ML (mlCrMess107), 
						crbk_rec.curr_code, 
						DOLLARS (local_rec.orig_chq_amt),
						local_rec.loc_curr, 
						DOLLARS (local_rec.loc_chq_amt));
			else
				print_at (4,15,ML (mlCrMess107), "",
						DOLLARS (local_rec.orig_chq_amt),
 						local_rec.loc_curr,0.00);
			DisplayAlloc ();
			fflush (stdout);
			break;
		
		case 3 :
			box (0,3,79,1);
			break;
		}

		move (0,20);
		line (80);
		strcpy (err_str,ML (mlStdMess038));
		print_at (21,0,err_str,comm_rec.co_no,
									   comm_rec.co_name);
		strcpy (err_str,ML (mlStdMess039));
		print_at (22,0,err_str,comm_rec.est_no,
									   comm_rec.est_name);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}

/*=======================================================
| Rountine to sum cheques by bank id and create glwk    |
| transaction for each allocation.                      |
=======================================================*/
int
Update (
 void)
{
	int		chq_pd;
	double	tot_loc = 0.00,
			tot_fgn = 0.00,
			diff = 0.00;

	char	chq_pd_str [3];

	/*-------------------------------------------
	| Add gl transaction for cheque header.     |
	-------------------------------------------*/
	if (local_rec.cheque_date <= MonthEnd (comm_rec.gl_date))	
		chq_pd = currentGlPeriod;
	else
		chq_pd =13;

	sprintf (chq_pd_str,"%02d",chq_pd);

	if (FindGlmr (crbk_rec.gl_bank_acct))
	{ 
		GL_GLI 
		(
			comm_rec.co_no,
			comm_rec.est_no,
			"  ",
			"SUSPENSE  ",
			"   ", 
			" "
		);

		strcpy (crbk_rec.gl_bank_acct,glmrRec.acc_no);
		cc = FindGlmr (crbk_rec.gl_bank_acct);
		if (cc)
			file_err (cc, glmr, "DBFIND");
	}
	strcpy (glwkRec.acc_no,crbk_rec.gl_bank_acct);
	glwkRec.hhgl_hash = glmrRec.hhmr_hash;

	strcpy (glwkRec.co_no,comm_rec.co_no);
	strcpy (glwkRec.est_no,comm_rec.est_no);
	sprintf (glwkRec.acronym,"SUNDRY   ");
	strcpy (glwkRec.name, local_rec.name);
	sprintf (glwkRec.chq_inv_no, "%-15.15s", local_rec.cheque_no);
	glwkRec.ci_amt = 0.0;

	if (MCURR)
	{
		glwkRec.o1_amt 		= no_dec (local_rec.orig_chq_amt);
		glwkRec.exch_rate 	= local_rec.exch_rate;
		glwkRec.amount 		= no_dec (local_rec.orig_chq_amt);
		glwkRec.loc_amount 	= no_dec (local_rec.loc_chq_amt);
		strcpy (glwkRec.currency, crbk_rec.curr_code);
	}
	else
	{
		glwkRec.o1_amt 		= no_dec (local_rec.orig_chq_amt);
		glwkRec.exch_rate 	= 1.0;
		glwkRec.amount 		= no_dec (local_rec.loc_chq_amt);
		glwkRec.loc_amount 	= no_dec (local_rec.loc_chq_amt);
		strcpy (glwkRec.currency, local_rec.loc_curr);
	}
	glwkRec.o2_amt = 0.0;
	glwkRec.o3_amt = 0.0;
	glwkRec.o4_amt = 0.0;
	strcpy (glwkRec.tran_type," 9");
	strcpy (glwkRec.sys_ref, terminalNumber);
	glwkRec.tran_date = local_rec.cheque_date;
	strcpy (glwkRec.period_no,chq_pd_str);
	glwkRec.post_date = TodaysDate ();
	strcpy (glwkRec.narrative,local_rec.narrative);
	strcpy (glwkRec.user_ref, local_rec.cheque_no);
	sprintf (glwkRec.alt_desc1, "Bank id : %s", crbk_rec.bank_id);
	strcpy (glwkRec.alt_desc2, crbk_rec.acct_name);
	strcpy (glwkRec.alt_desc3, crbk_rec.bank_acct_no);
	strcpy (glwkRec.batch_no," ");
	strcpy (glwkRec.stat_flag,"2");
	strcpy (glwkRec.jnl_type,"2");

	GL_AddBatch ();

	sprintf (err_str,ML (mlCrMess188),local_rec.cheque_no);
			
	/*---------------------------------
	| Write entry to cash book system |
	---------------------------------*/
	WriteCashBook
	 (								/*--------------------------*/
		comm_rec.co_no,			/* Company Number			*/
		comm_rec.est_no,			/* Branch Number.			*/
		crbk_rec.bank_id,			/* Bank Id.					*/
		local_rec.cheque_date,		/* Transaction Date			*/
		err_str,					/* Transaction Narrative.	*/
		"D",						/* Transaction Type.		*/
		local_rec.loc_chq_amt * -1,	/* Amount posted to bank.	*/
		"0",						/* Status flag.				*/
		TodaysDate ()				/* System/period date.		*/
	);								/*--------------------------*/

	if (MCURR)
		batchTotal += no_dec (local_rec.loc_chq_amt);
	else
		batchTotal += no_dec (local_rec.orig_chq_amt);

	numberCheques ++;
	strcpy (local_rec.prev_cheque,local_rec.cheque_no);

	/*-------------------------------------------
	| Create cheque history header file record. |
	-------------------------------------------*/
	CreateSuhp ();

	/*-------------------------------------------------
	| Add allocations to glwk file.                   |
	-------------------------------------------------*/
	scn_set (2);

	for (line_cnt = 0; line_cnt < lcount [2]; line_cnt++) 
	{
		getval (line_cnt);

		if (FindGlmr (local_rec.acc_no))
		{ 
			GL_GLI 
			(
				comm_rec.co_no,
				comm_rec.est_no,
				"  ",
				"SUSPENSE  ",
				"   ", 
				" "
			);
			strcpy (local_rec.acc_no, glmrRec.acc_no);
			cc = FindGlmr (local_rec.acc_no);
			if (cc)
				file_err (cc, glmr, "DBFIND");
		}
		strcpy (glwkRec.acc_no,local_rec.acc_no);
		glwkRec.hhgl_hash = glmrRec.hhmr_hash;
		
		strcpy (glwkRec.co_no,comm_rec.co_no);
		strcpy (glwkRec.est_no,comm_rec.est_no);
		sprintf (glwkRec.acronym,"SUNDRY   ");
		strcpy (glwkRec.name, local_rec.name);
		sprintf (glwkRec.chq_inv_no, "%-15.15s", local_rec.cheque_no);
		glwkRec.ci_amt = 0.0;
	
		if (MCURR)
		{
			glwkRec.o1_amt 		= no_dec (local_rec.gl_amt);
			glwkRec.exch_rate 	= local_rec.exch_rate;
			glwkRec.amount 		= no_dec (local_rec.gl_amt);
			glwkRec.loc_amount 	= no_dec (local_rec.gl_amt_loc);
			strcpy (glwkRec.currency, crbk_rec.curr_code);
		}
		else
		{
			glwkRec.o1_amt 		= no_dec (local_rec.gl_amt);
			glwkRec.exch_rate 	= 1.0;
			glwkRec.amount 		= no_dec (local_rec.gl_amt);
			glwkRec.loc_amount 	= no_dec (local_rec.gl_amt);
			strcpy (glwkRec.currency, local_rec.loc_curr);
		}
	
		glwkRec.o2_amt = 0.0;
		glwkRec.o3_amt = 0.0;
		glwkRec.o4_amt = 0.0;
		strcpy (glwkRec.tran_type," 9");
		strcpy (glwkRec.sys_ref, terminalNumber);
		glwkRec.tran_date = local_rec.cheque_date;
		strcpy (glwkRec.period_no,chq_pd_str);
		glwkRec.post_date = TodaysDate ();
		strcpy (glwkRec.narrative,local_rec.gl_narrative);
		strcpy (glwkRec.user_ref, local_rec.cheque_no);
		sprintf (glwkRec.alt_desc1, "Bank id : %s", crbk_rec.bank_id);
		strcpy (glwkRec.alt_desc2, crbk_rec.acct_name);
		strcpy (glwkRec.alt_desc3, crbk_rec.bank_acct_no);
		strcpy (glwkRec.batch_no," ");
		strcpy (glwkRec.stat_flag,"2");
		strcpy (glwkRec.jnl_type,"1");

		tot_loc += no_dec (local_rec.gl_amt_loc);
		tot_fgn += no_dec (local_rec.gl_amt);

		/*---------------------------------------------------
		| Check last line and add amount to it if required. |
		---------------------------------------------------*/
		if ( (line_cnt + 1) == lcount [2])
		{
			diff = no_dec (local_rec.loc_chq_amt - tot_loc);
			if (diff != 0.00)
				local_rec.gl_amt_loc += diff;
		}
		GL_AddBatch ();

		/*-------------------------------------------
		| Create cheque history detail file record. |
		-------------------------------------------*/
		CreateSuht ();
	}
	return (EXIT_SUCCESS);
}

/*====================================
| Update GLBC Batch Control Records. |
====================================*/
void
UpdateWfbc (
 void)
{
	/*---------------------------------------------
	| Add general ledger batch control record.    |
	---------------------------------------------*/
	strcpy (wfbc_rec.co_no, comm_rec.co_no);
	wfbc_rec.pid_no = pidNumber;
	sprintf (wfbc_rec.work_file,"gl_work%05d",pidNumber);
	cc = find_rec (wfbc, &wfbc_rec, COMPARISON , "u");
    if (cc)
	{
		strcpy (wfbc_rec.system, "CR");
		wfbc_rec.date_create = TodaysDate ();
		strcpy (wfbc_rec.stat_flag, "1");
		wfbc_rec.batch_tot_1 = no_dec (batchTotal);
		cc = abc_add (wfbc,&wfbc_rec);
		if (cc)
			file_err (cc, wfbc, "DBADD");
	}
	else
	{
		wfbc_rec.batch_tot_1 += no_dec (batchTotal);
		cc = abc_update (wfbc,&wfbc_rec);
		if (cc)
			file_err (cc, wfbc, "DBUPDATE");
	}
	abc_unlock (wfbc);
}
		
int
CheckClass (
 void)
{
	if (glmrRec.glmr_class [2][0] != 'P')
		return (print_err (ML (mlStdMess025)));

	return (EXIT_SUCCESS);
}

/*======================================
| Create cheque history header record. |
======================================*/
void
CreateSuhp (
 void)
{
	strcpy (suhp_rec.co_no, comm_rec.co_no);
	strcpy (suhp_rec.bank_id, crbk_rec.bank_id);
	sprintf (suhp_rec.cheq_no, "%-15.15s", local_rec.cheque_no);
	cc = find_rec (suhp, &suhp_rec, COMPARISON, "r");
	if (cc)
	{
		strcpy (suhp_rec.co_no, comm_rec.co_no);
		sprintf (suhp_rec.payee_name,"%-40.40s",local_rec.name);
		strcpy (suhp_rec.payee_acr, " SUNDRY. ");
		strcpy (suhp_rec.narrative,local_rec.narrative);
		strcpy (suhp_rec.bank_id, crbk_rec.bank_id);
		suhp_rec.date_payment 	= local_rec.cheque_date;
		suhp_rec.date_post    	= TodaysDate ();
		suhp_rec.tot_amt_paid 	= local_rec.orig_chq_amt;
		suhp_rec.loc_amt_paid 	= local_rec.loc_chq_amt;
		suhp_rec.disc_taken   	= 0.00;
		suhp_rec.loc_disc_take 	= 0.00;
		strcpy (suhp_rec.pay_type, "5");
		strcpy (suhp_rec.stat_flag, "0");
		
		cc = abc_add (suhp, &suhp_rec);
		if (cc)
			file_err (cc, suhp, "DBADD");

		strcpy (suhp_rec.co_no, comm_rec.co_no);
		strcpy (suhp_rec.bank_id, crbk_rec.bank_id);
		strcpy (suhp_rec.cheq_no,local_rec.cheque_no);
		cc = find_rec (suhp, &suhp_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, suhp, "DBFIND");
	}
}
/*======================================
| Create cheque history detail record. |
======================================*/
void
CreateSuht (
 void)
{
	suht_rec.hhsq_hash = suhp_rec.hhsq_hash;
	strcpy (suht_rec.est_no,comm_rec.est_no);
	suht_rec.est_amt_paid = local_rec.gl_amt;
	suht_rec.est_loc_amt = local_rec.gl_amt_loc;
	strcpy (suht_rec.stat_flag,"0");
	cc = abc_add (suht,&suht_rec);
	if (cc) 
		file_err (cc, suht, "DBADD");
}

/*====================================================================+
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: cr_bk_inpt.c,v 5.9 2002/10/07 03:31:03 robert Exp $
|  Program Name  : (cr_bk_inpt.c)                                   |
|  Program Desc  : (Add/ Maintain Suppliers Bank File.      )       |
|---------------------------------------------------------------------|
|  Author        : Terry Keillor.  | Date Written  : 15/03/89         |
|---------------------------------------------------------------------|
| $Log: cr_bk_inpt.c,v $
| Revision 5.9  2002/10/07 03:31:03  robert
| SC 4285 - fixed field alignment
|
| Revision 5.8  2002/07/24 08:38:43  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.7  2002/07/18 06:17:35  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.6  2002/07/16 08:45:11  scott
| *** empty log message ***
|
| Revision 5.5  2002/06/21 04:10:17  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.4  2002/02/01 05:33:14  robert
| SC 00745 - fixed box aligned on 3rd screen
|
| Revision 5.3  2001/08/09 08:51:30  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:01:00  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 04:22:12  scott
| Updated for LS10.5
|
+====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cr_bk_inpt.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CR/cr_bk_inpt/cr_bk_inpt.c,v 5.9 2002/10/07 03:31:03 robert Exp $";

#define	MAXLINES	2000
#include <pslscr.h>
#include <GlUtils.h>
#include <ml_std_mess.h>
#include <ml_cr_mess.h>

#define	CR_MCURR		 (cr_mcurr [0] == 'Y' || cr_mcurr [0] == '1')
#define	DB_MCURR		 (db_mcurr [0] == 'Y' || db_mcurr [0] == '1')

	/*================================================================
	| Special fields and flags  ################################## . |
	================================================================*/
   	int		newBank = 0,
   			newBlhd,
   			edit_mode = 0;
	
	char	cr_mcurr [2],
			db_mcurr [2];

#include	"schema"

struct commRecord	comm_rec;
struct esmrRecord	esmr_rec;
struct crbkRecord	crbk_rec;
struct bkcrRecord	bkcr_rec;
struct blhdRecord	blhd_rec;

	char	*data = "data";

	long	orig_lodge = 0L;

	struct storeRec {
		double	factors [7];
	} store [MAXLINES];
	
	char	*SixteenSpace	=	"                ";


/*=============================
| Local & Screen Structures . |
=============================*/
struct {
	char	dummy [11];
	char	systemDate [11];
	char	acct_desc [81];
	char	acct_desc1 [26];
	char	chg_desc [26];
	char	exch_desc [26];
	char	bill_desc [26];
	char	fwd_desc [26];
	char	int_acct_desc [26];
	char	pcash_acct_desc [26];
	char	code [4];
	double	factor;
	long	ldate_up;
	long	nx_lodge_no;
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "bank_id",	 3, 24, CHARTYPE,
		"UUUUU", "          ",
		" ", "", "Bank Code             ", "Enter Bank Code. [SEARCH] available.",
		 NE, NO,  JUSTLEFT, "", "", crbk_rec.bank_id},
	{1, LIN, "bk_name",	 5, 24, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Bank Name             ", "Enter Bank name.",
		 NO, NO,  JUSTLEFT, "", "", crbk_rec.bank_name},
	{1, LIN, "br_name",	 6, 24, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Branch Name           ", "Enter Branch name.",
		 NO, NO,  JUSTLEFT, "", "", crbk_rec.branch_name},
	{1, LIN, "adr1",	 	8, 24, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Address Part 1        ", "Enter bank Address part one.",
		 NO, NO,  JUSTLEFT, "", "", crbk_rec.adr1},
	{1, LIN, "adr2",	 	9, 24, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Address Part 2        ", "Enter bank address part two. ",
		 NO, NO,  JUSTLEFT, "", "", crbk_rec.adr2},
	{1, LIN, "adr3",	 	10, 24, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Address Part 3        ", "Enter bank address part three. ",
		 NO, NO,  JUSTLEFT, "", "", crbk_rec.adr3},
	{1, LIN, "adr4",	 	11, 24, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Address Part 4        ", "Enter bank address part four. ",
		 NO, NO,  JUSTLEFT, "", "", crbk_rec.adr4},

	{2, LIN, "acct_name",	7, 24, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Account Name        ", "Enter bank account name.",
		 NO, NO,  JUSTLEFT, "", "", crbk_rec.acct_name},
	{2, LIN, "bank_no",	8, 24, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", "", "Bank Number           ", "Enter bank number. ",
		 NO, NO,  JUSTLEFT, "", "", crbk_rec.bank_no},
	{2, LIN, "bk_acno",	9, 24, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", "", "Bank Acct No          ", "Enter bank account number.",
		 NO, NO,  JUSTLEFT, "", "", crbk_rec.bank_acct_no},
	{2, LIN, "curcode",	10, 24, CHARTYPE,
		"UUU", "          ",
		" ", "", "Currency Code.        ", "Enter currency code. [SEARCH] Available.",
		YES, NO,  JUSTLEFT, "", "", crbk_rec.curr_code},
	{2, LIN, "curdesc",	10, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", pocrRec.description},
	{2, LIN, "nx_lodge_no",	11, 24, LONGTYPE,
		"NNNNNNNNNN", "          ",
		"0", "1", "Next Lodgement No. ", "Enter Next Lodgement Number.",
		YES, NO,  JUSTRIGHT, "", "", (char *)&local_rec.nx_lodge_no},
	{2, LIN, "clear_fee",	11, 65, MONEYTYPE,
		"NNN,NNN.NN", "          ",
		" ", "0", "Cheque Clearance Fee", "Enter Cheque clearance fee for Bank Currency.",
		YES, NO,  JUSTRIGHT, "", "", (char *)&crbk_rec.clear_fee},
	{2, LIN, "gl_bk_acct",	13, 24, CHARTYPE,
		GlMask, "          ",
		"0", "", "Bank Account      ", "Enter General ledger bank account number. [SEARCH] Available. ",
		YES, NO,  JUSTLEFT, "", "", crbk_rec.gl_bank_acct},
	{2, LIN, "gl_acct_desc",	13, 42, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.acct_desc1},
	{2, LIN, "gl_bk_chg",	14, 24, CHARTYPE,
		GlMask, "          ",
		"0", "", "Bank Charges Account", "Enter General ledger bank charges account. [SEARCH] Available. ",
		YES, NO,  JUSTLEFT, "", "", crbk_rec.gl_bank_chg},
	{2, LIN, "gl_chg_desc",	14, 42, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.chg_desc},
	{2, LIN, "gl_exch_var",	15, 24, CHARTYPE,
		GlMask, "          ",
		"0", "", "Exch Var Acct     ", "Enter General ledger Exchange Variation account. [SEARCH] Available. ",
		YES, NO,  JUSTLEFT, "", "", crbk_rec.gl_exch_var},
	{2, LIN, "gl_exch_desc",	15, 42, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.exch_desc},
	{2, LIN, "gl_bill_rec",	16, 24, CHARTYPE,
		GlMask, "          ",
		"0", "", "Bills Rec Acct.   ", "Enter General Ledger Bills Receivable Account.",
		YES, NO,  JUSTLEFT, "", "", crbk_rec.gl_bill_rec},
	{2, LIN, "gl_bill_desc",	16, 42, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.bill_desc},
	{2, LIN, "gl_fwd_rec",	17, 24, CHARTYPE,
		GlMask, "          ",
		"0", "", "Forward Receipts  ", "Enter General Ledger Forward Receipts Account.",
		YES, NO,  JUSTLEFT, "", "", crbk_rec.gl_fwd_rec},
	{2, LIN, "gl_fwd_desc",	17, 42, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.fwd_desc},
	{2, LIN, "gl_int_acct",	18, 24, CHARTYPE,
		GlMask, "          ",
		"0", "", "Interest Account  ", "Enter General Ledger Interest Account Account.",
		YES, NO,  JUSTLEFT, "", "", crbk_rec.gl_int_acct},
	{2, LIN, "gl_int_acct_desc",	18, 42, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.int_acct_desc},
	{2, LIN, "gl_pcash_acct",	19, 24, CHARTYPE,
		GlMask, "          ",
		"0", "", "Petty Cash Account  ", "Enter General Ledger Petty Cash Account Account.",
		YES, NO,  JUSTLEFT, "", "", crbk_rec.gl_pcash_acct},
	{2, LIN, "gl_pcash_acct_desc",	19, 42, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.pcash_acct_desc},

	{3, TAB, "code",	MAXLINES, 1, CHARTYPE,
		"UUU", "          ",
		" ", "", "Curr.", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.code},
	{3, TAB, "desc",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "C u r r e n c y   D e s c r i p t i o n.", " ",
		 NA, NO,  JUSTLEFT, "", "", bkcr_rec.description},
	{3, TAB, "factor",	 0, 0, DOUBLETYPE,
		"NNNN.NNNNNNNN", "          ",
		" ", "", "Bank Exch Rate", " ",
		YES, NO, JUSTRIGHT, "0", "9999.99999999", (char *)&local_rec.factor},
	{3, TAB, "updateDate",	 0, 1, EDATETYPE,
		"NN/NN/NN", "          ",
		" ", local_rec.systemDate, "Date Update.", " ",
		 NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.ldate_up},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy},
};

/*===========================
| Local function prototypes |
===========================*/
int		spec_valid		 (int);
int		LoadDetails	 	 (void);
int		Update			 (void);
int		heading			 (int);
int		CheckClass		 (void);
int		Getgl_chg		 (void);
int		Getgl_acct		 (void);
int		Getgl_exch		 (void);
int		Getgl_bill		 (void);
int		Getgl_fwd		 (void);
int		Getint_acct	 	 (void);
int		Getpcash_acct	 (void);
void	shutdown_prog	 (void);
void	OpenDB			 (void);
void	CloseDB			 (void);
void	crbk_search		 (char *);
void	LoadDefaults	 (void);
void	tab_other		 (int);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char *	argv [])
{

	SETUP_SCR (vars);


	sprintf (cr_mcurr,  "%-1.1s", get_env ("CR_MCURR"));
	sprintf (db_mcurr,  "%-1.1s", get_env ("DB_MCURR"));

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));

	/*----------------------------------------------
	| Modify screen layout for NON Multi-currency. |
	----------------------------------------------*/
	if (! (DB_MCURR || CR_MCURR))
	{
		FLD ("curcode") 		= ND;
		FLD ("curdesc") 		= ND;
		FLD ("gl_exch_var") 	= ND;
		FLD ("gl_exch_desc") 	= ND;
		vars [label ("nx_lodge_no")].row 	= 11;
		vars [label ("clear_fee")].row 		= 11;
		vars [label ("gl_bk_acct")].row 		= 13;
		vars [label ("gl_acct_desc")].row 	= 13;
		vars [label ("gl_bk_chg")].row 		= 14;
		vars [label ("gl_chg_desc")].row 	= 14;
		vars [label ("gl_bill_rec")].row 	= 15;
		vars [label ("gl_bill_desc")].row 	= 15;
		vars [label ("gl_fwd_rec")].row 		= 16;
		vars [label ("gl_fwd_desc")].row 	= 16;
		vars [label ("gl_int_acct")].row 	= 17;
		vars [label ("gl_int_acct_desc")].row  = 17;
		vars [label ("gl_pcash_acct")].row 	= 18;
		vars [label ("gl_pcash_acct_desc")].row  = 18;
	}
		
	tab_row = 8;
	tab_col = 2;

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
	SetSortArray (3, store, sizeof (struct storeRec));
#endif
	init_vars (1);

	OpenDB ();

	GL_SetMask (GlFormat);

	while (prog_exit == 0)
	{

		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		newBank		= FALSE;
		edit_mode	= FALSE;
		init_vars (1);
		init_vars (2);
		init_vars (3);
		lcount [3] = 0;
		search_ok = 1;

		/*------------------------------
		| Enter screen 1 linear input. |
		------------------------------*/
		heading (1);
		entry (1);
		if (prog_exit || restart) 
		{
			abc_unlock (crbk);
			continue;
		}

		if (newBank)
		{
			heading (2);
			entry (2);
			if (restart) 
			{
				abc_unlock (crbk);
				continue;
			}
		}

		heading (1);
		scn_display (1);

		if (!DB_MCURR && !CR_MCURR)
			no_edit (3);
		
		edit_all ();
		if (restart) 
		{
			abc_unlock (crbk);
			continue;
		}

		/*-------------------------
		| Update cr bank record.  |
		-------------------------*/
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

/*========================
| Program exit sequence. |
========================*/
void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (crbk, crbk_list, CRBK_NO_FIELDS, "crbk_id_no");
	open_rec (bkcr, bkcr_list, BKCR_NO_FIELDS, "bkcr_id_no");
	open_rec (blhd, blhd_list, BLHD_NO_FIELDS, "blhd_id_no");

	OpenPocr ();
	OpenGlmr ();
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (crbk);
	abc_fclose (bkcr);
	abc_fclose (blhd);

	GL_Close ();
	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
	/*--------------------------------------------
	| Validate Supplier Number And Allow Search. |
	--------------------------------------------*/
	if (LCHECK ("bank_id"))
	{
		if (SRCH_KEY)
		{
			crbk_search (temp_str);
			return (EXIT_SUCCESS);
		}

		if (!strcmp (crbk_rec.bank_id , "     "))
			return (EXIT_FAILURE);

		strcpy (crbk_rec.co_no,comm_rec.co_no);
		newBank = find_rec (crbk, &crbk_rec, COMPARISON, "w");
		newBlhd = TRUE;
		if (!newBank)
		{
			entry_exit = TRUE;

			LoadDetails ();

			strcpy (pocrRec.co_no, comm_rec.co_no);
			strcpy (pocrRec.code, crbk_rec.curr_code);
			cc = find_rec (pocr, &pocrRec, COMPARISON, "r");
			if (cc)
				sprintf (pocrRec.description, "%40.40s"," ");
		}
		else
			LoadDefaults ();

		return (EXIT_SUCCESS);
	}

	/*-------------------------------
	| Validate Currency Code Input. |
	-------------------------------*/
	if (LCHECK ("curcode")) 
	{
		if (FLD ("curcode") == ND)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			SearchPocr (comm_rec.co_no, temp_str);
			return (EXIT_SUCCESS);
		}
		cc = FindPocr (comm_rec.co_no, crbk_rec.curr_code, "r");
		if (cc)
		{
			errmess (ML (mlStdMess040));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		DSP_FLD ("curdesc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("nx_lodge_no"))
	{
		if (!newBlhd && local_rec.nx_lodge_no < orig_lodge)
		{
			print_mess (ML (mlCrMess125));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
			
		}
		return (EXIT_SUCCESS);
	}
	/*------------------------------------
	| Validate G/L Bank Charges Account. |
	-------------------------------------*/
	if (LCHECK ("gl_bk_chg"))
	{
		if (SRCH_KEY) 
		{
			SearchGlmr (comm_rec.co_no, temp_str, "F*P");
			return (EXIT_SUCCESS);
		}

		if (Getgl_chg ())
			return (EXIT_FAILURE);

		DSP_FLD ("gl_chg_desc");

		return (EXIT_SUCCESS);
	}
	
	/*---------------------------
	| Validate G/L Bank Account. |
	----------------------------*/
	if (LCHECK ("gl_bk_acct"))
	{
		if (SRCH_KEY) 
		{
			SearchGlmr (comm_rec.co_no, temp_str, "F*P");
			return (EXIT_SUCCESS);
		}

		if (Getgl_acct ())
			return (EXIT_FAILURE);

		DSP_FLD ("gl_acct_desc");

		return (EXIT_SUCCESS);
	}
	
	/*----------------------------
	| Validate Exch Var Account. |
	-----------------------------*/
	if (LCHECK ("gl_exch_var"))
	{
		if (FLD ("gl_exch_var") == ND)
			return (EXIT_SUCCESS);

		if (SRCH_KEY) 
		{
			SearchGlmr (comm_rec.co_no, temp_str, "F*P");
			return (EXIT_SUCCESS);
		}

		if (Getgl_exch ())
			return (EXIT_FAILURE);

		DSP_FLD ("gl_exch_desc");

		return (EXIT_SUCCESS);
	}
	/*------------------------------------
	| Validate Bills Receivable Account. |
	-------------------------------------*/
	if (LCHECK ("gl_bill_rec"))
	{
		if (SRCH_KEY) 
		{
			SearchGlmr (comm_rec.co_no, temp_str, "F*P");
			return (EXIT_SUCCESS);
		}

		if (Getgl_bill ())
			return (EXIT_FAILURE);

		DSP_FLD ("gl_bill_desc");

		return (EXIT_SUCCESS);
	}

	/*--------------------------------------
	| Validate Forward Receivable Account. |
	---------------------------------------*/
	if (LCHECK ("gl_fwd_rec"))
	{
		if (SRCH_KEY) 
		{
			SearchGlmr (comm_rec.co_no, temp_str, "F*P");
			return (EXIT_SUCCESS);
		}

		if (Getgl_fwd ())
			return (EXIT_FAILURE);

		DSP_FLD ("gl_fwd_desc");

		return (EXIT_SUCCESS);
	}
	/*----------------------------
	| Validate Interest Account. |
	----------------------------*/
	if (LCHECK ("gl_int_acct"))
	{
		if (SRCH_KEY) 
		{
			SearchGlmr (comm_rec.co_no, temp_str, "F*P");
			return (EXIT_SUCCESS);
		}

		if (Getint_acct ())
			return (EXIT_FAILURE);

		DSP_FLD ("gl_int_acct_desc");

		return (EXIT_SUCCESS);
	}

	/*------------------------------
	| Validate Petty Cash Account. |
	------------------------------*/
	if (LCHECK ("gl_pcash_acct"))
	{
		if (SRCH_KEY) 
		{
			SearchGlmr (comm_rec.co_no, temp_str, "F*P");
			return (EXIT_SUCCESS);
		}

		if (Getpcash_acct ())
			return (EXIT_FAILURE);

		DSP_FLD ("gl_pcash_acct_desc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("factor"))
	{
		local_rec.ldate_up = TodaysDate ();
		DSP_FLD ("updateDate");
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

/*--------------------
| Load bank details. |
--------------------*/
int
LoadDetails (void)
{
	/*---------------------------
	| get gl acct desc for scn 2 |
	---------------------------*/
	Getgl_chg ();
	Getgl_acct ();
	Getgl_exch ();
	Getgl_bill ();
	Getgl_fwd ();
	Getint_acct ();
	Getpcash_acct ();

	scn_set (3);
	lcount [3] = 0;
	
	strcpy (pocrRec.co_no, comm_rec.co_no);
	strcpy (pocrRec.code, "   ");
	cc = find_rec (pocr, &pocrRec, GTEQ, "r");
	while (!cc && !strcmp (pocrRec.co_no, comm_rec.co_no))
	{
		strcpy (bkcr_rec.co_no,comm_rec.co_no);
		strcpy (bkcr_rec.bank_id, crbk_rec.bank_id);
		sprintf (bkcr_rec.curr_code , pocrRec.code);
		cc = find_rec (bkcr, &bkcr_rec, COMPARISON, "r");
		if (cc)
		{
			/*---------------------
			| Setup local record. |
			---------------------*/
			strcpy (local_rec.code,pocrRec.code);
			strcpy (bkcr_rec.description,pocrRec.description);
			local_rec.factor = pocrRec.ex1_factor;
			store [lcount [3]].factors [0] = pocrRec.ex1_factor;
			store [lcount [3]].factors [1] = pocrRec.ex2_factor;
			store [lcount [3]].factors [2] = pocrRec.ex3_factor;
			store [lcount [3]].factors [3] = pocrRec.ex4_factor;
			store [lcount [3]].factors [4] = pocrRec.ex5_factor;
			store [lcount [3]].factors [5] = pocrRec.ex6_factor;
			store [lcount [3]].factors [6] = pocrRec.ex7_factor;
			local_rec.ldate_up = pocrRec.ldate_up;
		}
		else
		{
			/*---------------------
			| Setup local record. |
			---------------------*/
			strcpy (local_rec.code,bkcr_rec.curr_code);
			local_rec.factor = bkcr_rec.ex1_factor;
			store [lcount [3]].factors [0] = bkcr_rec.ex1_factor;
			store [lcount [3]].factors [1] = bkcr_rec.ex2_factor;
			store [lcount [3]].factors [2] = bkcr_rec.ex3_factor;
			store [lcount [3]].factors [3] = bkcr_rec.ex4_factor;
			store [lcount [3]].factors [4] = bkcr_rec.ex5_factor;
			store [lcount [3]].factors [5] = bkcr_rec.ex6_factor;
			store [lcount [3]].factors [6] = bkcr_rec.ex7_factor;
			local_rec.ldate_up = bkcr_rec.ldate_up;
		}
		putval (lcount [3]++);

		cc = find_rec (pocr, &pocrRec, NEXT, "r");
	}

	/*-----------------------------
	| Find next lodgement number. |
	-----------------------------*/
	strcpy (blhd_rec.co_no, comm_rec.co_no);
	sprintf (blhd_rec.bank_id, "%-5.5s", crbk_rec.bank_id);
	newBlhd = find_rec (blhd, &blhd_rec, COMPARISON, "u");

	if (!newBlhd)
	{
		local_rec.nx_lodge_no = blhd_rec.nx_lodge_no;
		orig_lodge = local_rec.nx_lodge_no;
	}

	/*-------------------------
	| Normal exit - return 0. |
	-------------------------*/
	scn_set (1);
	return (EXIT_SUCCESS);
}

/*---------------
| Update files. |
---------------*/
int
Update (void)
{
	int	i;
	int	newBkcr;

	clear ();
	strcpy (crbk_rec.stat_flag,"0");

	if (newBank)
	{
		cc = abc_add (crbk, &crbk_rec);
		if (cc) 
			file_err (cc, crbk, "DBADD");
	}
	else 
	{
		cc = abc_update (crbk, &crbk_rec);
		if (cc) 
			file_err (cc, crbk, "DBUPDATE");
	}

	/*--------------------
	| Add / Update blhd. |
	--------------------*/
	if (newBlhd)
	{
		strcpy (blhd_rec.co_no, comm_rec.co_no);
		sprintf (blhd_rec.bank_id, "%-5.5s", crbk_rec.bank_id);
		blhd_rec.nx_lodge_no = local_rec.nx_lodge_no;
		strcpy (blhd_rec.stat_flag, "0");
		cc = abc_add (blhd, &blhd_rec);
		if (cc)
			file_err (cc, blhd, "DBADD");
	}
	else
	{
		blhd_rec.nx_lodge_no = local_rec.nx_lodge_no;
		cc = abc_update (blhd, &blhd_rec);
		if (cc)
			file_err (cc, blhd, "DBUPDATE");
	}

	if (!DB_MCURR && !CR_MCURR)
		return (EXIT_SUCCESS);

	/*--------------------------------------------
	| Update currency details on tabular screen. |
	--------------------------------------------*/
	for (i = 0; i < lcount [3]; i++)
	{
		getval (i);

		strcpy (bkcr_rec.co_no, comm_rec.co_no);
		strcpy (bkcr_rec.bank_id, crbk_rec.bank_id);
		strcpy (bkcr_rec.curr_code, local_rec.code);

		newBkcr = find_rec (bkcr,&bkcr_rec,COMPARISON,"u");

		bkcr_rec.ex1_factor = local_rec.factor;
		bkcr_rec.ex2_factor = store [i].factors [1];
		bkcr_rec.ex3_factor = store [i].factors [2];
		bkcr_rec.ex4_factor = store [i].factors [3];
		bkcr_rec.ex5_factor = store [i].factors [4];
		bkcr_rec.ex6_factor = store [i].factors [5];
		bkcr_rec.ex7_factor = store [i].factors [6];

		bkcr_rec.ldate_up = local_rec.ldate_up;

		strcpy (bkcr_rec.stat_flag, "0");

		if (newBkcr)
			cc = abc_add (bkcr,&bkcr_rec);
		else
			cc = abc_update (bkcr,&bkcr_rec);
		if (cc)
			file_err (cc, bkcr, "DBUPDATE/DBADD");
	}
	return (EXIT_SUCCESS);
}

/*=========================================
| Search routine for Suppliers Bank File. |
=========================================*/
void
crbk_search (
 char *	key_val)
{
	_work_open (6,0,40);
	strcpy (crbk_rec.co_no, comm_rec.co_no);
	strcpy (crbk_rec.bank_id, key_val);
	cc = save_rec ("#Bank Id ","#Bank Name             ");
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
	if (!restart)
	{
		init_vars (1);
		return;
	}

	if (cc)
		return;

	strcpy (crbk_rec.co_no,comm_rec.co_no);
	strcpy (crbk_rec.bank_id,temp_str);
	cc = find_rec (crbk, &crbk_rec, GTEQ, "r");
	if (cc) 
		file_err (cc, "crbk", "DBFIND");
}

/*====================================================
| Display Screen Headings                            |
====================================================*/
int
heading (
	int		scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();
		rv_pr (ML (mlCrMess126),20,0,1);
		line_at (1,0,80);

		if (scn == 1)
		{
			box (0, 2, 80, 9);
			line_at (4,1,79);
			line_at (7,1,79);
		}
		else
		{
			if (scn == 2)
			{
				if (! (DB_MCURR || CR_MCURR))
				{
					box (0, 6, 80, 12);
					print_at (6, 32, ML (mlCrMess127));

					line_at (10,1,79);
					line_at (12,1,79);
					print_at (12, 27, ML (mlCrMess159));
				}
				else
				{
					box (0, 6, 80, 13);
					print_at (6, 32, ML (mlCrMess127));

					line_at (12,1,79);
					print_at (12, 27, ML (mlCrMess159));
				}
			}
			else
			{
#ifndef GVISION
				box (2, 7, 76, 13);
#endif
				print_at (7,25,ML (mlCrMess160));
			}
			print_at (2,0,ML (mlCrMess161), 
		 		crbk_rec.bank_id, crbk_rec.bank_name);
			print_at (3,29,"%s", crbk_rec.branch_name);
			print_at (4,29,"%s", crbk_rec.adr1);
			print_at (5,29,"%s", crbk_rec.adr2);
		}
		
		if (scn != 2)
			line_at (21,1,79);
		
		sprintf (err_str,ML (mlStdMess038), comm_rec.co_no,comm_rec.co_name);
		print_at (22,0,err_str);

		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
	else 
		abc_unlock (crbk);

	return (EXIT_SUCCESS);
}

int
CheckClass (void)
{
	if (glmrRec.glmr_class [2][0] != 'P')
		return print_err (ML (mlStdMess025));

	return (EXIT_SUCCESS);
}

int
Getgl_chg (void)
{  
	if (!strncmp (crbk_rec.gl_bank_chg, SixteenSpace,MAXLEVEL))
		return (EXIT_SUCCESS);

	strcpy (glmrRec.co_no,comm_rec.co_no);
	GL_FormAccNo (crbk_rec.gl_bank_chg, glmrRec.acc_no, 0);
	cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
	if (cc) 
	{
		errmess (ML (mlStdMess024));
		sleep (sleepTime);		
	    return (EXIT_FAILURE);
	}
	if (CheckClass ())
		return (EXIT_FAILURE);

	strcpy (local_rec.chg_desc,glmrRec.desc);
	return (EXIT_SUCCESS);
}

int
Getgl_acct (void)
{
	if (!strncmp (crbk_rec.gl_bank_acct, SixteenSpace,MAXLEVEL))
		return (EXIT_SUCCESS);

	strcpy (glmrRec.co_no,comm_rec.co_no);
	GL_FormAccNo (crbk_rec.gl_bank_acct, glmrRec.acc_no, 0);
	cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
	if (cc) 
	{
	    errmess (ML (mlStdMess024));
	    sleep (sleepTime);		
	    return (EXIT_FAILURE);
	}
	if (CheckClass ())
		return (EXIT_FAILURE);

	strcpy (local_rec.acct_desc1,glmrRec.desc);
	return (EXIT_SUCCESS);
}

int
Getgl_exch (void)
{
	if (!strncmp (crbk_rec.gl_exch_var, SixteenSpace,MAXLEVEL))
		return (EXIT_SUCCESS);

	strcpy (glmrRec.co_no,comm_rec.co_no);
	GL_FormAccNo (crbk_rec.gl_exch_var, glmrRec.acc_no, 0);
	cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
	if (cc) 
	{
	    errmess (ML (mlStdMess024));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}
	if (CheckClass ())
		return (EXIT_FAILURE);

	strcpy (local_rec.exch_desc,glmrRec.desc);
	return (EXIT_SUCCESS);
}

int
Getgl_bill (void)
{
	if (!strncmp (crbk_rec.gl_bill_rec, SixteenSpace,MAXLEVEL))
		return (EXIT_SUCCESS);

	strcpy (glmrRec.co_no,comm_rec.co_no);
	GL_FormAccNo (crbk_rec.gl_bill_rec, glmrRec.acc_no, 0);
	cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
	if (cc) 
	{
	    errmess (ML (mlStdMess024));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}
	if (CheckClass ())
		return (EXIT_FAILURE);

	strcpy (local_rec.bill_desc,glmrRec.desc);
	return (EXIT_SUCCESS);
}

int
Getgl_fwd (void)
{
	if (!strncmp (crbk_rec.gl_fwd_rec, SixteenSpace,MAXLEVEL))
		return (EXIT_SUCCESS);

	strcpy (glmrRec.co_no,comm_rec.co_no);
	GL_FormAccNo (crbk_rec.gl_fwd_rec, glmrRec.acc_no, 0);
	cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
	if (cc) 
	{
	    errmess (ML (mlStdMess024));
		sleep (sleepTime);		
		return (EXIT_FAILURE);
	}
	if (CheckClass ())
		return (EXIT_FAILURE);

	strcpy (local_rec.fwd_desc,glmrRec.desc);
	return (EXIT_SUCCESS);
}

int
Getint_acct (void)
{
	if (!strncmp (crbk_rec.gl_int_acct, SixteenSpace,MAXLEVEL))
		return (EXIT_SUCCESS);

	strcpy (glmrRec.co_no,comm_rec.co_no);
	GL_FormAccNo (crbk_rec.gl_int_acct, glmrRec.acc_no, 0);
	cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
	if (cc) 
	{
	    errmess (ML (mlStdMess024));
		sleep (sleepTime);		
		return (EXIT_FAILURE);
	}
	if (CheckClass ())
		return (EXIT_FAILURE);

	strcpy (local_rec.int_acct_desc,glmrRec.desc);
	return (EXIT_SUCCESS);
}

int
Getpcash_acct (void)
{
	if (!strncmp (crbk_rec.gl_pcash_acct, SixteenSpace,MAXLEVEL))
		return (EXIT_SUCCESS);

	strcpy (glmrRec.co_no,comm_rec.co_no);
	GL_FormAccNo (crbk_rec.gl_pcash_acct, glmrRec.acc_no, 0);
	cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
	if (cc) 
	{
	    errmess (ML (mlStdMess024));
		sleep (sleepTime);		
		return (EXIT_FAILURE);
	}
				
	if (CheckClass ())
		return (EXIT_FAILURE);

	strcpy (local_rec.pcash_acct_desc,glmrRec.desc);
	return (EXIT_SUCCESS);
}

void
LoadDefaults (void)
{
	scn_set (3);
	lcount [3] = 0;
	
	strcpy (pocrRec.co_no, comm_rec.co_no);
	strcpy (pocrRec.code, "   ");
	cc = find_rec (pocr, &pocrRec, GTEQ, "r");
	while (!cc && !strcmp (pocrRec.co_no, comm_rec.co_no))
	{
		/*---------------------
		| Setup local record. |
		---------------------*/
		strcpy (local_rec.code,pocrRec.code);
		strcpy (bkcr_rec.description,pocrRec.description);
		local_rec.factor = pocrRec.ex1_factor;
		store [lcount [3]].factors [0] = pocrRec.ex1_factor;
		store [lcount [3]].factors [1] = pocrRec.ex2_factor;
		store [lcount [3]].factors [2] = pocrRec.ex3_factor;
		store [lcount [3]].factors [3] = pocrRec.ex4_factor;
		store [lcount [3]].factors [4] = pocrRec.ex5_factor;
		store [lcount [3]].factors [5] = pocrRec.ex6_factor;
		store [lcount [3]].factors [6] = pocrRec.ex7_factor;
		local_rec.ldate_up = pocrRec.ldate_up;
		putval (lcount [3]++);

		cc = find_rec (pocr, &pocrRec, NEXT, "r");
	}
	scn_set (1);
}

void
tab_other (
 int line_no)
{
	if (line_no >= lcount [3])
		FLD ("factor") = NA;
	else
		FLD ("factor") = YES;
}

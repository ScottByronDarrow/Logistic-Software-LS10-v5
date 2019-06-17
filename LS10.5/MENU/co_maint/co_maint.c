/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: co_maint.c,v 5.3 2002/04/04 09:44:10 robert Exp $
|  Program Name  : (co_maint.c)
|  Program Desc  : (Company Maintenance)
|---------------------------------------------------------------------|
|  Date Written  : (10/05/86)      | Author       : Scott Darrow.     |
|---------------------------------------------------------------------|
| $Log: co_maint.c,v $
| Revision 5.3  2002/04/04 09:44:10  robert
| SC00769 - Updated to use the correct default environment path on LS10-GUI
|
| Revision 5.2  2001/08/09 05:13:19  scott
| Updated to use FinishProgram ();
|
| Revision 5.1  2001/08/06 23:32:10  scott
| RELEASE 5.0
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: co_maint.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MENU/co_maint/co_maint.c,v 5.3 2002/04/04 09:44:10 robert Exp $";

#include	<pslscr.h>
#include	<license2.h>
#include	<GlUtils.h>
#include	<pinn_env.h>
#include	<ml_std_mess.h>
#include	<ml_menu_mess.h>

#include	"schema"

#define	GST		 (gst_applies [0] == 'Y')

	struct	commRecord		comm_rec;
	struct	comrRecord		comr_rec;
	struct	esmrRecord		esmr_rec;
	struct	exsiRecord		exsi_rec;

	char	gst_applies	 [2];
	char	gst_code	 [4];
	char	gst_amt_prmt [40];
	char	gst_ird_prmt [40];

	char	*data	= "data";

	char	*Spaces	=	"                                        ";

	int		sk_dbprinum;

	char	CoCurrCode [12];

struct	DES_REC	des_rec;
struct	LIC_REC	lic_rec;
PinnEnv	env_rec;

/*===========================
| Special fields and flags. |
===========================*/
int		new_comp = 0;
char	*prog_path;

/*============================
| Local & Screen Structures. |
============================*/
struct
{
	char	fin_acct [FORM_LEN + 1];
	char	fin_desc [26];
	char	nfn_acct [FORM_LEN + 1];
	char	nfn_desc [26];
	char	pl_app_acc [FORM_LEN + 1];
	char	pl_app_desc [26];
	char	prev_co [3];
	char	cur_co [3];
	char	poText [3][51];
	char	dummy [11];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "compno",	 3, 2, CHARTYPE,
		"AA", "          ",
		" ", "", "Company Number       ", "Enter Company Number. [SEARCH] available ",
		 NE, NO, JUSTRIGHT, "1", "99", comr_rec.co_no},
	{1, LIN, "coname",	 4, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Company name         ", "Enter company name. ",
		YES, NO,  JUSTLEFT, "", "", comr_rec.co_name},
	{1, LIN, "cosname",	 5, 2, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", "", "Short Name           ", " Enter company short name/acronym.",
		YES, NO,  JUSTLEFT, "", "", comr_rec.co_short_name},
	{1, LIN, "addr1",	 6, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Address 1            ", "Enter company address part 1",
		 NO, NO,  JUSTLEFT, "", "", comr_rec.co_adr1},
	{1, LIN, "addr2",	 7, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Address 2            ", " Enter company address part 2",
		 NO, NO,  JUSTLEFT, "", "", comr_rec.co_adr2},
	{1, LIN, "addr3",	 8, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Address 3            ", "Enter company address part 3",
		 NO, NO,  JUSTLEFT, "", "", comr_rec.co_adr3},
	{1, LIN, "po_sic1",	10, 2, INTTYPE,
		"NNN", "          ",
		" ", "0",    "P/O Special inst.1   ", "Enter 0-999 or use [ SEARCH ]",
		YES, NO,  JUSTLEFT, "0", "999", (char *)&comr_rec.po_sic1},
	{1, LIN, "po_sic1_text",	 10, 27, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.poText [0]},
	{1, LIN, "po_sic2",	11, 2, INTTYPE,
		"NNN", "          ",
		" ", "0",    "P/O Special inst.2   ", "Enter 0-999 or use [ SEARCH ]",
		YES, NO,  JUSTLEFT, "0", "999", (char *)&comr_rec.po_sic2},
	{1, LIN, "po_sic2_text",	 11, 27, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.poText [1]},
	{1, LIN, "po_sic3",	12, 2, INTTYPE,
		"NNN", "          ",
		" ", "0",    "P/O Special inst.3   ", "Enter 0-999 or use [ SEARCH ]",
		YES, NO,  JUSTLEFT, "0", "999", (char *)&comr_rec.po_sic3},
	{1, LIN, "po_sic3_text",	 12, 27, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.poText [2]},
	{1, LIN, "env_name",	14, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ",    "Environment      ","Enter Company Specific Environment Filename or <ENTER> to Default ",
		NO, NO,  JUSTLEFT, "", "", comr_rec.env_name},
	{1, LIN, "masterBr",	 16, 2, CHARTYPE,
		"AA", "          ",
		" ", " ", "Master Branch No.    ", "Enter Master Branch Number. [SEARCH] available ",
		 YES, NO, JUSTRIGHT, "0", "99", comr_rec.master_br},
	{1, LIN, "masterBrName",	 17, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Master Branch Name   ", "",
		NA, NO,  JUSTLEFT, "", "", esmr_rec.est_name},
	{1, LIN, "base_curr",	19,2,CHARTYPE,
		"UUU", "          ",
		" ", "", "Base Currency      ", "Enter base currency for this company",
		 NO, NO,  JUSTLEFT, "", "", comr_rec.base_curr},
	{1, LIN, "curr_desc",	20, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Base Currency Desc ", "",
		 NA, NO,  JUSTLEFT, "", "", pocrRec.description},
	{2, LIN, "gst_amt",	3, 2, FLOATTYPE,
		"NN.NN", "          ",
		" ", "", gst_amt_prmt, " ",
		 NO, NO,  JUSTLEFT, "", "", (char *)&comr_rec.gst_rate},
	{2, LIN, "gst_ird",	3, 43, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", "", gst_ird_prmt, " ",
		 NO, NO,  JUSTLEFT, "", "", comr_rec.gst_ird_no},
	{2, LIN, "sur_amt",	 4, 2, MONEYTYPE,
		"NNNNN.NN", "          ",
		" ", "",     "Surcharge Amount   ", "Input Small Order Surcharge Amount. ",
		 NO, NO,  JUSTLEFT, "", "", (char *)&comr_rec.sur_amt},
	{2, LIN, "cur_cutoff",	 4, 43, MONEYTYPE,
		"NNNNN.NN", "          ",
		" ", "",     "Surcharge Cutoff   ", "Input Small Order Surcharge Cutoff. ",
		 NO, NO,  JUSTLEFT, "", "", (char *)&comr_rec.sur_cof},
	{2, LIN, "frt_min",	 5, 2, MONEYTYPE,
		"NNNNN.NN", "          ",
		" ", "",     "Min freight Amount ", "Input Minimum amount of freight charge.",
		 NO, NO,  JUSTLEFT, "", "", (char *)&comr_rec.frt_min_amt},
	{2, LIN, "frt_weight",	 5, 43, FLOATTYPE,
		"NNNNN.NNNN", "          ",
		" ", "",     "Min Wgt. for Item  ", "Input Minimum weight for each inventory item.",
		 NO, NO,  JUSTLEFT, "", "", (char *)&comr_rec.frt_mweight},
	{2, LIN, "re_stockpc",	6, 2, FLOATTYPE,
		"NNN.NN", "          ",
		" ", "0.00", "Restocking %       ", "Enter % charged on credit notes for re-stocking fee",
		 NO, NO,  JUSTLEFT, "", "", (char *)&comr_rec.restock_pc},
	{2, LIN, "int_rate",	7, 2, FLOATTYPE,
		"NNN.NNN", "          ",
		" ", "0.00", "Int Rate Charged   ", "Enter Current interest rate charged to customers. ",
		 NO, NO,  JUSTLEFT, "", "", (char *)&comr_rec.int_rate},
	{2, LIN, "contingency",	 8, 2, DOUBLETYPE,
		"NNN.NN", "          ",
		" ", "",     "Freight Cont. %    ", "Input freight contingency percentage.",
		 NO, NO,  JUSTLEFT, "", "", (char *)&comr_rec.contingency},
	{2, LIN, "Price1",	10, 2, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", "", "Price Desc No. 1   ", "Enter the description for stock price #1 ",
		 NO, NO,  JUSTLEFT, "", "", comr_rec.price1_desc},
	{2, LIN, "Price2",	11, 2, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", "", "Price Desc No. 2   ", "Enter the description for stock price #2 ",
		 NO, NO,  JUSTLEFT, "", "", comr_rec.price2_desc},
	{2, LIN, "Price3",	12, 2, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", "", "Price Desc No. 3   ", "Enter the description for stock price #3 ",
		 NO, NO,  JUSTLEFT, "", "", comr_rec.price3_desc},
	 {2, LIN, "Price4",	13, 2, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", "", "Price Desc No. 4   ", "Enter the description for stock price #4 ",
		 NO, NO,  JUSTLEFT, "", "", comr_rec.price4_desc},
	{2, LIN, "Price5",	14, 2, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", "", "Price Desc No. 5   ", "Enter the description for stock price #5 ",
		 NO, NO,  JUSTLEFT, "", "", comr_rec.price5_desc},
	{2, LIN, "Price6",	15, 2, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", "", "Price Desc No. 6   ", "Enter the description for stock price #6 ",
		 NO, NO,  JUSTLEFT, "", "", comr_rec.price6_desc},
	{2, LIN, "Price7",	16, 2, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", "", "Price Desc No. 7   ", "Enter the description for stock price #7 ",
		 NO, NO,  JUSTLEFT, "", "", comr_rec.price7_desc},
	{2, LIN, "Price8",	17, 2, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", "", "Price Desc No. 8   ", "Enter the description for stock price #8 ",
		 NO, NO,  JUSTLEFT, "", "", comr_rec.price8_desc},
	{2, LIN, "Price9",	18, 2, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", "", "Price Desc No. 9   ", "Enter the description for stock price #9 ",
		 NO, NO,  JUSTLEFT, "", "", comr_rec.price9_desc},
	{3, LIN, "cons_co",	3, 2, INTTYPE,
		"N", "          ",
		" ", "0", "Consolidation Co.  ", "Enter 1 if this company is to be the G/L Consolidation Company",
		 NO, NO,  JUSTLEFT, "0", "1", (char *)&comr_rec.consolidate},
	{3, LIN, "gl_yend_date",	4, 2, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "", "G/L Year End       ", "General Ledger Year End Date. This is the fiscal year end date.",
		YES, NO, JUSTRIGHT, "", "", (char *)&comr_rec.yend_date},
	{3, LIN, "stmt_date",	5, 2, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "", "Statement. Cutoff  ", "Last statement cuttoff date.",
		YES, NO, JUSTRIGHT, "", "", (char *)&comr_rec.stmt_date},
	{3, LIN, "fiscal",	6, 2, INTTYPE,
		"NN", "          ",
		" ", "0",    "Fiscal Period      ", "Enter 1 (Jan) to 12 (Dec)",
		YES, NO,  JUSTLEFT, "1", "12", (char *)&comr_rec.fiscal},
	{3, LIN, "gl_fin",	8, 2, CHARTYPE,
		"NNNNNNNNNNNNNNNNN", "          ",
		" ", " ",    "Financial G/L Susp ", "Financial G/L Suspense Account",
		YES, NO,  JUSTLEFT, "0123456789*-", "", local_rec.fin_acct},
	{3, LIN, "gl_fin_desc",	 8, 45, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.fin_desc},
	{3, LIN, "gl_nfn",	9, 2, CHARTYPE,
		"NNNNNNNNNNNNNNNNN", "          ",
		" ", " ",    "NFn. G/L suspense  ", "Non-financial G/L Suspense Account",
		YES, NO,  JUSTLEFT, "0123456789*-", "", local_rec.nfn_acct},
	{3, LIN, "gl_nfn_desc",	 9, 45, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.nfn_desc},
	{3, LIN, "pl_acc",	10, 2, CHARTYPE,
		"NNNNNNNNNNNNNNNNN", "          ",
		" ", " ",    "P&L Appropriation. ", "Profit&Loss Appropriation Account",
		YES, NO,  JUSTLEFT, "0123456789*-", "", local_rec.pl_app_acc},
	{3, LIN, "pl_app_desc",	 10, 45, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.pl_app_desc},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

int		main			 (int argc, char * argv []);
void	shutdown_prog	 (void);
void	OpenDB			 (void);
void	CloseDB			 (void);
int		Update			 (void);
void	SrchComr		 (char *);
void	SrchEsmr		 (char *);
void	SrchExsi		 (char *);
void	SrchPocr		 (char *);
int		heading			 (int);

extern	int		TruePosition;

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char * argv [])
{
	int		i;
	char	ws_label [7];
	char	*sptr;

	TruePosition	=	TRUE;
#ifndef GVISION
	prog_path = getenv ("PROG_PATH");
#else
	prog_path = ServerPROG_PATH ();
#endif

	sprintf (gst_applies, "%-1.1s", get_env ("GST"));

	sprintf (gst_code, "%-3.3s", get_env ("GST_TAX_NAME"));
	
	if (GST)
	{
		sprintf (gst_amt_prmt, "%-3.3s %%              ", gst_code);
		sprintf (gst_ird_prmt, "%-3.3s No             ", gst_code);
	}
	else
	{
		sprintf (gst_amt_prmt, "%-3.3s %%              ", "Tax");
		sprintf (gst_ird_prmt, "%-3.3s No             ", "Tax");
	}

	SETUP_SCR (vars);

	init_scr ();
	set_tty ();

	sptr = chk_env ("SK_DBPRINUM");
	if (sptr == (char *)0)
		sk_dbprinum = 5;
	else
	{
		sk_dbprinum = atoi (sptr);
		if (sk_dbprinum > 9 || sk_dbprinum < 1)
			sk_dbprinum = 9;
	}

	for (i = 1; i < 10; i++)
	{
		if (i > sk_dbprinum)
		{
			sprintf (ws_label, "Price%1d", i);
			FLD (ws_label) = ND;
		}
	}

	OpenDB ();

	if (FindGlct ())
	{
		print_mess (ML (mlMenuMess013));

		sleep (sleepTime);
		clear_mess ();
		shutdown_prog ();
		return (EXIT_SUCCESS);
	}
	vars [label ("gl_fin")].mask = GL_SetAccWidth (" 1", TRUE);
	vars [label ("gl_nfn")].mask = vars [label ("gl_fin")].mask;
	vars [label ("pl_acc")].mask = vars [label ("gl_fin")].mask;

#ifndef GVISION
	ser_msg (lc_check (&des_rec, &lic_rec), &lic_rec, FALSE);
#endif	/* GVISION */

	_set_masks ("co_maint.s");

	init_vars (1);

	strcpy (local_rec.prev_co, "  ");

	while (prog_exit == 0)
	{
		entry_exit	= FALSE;
		restart		= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		search_ok	= TRUE;
		new_comp	= 0;
		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		if (new_comp == 1)
		{
			heading (2);
			entry (2);
			if (prog_exit || restart)
				continue;

			heading (3);
			entry (3);
			if (prog_exit || restart)
				continue;
		}
		else
			scn_display (1);
		edit_all ();
		if (!restart)
			Update ();
	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

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
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");
	open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (exsi, exsi_list, EXSI_NO_FIELDS, "exsi_id_no");

	OpenGlmr ();
	OpenPocr ();
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (comr);
	abc_fclose (esmr);
	abc_fclose (exsi);
	GL_Close ();
	abc_dbclose (data);
}

int
spec_valid (
 int	field)
{
	char	env_name [61];

	if (LCHECK ("env_name"))
	{
		if (dflt_used)
		{
			sprintf (comr_rec.env_name,"%s/BIN/%s",
				 (prog_path != (char *)0) ? prog_path:"/usr/LS10.5", "LOGISTIC");
			DSP_FLD ("env_name");

			/*----------------------
			| Warn user of change. |
			----------------------*/
			if (prog_status != ENTRY)
			{
				print_mess (ML (mlMenuMess014));

				sleep (sleepTime);
				clear_mess ();
			}
			return (EXIT_SUCCESS);
		}

		clip (comr_rec.env_name) ;


#ifndef GVISION
		if (access (comr_rec.env_name, 0))
		{
			print_mess (ML (mlStdMess145));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
#endif	/* GVISION */

		/*=========================
		| make sure not directory |
		=========================*/
		sprintf (env_name, "%s/%s", comr_rec.env_name, "..");

#ifndef GVISION
		if (!access (env_name, 0))
		{
			print_mess (ML (mlStdMess145));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
#endif	/* GVISION */

		/*----------------------
		| Warn user of change. |
		----------------------*/
		if (prog_status != ENTRY)
		{
			print_mess (ML (mlMenuMess014));
			sleep (sleepTime);
			clear_mess ();
		}

		return (EXIT_SUCCESS);
	}
	/*--------------------------
	| Validate Company Number. |
	--------------------------*/
	if (LCHECK ("compno"))
	{
		if (comr_rec.co_no [0] == '0')
		{
			print_mess (ML ("Sorry, Company 1-9 must be blank + digit not zero and digit."));

			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (SRCH_KEY)
		{
			SrchComr (temp_str);
			return (EXIT_SUCCESS);
		}
		new_comp = 0;
		strcpy (local_rec.cur_co, comr_rec.co_no);
		cc = find_rec (comr, &comr_rec, COMPARISON, "w");
		if (cc)
		{
			new_comp = 1;
			return (EXIT_SUCCESS);
		}

		strcpy (local_rec.fin_acct, "000000000");
		strcpy (local_rec.nfn_acct, "000000000");
		strcpy (local_rec.pl_app_acc, "000000000");
		abc_selfield (glmr, "glmr_hhmr_hash");
		cc = find_hash (glmr, &glmrRec, EQUAL, "r", comr_rec.fin_susp);
		if (!cc)
		{
			strcpy (local_rec.fin_acct, glmrRec.acc_no);
			strcpy (local_rec.fin_desc, glmrRec.desc);
		}

		cc = find_hash (glmr, &glmrRec, EQUAL, "r", comr_rec.nfn_susp);
		if (!cc)
		{
			strcpy (local_rec.nfn_acct, glmrRec.acc_no);
			strcpy (local_rec.nfn_desc, glmrRec.desc);
		}

		cc = find_hash (glmr, &glmrRec, EQUAL, "r", comr_rec.pl_app_acc);
		if (!cc)
		{
			strcpy (local_rec.pl_app_acc, glmrRec.acc_no);
			strcpy (local_rec.pl_app_desc, glmrRec.desc);
		}
		GL_FormAccNo (local_rec.fin_acct, glmrRec.acc_no, 0);
		GL_FormAccNo (local_rec.nfn_acct, glmrRec.acc_no, 0);
		GL_FormAccNo (local_rec.pl_app_acc, glmrRec.acc_no, 0);
		abc_selfield (glmr, "glmr_id_no");

		strcpy (pocrRec.co_no,comr_rec.co_no);
		sprintf (pocrRec.code,"%-3.3s",comr_rec.base_curr);
		cc = find_rec (pocr, &pocrRec, COMPARISON, "r");
		if (cc)
			strcpy (pocrRec.description, " ");

		strcpy (esmr_rec.co_no,comr_rec.co_no);
		strcpy (esmr_rec.est_no,comr_rec.master_br);
		cc = find_rec (esmr,&esmr_rec,COMPARISON,"r");
		if (cc)
			strcpy (esmr_rec.est_name, " ");

		strcpy (exsi_rec.co_no,comr_rec.co_no);
		exsi_rec.inst_code = comr_rec.po_sic1;
		cc = find_rec ("exsi",&exsi_rec,COMPARISON,"r");
		if (cc)
			strcpy (local_rec.poText [0], " ");
		else
			sprintf (local_rec.poText [0], "%-50.50s", exsi_rec.inst_text);

		strcpy (exsi_rec.co_no,comr_rec.co_no);
		exsi_rec.inst_code = comr_rec.po_sic2;
		cc = find_rec ("exsi",&exsi_rec,COMPARISON,"r");
		if (cc)
			strcpy (local_rec.poText [1], " ");
		else
			sprintf (local_rec.poText [1], "%-50.50s", exsi_rec.inst_text);

		strcpy (exsi_rec.co_no,comr_rec.co_no);
		exsi_rec.inst_code = comr_rec.po_sic3;
		cc = find_rec ("exsi",&exsi_rec,COMPARISON,"r");
		if (cc)
			strcpy (local_rec.poText [2], " ");
		else
			sprintf (local_rec.poText [2], "%-50.50s", exsi_rec.inst_text);

		entry_exit = TRUE;

		return (EXIT_SUCCESS);
	}
	if (LCHECK ("coname"))
	{
		if (!strncmp (comr_rec.co_name, Spaces, 40))
		{
			print_mess (ML (mlMenuMess015));

			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("cosname"))
	{
		if (!strncmp (comr_rec.co_short_name, Spaces, 15))
		{
			print_mess (ML (mlMenuMess016));

			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("masterBr"))
	{
		if (SRCH_KEY)
		{
			SrchEsmr (temp_str);
			return (EXIT_SUCCESS);
		}
		if (dflt_used)
		{
			strcpy (esmr_rec.est_name, " ");
			DSP_FLD ("masterBrName");
			return (EXIT_SUCCESS);
		}

		strcpy (esmr_rec.co_no,comr_rec.co_no);
		strcpy (esmr_rec.est_no,comr_rec.master_br);
		cc = find_rec (esmr,&esmr_rec,COMPARISON,"r");
		if (cc)
		{
			/*-----------------------
			| Branch No. Not found. |
			-----------------------*/
			print_mess (ML (mlStdMess073));
			return (EXIT_FAILURE);
		}
		DSP_FLD ("masterBrName");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("gl_fin"))
	{
		if (SRCH_KEY)
		{
			SearchGlmr_F (comr_rec.co_no, temp_str, "F*P");
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			strcpy (local_rec.fin_acct, "000000000");
			GL_FormAccNo (local_rec.fin_acct, glmrRec.acc_no, 0);
			comr_rec.fin_susp = 0L;
			return (EXIT_SUCCESS);
		}

		strcpy (glmrRec.co_no, comr_rec.co_no);
		if (GL_FormAccNo (local_rec.fin_acct, glmrRec.acc_no, 0))
			return (EXIT_FAILURE);

		cc = find_rec (glmr, &glmrRec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess186));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (glmrRec.glmr_class [0][0] != 'F' || glmrRec.glmr_class [2][0] != 'P')
		{
			print_mess (ML (mlMenuMess017));

			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		comr_rec.fin_susp = glmrRec.hhmr_hash;
		strcpy (local_rec.fin_desc, glmrRec.desc);
		DSP_FLD ("gl_fin_desc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("gl_nfn"))
	{
		if (SRCH_KEY)
		{
			SearchGlmr_F (comr_rec.co_no, temp_str, "N*P");
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			strcpy (local_rec.nfn_acct, "000000000");
			GL_FormAccNo (local_rec.nfn_acct, glmrRec.acc_no, 0);
			comr_rec.nfn_susp = 0L;
			return (EXIT_SUCCESS);
		}

		strcpy (glmrRec.co_no, comr_rec.co_no);
		if (GL_FormAccNo (local_rec.nfn_acct, glmrRec.acc_no, 0))
			return (EXIT_FAILURE);

		cc = find_rec (glmr, &glmrRec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess186));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (glmrRec.glmr_class [0][0] != 'N' || glmrRec.glmr_class [2][0] != 'P')
		{
			print_mess (ML (mlMenuMess018));

			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		comr_rec.nfn_susp = glmrRec.hhmr_hash;
		strcpy (local_rec.nfn_desc, glmrRec.desc);
		DSP_FLD ("gl_nfn_desc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("pl_acc"))
	{
		if (SRCH_KEY)
		{
			SearchGlmr_F (comr_rec.co_no, temp_str, "F*P");
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			strcpy (local_rec.pl_app_acc, "000000000");
			GL_FormAccNo (local_rec.pl_app_acc, glmrRec.acc_no, 0);
			comr_rec.pl_app_acc = 0L;
			return (EXIT_SUCCESS);
		}

		strcpy (glmrRec.co_no, comr_rec.co_no);
		if (GL_FormAccNo (local_rec.pl_app_acc, glmrRec.acc_no, 0))
			return (EXIT_FAILURE);

		cc = find_rec (glmr, &glmrRec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess186));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (glmrRec.glmr_class [0][0] != 'F' || glmrRec.glmr_class [2][0] != 'P')
		{
			print_mess (ML (mlMenuMess017));

			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		comr_rec.pl_app_acc = glmrRec.hhmr_hash;
		strcpy (local_rec.pl_app_desc, glmrRec.desc);
		DSP_FLD ("pl_app_desc");

		return (EXIT_SUCCESS);
	}

	/*-----------------------------
	| Validate Shipment method (s) |
	-----------------------------*/
	if (LCHECK ("po_sic1"))
	{
		if (dflt_used)
		{
			comr_rec.po_sic1 = 0;
			strcpy (local_rec.poText [0], " ");
			DSP_FLD ("po_sic1_text");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchExsi (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (exsi_rec.co_no,comr_rec.co_no);
		exsi_rec.inst_code = comr_rec.po_sic1;
		cc = find_rec ("exsi",&exsi_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess (ML (mlStdMess184));

			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		sprintf (local_rec.poText [0], "%-50.50s", exsi_rec.inst_text);
		DSP_FLD ("po_sic1_text");
		return (EXIT_SUCCESS);
	}
	/*-----------------------------
	| Validate Shipment method (s) |
	-----------------------------*/
	if (LCHECK ("po_sic2"))
	{
		if (dflt_used)
		{
			comr_rec.po_sic2 = 0;
			strcpy (local_rec.poText [1], " ");
			DSP_FLD ("po_sic2_text");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchExsi (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (exsi_rec.co_no,comr_rec.co_no);
		exsi_rec.inst_code = comr_rec.po_sic2;

		cc = find_rec ("exsi",&exsi_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess (ML (mlStdMess184));

			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		sprintf (local_rec.poText [1], "%-50.50s", exsi_rec.inst_text);
		DSP_FLD ("po_sic2_text");
		return (EXIT_SUCCESS);
	}
	/*-----------------------------
	| Validate Shipment method (s) |
	-----------------------------*/
	if (LCHECK ("po_sic3"))
	{
		if (dflt_used)
		{
			comr_rec.po_sic3 = 0;
			strcpy (local_rec.poText [2], " ");
			DSP_FLD ("po_sic3_text");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchExsi (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (exsi_rec.co_no,comr_rec.co_no);
		exsi_rec.inst_code = comr_rec.po_sic3;

		cc = find_rec ("exsi",&exsi_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess (ML (mlStdMess184));

			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		sprintf (local_rec.poText [2], "%-50.50s", exsi_rec.inst_text);
		DSP_FLD ("po_sic3_text");
		return (EXIT_SUCCESS);
	}

	/*===============
	| Base currency |
	===============*/
	if (LCHECK ("base_curr"))
	{
		if (dflt_used)
		{
			sprintf (pocrRec.code,"%-3.3s","   ");
			return (EXIT_SUCCESS);
		}
		if (SRCH_KEY)
			SrchPocr (temp_str);

		strcpy (pocrRec.co_no, comr_rec.co_no);
		sprintf (pocrRec.code,"%-3.3s",comr_rec.base_curr);
		cc = find_rec (pocr, &pocrRec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess040));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		DSP_FLD ("curr_desc");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

int
Update (void)
{
	clear ();

	/*----------------------------------
	| Update existing company record . |
	----------------------------------*/
	if (new_comp == 0)
	{
		/*----------------------------------------------------------
		| Updated currency code for standard environment variable. |
		----------------------------------------------------------*/
		if (!comr_rec.consolidate)
		{
			strcpy (CoCurrCode, "CURR_CODE");
			sprintf (env_rec.env_name,"%-15.15s", CoCurrCode);
			sprintf (env_rec.env_value,"%-30.30s", comr_rec.base_curr);
			sprintf (env_rec.env_desc,"%-70.70s",  pocrRec.description);
			put_env (env_rec.env_name,env_rec.env_value,env_rec.env_desc);
		}

		sprintf (CoCurrCode, "CURR_CODE%2.2s", comr_rec.co_no);
		sprintf (env_rec.env_name,"%-15.15s", CoCurrCode);
		sprintf (env_rec.env_value,"%-30.30s", comr_rec.base_curr);
		sprintf (env_rec.env_desc,"%-70.70s",  pocrRec.description);
		put_env (env_rec.env_name,env_rec.env_value,env_rec.env_desc);

		cc = abc_update (comr, &comr_rec);
		if (cc)
			file_err (cc, comr, "DBUPDATE");
	}
	/*-----------------------------
	| Update New company record . |
	-----------------------------*/
	else
	{
		if (!comr_rec.consolidate)
		{
			strcpy (CoCurrCode, "CURR_CODE");
			sprintf (env_rec.env_name,"%-15.15s", CoCurrCode);
			sprintf (env_rec.env_value,"%-30.30s", comr_rec.base_curr);
			sprintf (env_rec.env_desc,"%-70.70s",  pocrRec.description);
			put_env (env_rec.env_name,env_rec.env_value,env_rec.env_desc);
		}
		sprintf (CoCurrCode, "CURR_CODE%2.2s", comr_rec.co_no);
		sprintf (env_rec.env_name,"%-15.15s", CoCurrCode);
		sprintf (env_rec.env_value,"%-30.30s", comr_rec.base_curr);
		sprintf (env_rec.env_desc,"%-70.70s",  pocrRec.description);
		put_env (env_rec.env_name,env_rec.env_value,env_rec.env_desc);

		strcpy (comr_rec.co_no, local_rec.cur_co);
		strcpy (comr_rec.module_inst, "NNNNNNNNNNNNNNNNNNNN");
		strcpy (comr_rec.stat_flag, "0");
		cc = abc_add (comr,&comr_rec);
		if (cc)
			file_err (cc, comr, "DBADD");
	}

	strcpy (local_rec.prev_co, local_rec.cur_co);

	return (EXIT_SUCCESS);
}

void
SrchComr (
 char *	key_val)
{
	work_open ();
	save_rec ("#Co No", "#Company Name");

	sprintf (comr_rec.co_no, "%-2.2s", key_val);
	cc = find_rec (comr, &comr_rec, GTEQ, "r");
	while (!cc && !strncmp (comr_rec.co_no, key_val, strlen (key_val)))
	{
		cc = save_rec (comr_rec.co_no, comr_rec.co_name);
		if (cc)
			break;

		cc = find_rec (comr, &comr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	sprintf (comr_rec.co_no, "%-2.2s", temp_str);
	cc = find_rec (comr, &comr_rec, COMPARISON, "r");
}

void
SrchEsmr (
 char *	key_val)
{
	work_open ();
	save_rec ("#Br","#Branch Name");
	strcpy (esmr_rec.co_no, comr_rec.co_no);
	sprintf (esmr_rec.est_no, "%2.2s", key_val);
	cc = find_rec (esmr, &esmr_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (esmr_rec.co_no, comr_rec.co_no) && 
	       !strncmp (esmr_rec.est_no, key_val, strlen (key_val)))
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
	sprintf (esmr_rec.co_no, comr_rec.co_no);
	sprintf (esmr_rec.est_no, "%2.2s", temp_str);
	cc = find_rec (esmr, &esmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "esmr", "DBFIND");
}
/*==================================
| Search for Special instructions. |
==================================*/
void
SrchExsi (
 char *	key_val)
{
	char	wk_code [4];

	work_open ();
	save_rec ("#Spec Inst","#Instruction description.");

	strcpy (exsi_rec.co_no,comr_rec.co_no);
	exsi_rec.inst_code = atoi (key_val);

	cc = find_rec (exsi,&exsi_rec,GTEQ,"r");
	while (!cc && !strcmp (exsi_rec.co_no,comr_rec.co_no))
	{
		sprintf (wk_code, "%03d", exsi_rec.inst_code);
		sprintf (err_str, "%-60.60s", exsi_rec.inst_text);
		cc = save_rec (wk_code, err_str);
		if (cc)
			break;

		cc = find_rec (exsi,&exsi_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (exsi_rec.co_no,comr_rec.co_no);
	exsi_rec.inst_code = atoi (temp_str);
	cc = find_rec (exsi,&exsi_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, "exsi", "DBFIND");
}

/*===============================
| Search for currency pocr code |
===============================*/
void
SrchPocr (
 char *	key_val)
{
	work_open ();
	save_rec ("#Code","#Description        ");
	strcpy (pocrRec.co_no, comr_rec.co_no);
	sprintf (pocrRec.code ,"%-3.3s",key_val);
	cc = find_rec (pocr, &pocrRec, GTEQ, "r");
    while (!cc && !strcmp (pocrRec.co_no,comr_rec.co_no) && 
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

	strcpy (pocrRec.co_no,comr_rec.co_no);
	sprintf (pocrRec.code,"%-3.3s",temp_str);
	cc = find_rec (pocr, &pocrRec, COMPARISON, "r");
	if (cc)
		file_err (cc, pocr, "DBFIND");
}

int
heading (
 int	scn)
{
	if (restart) 
		return (EXIT_SUCCESS);
	
	if (scn != cur_screen)
		scn_set (scn);

	clear ();
	print_at (0, 25, ML (mlMenuMess019));
	print_at (0,60, ML (mlMenuMess020), local_rec.prev_co);


	pr_box_lines (scn);

	scn_write (scn);
	line_cnt = 0;
	return (EXIT_SUCCESS);
}

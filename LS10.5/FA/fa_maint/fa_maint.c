/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: fa_maint.c,v 5.5 2001/10/11 05:18:15 robert Exp $
|  Program Name  : (fa_maint.c)  
|  Program Desc  : (Fixed Assets Maintenance)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 30/12/87         |
|---------------------------------------------------------------------|
| $Log: fa_maint.c,v $
| Revision 5.5  2001/10/11 05:18:15  robert
| Updated to adjust cursor position of the fields
|
| Revision 5.4  2001/08/28 08:45:59  scott
| Update for small change related to " (" that should not have been changed from "("
|
| Revision 5.3  2001/08/09 09:13:11  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/09 01:39:26  scott
| RELEASE 5.0
|
| Revision 5.1  2001/08/06 23:25:54  scott
| RELEASE 5.0
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: fa_maint.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/FA/fa_maint/fa_maint.c,v 5.5 2001/10/11 05:18:15 robert Exp $";

#define	MAXWIDTH	150
#include	<pslscr.h>
#include	<GlUtils.h>
#include	<twodec.h>
#include	<ml_std_mess.h>
#include	<ml_fa_mess.h>
#include	<std_decs.h>

#define		INPUT		0

#define		FA_DISPLAY	 (prog_type == DISPLAY)
#define		FA_INPUT	 (prog_type == INPUT)

#include	"schema"

struct commRecord	comm_rec;
struct faglRecord	fagl_rec;
struct faglRecord	fagl2_rec;
struct famrRecord	famr_rec;
struct fatrRecord	fatr_rec;

	char	*data	= "data",
			*fagl2	= "fagl2";

	char	systemDate [11];

	int	NewAsset = 0;
	int	edit_mode = 0;
	int	prog_type;
	double	DV_TaxAmount	=	0.00,
			DV_IntAmount	=	0.00,
			DV_TaxMin		=	0.00,
			DV_IntMin		=	0.00;

struct
{
	char	dummy [11];
	char	AssetGroup [6],
			AssetNumber [6];
	char	prev_asset [6];
	char	curr_asset [6];
	char	tax_dtype [3];
	char	int_dtype [3];
	char	gl_desc [3][26];
	char	AssYears [4],
			AssMonth [2];
	double	TaxClosing,
			IntClosing;
	char	DispStr [41];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "AssetGroup",	 2, 2, CHARTYPE,
		"UUUUU", "          ",
		" ", "", "Asset Group code  : ", "Enter Asset group code. [SEARCH] available ",
		 NE, NO,  JUSTLEFT, "", "", local_rec.AssetGroup},
	{1, LIN, "AssetGroupDesc",2, 29, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "", " ",
		 NA, NO,  JUSTLEFT, "", "", fatr_rec.group_desc},
	{1, LIN, "AssetNumber",	 3, 2, CHARTYPE,
		"UUUUU", "          ",
		"0", " ", "Asset number      : ", "Enter Asset number. <Default = new asset> [SEARCH] available. ",
		 NE, NO,  JUSTRIGHT, "0123456789", "", local_rec.AssetNumber},
	{1, LIN, "DispStr",	 5, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.DispStr},
	{1, LIN, "AssetDesc1",	 2, 69, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Asset Desc.     #1 : ", "Enter asset description ",
		 YES, NO,  JUSTLEFT, "", "", famr_rec.ass_desc1},
	{1, LIN, "AssetDesc2",	 3, 69, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Asset Desc.     #2 : ", "Enter asset description ",
		 NO, NO,  JUSTLEFT, "", "", famr_rec.ass_desc2},
	{1, LIN, "AssetDesc3",	 4, 69, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Asset Desc.     #3 : ", "Enter asset description ",
		 NO, NO,  JUSTLEFT, "", "", famr_rec.ass_desc3},
	{1, LIN, "AssetDesc4",	 5, 69, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Asset Desc.     #4 : ", "Enter asset description ",
		 NO, NO,  JUSTLEFT, "", "", famr_rec.ass_desc4},
	{1, LIN, "PurchaseDate",	 7, 2, EDATETYPE,
		"DD/DD/DD", "          ",
		"0", systemDate, "Purchase date      : ", "Enter purchase date of asset. <default = today> ",
		YES, NO,  JUSTLEFT, "", "", (char *)&famr_rec.pur_date},
	{1, LIN, "CostPrice",	 7, 69, MONEYTYPE,
		"NNN,NNN,NNN.NN", "          ",
		" ", " ", "Cost price         : ", "Enter cost price of Asset. ",
		 NO, NO,  JUSTRIGHT, "", "", (char *)&famr_rec.cost_price},
	{1, LIN, "AssetLife",	 8, 2, CHARTYPE,
		"NNN", "          ",
		" ", "", "Asset Life Years   : ", "Enter Asset life Years ",
		 NO, NO,  JUSTRIGHT, "", "", local_rec.AssYears},
	{1, LIN, "AssetMonth",	 8, 27, CHARTYPE,
		"NN", "          ",
		" ", "0", ": ", "Enter Asset life Months ",
		 NO, NO,  JUSTLEFT, "0", "12", local_rec.AssMonth},
	{1, LIN, "PrivateUse",	8, 69, FLOATTYPE,
		"NN.N", "          ",
		" ", "", "Private use %      : ", "Enter the percentage asset will be user for private use. ",
		 NO, NO,  JUSTLEFT, "0", "99.9", (char *)&famr_rec.priv_use_tax},
	{1, LIN, "1styearrule",	9, 2, CHARTYPE,
		"U", "          ",
		" ", "", "First year rule    : ", "(F) - Full Year / (H) - Half Year) / (M) - Per Month / (A) - One Specified Amount.",
		 ND, NO,  JUSTLEFT, "FHMA", "", famr_rec.f_y_rule},
	{1, LIN, "1styearamt",	9, 69, MONEYTYPE,
		"NNN,NNN,NNN.NN", "          ",
		" ", "", "First year amount  : ", "Enter first year amount. ",
		 ND, NO,  JUSTRIGHT, "", "", (char *)&famr_rec.f_y_amt},
	{1, LIN, "DepRule",	9, 2, CHARTYPE,
		"U", "          ",
		" ", "M", "Depreciation Rule  : ", "(F) - Full Year /(H) - Half Year) / (M) - Per Month.",
		 NO, NO,  JUSTLEFT, "FHM", "", famr_rec.dep_rule},
	{1, LIN, "MaxDeprec",	9, 69, MONEYTYPE,
		"NNN,NNN,NNN.NN", "          ",
		" ", "", "Max. depreciation  : ", "Enter the maximum Depreciation amount ",
		 NO, NO,  JUSTRIGHT, "", "", (char *)&famr_rec.max_deprec},
	{1, LIN, "TaxOpenValue",	11, 2, MONEYTYPE,
		"NNN,NNN,NNN.NN", "          ",
		" ", "", "Opening value      : ", "Enter Asset opening book value for tax. ",
		 NO, NO,  JUSTRIGHT, "", "", (char *)&famr_rec.tax_open_val},
	{1, LIN, "TaxType",	12, 2, CHARTYPE,
		"UU", "          ",
		" ", "", "Depreciation type  : ", "Enter Depreciation type. CP(Cost Price - straight line) DV(Diminishing value) ",
		 NO, NO,  JUSTLEFT, "CPDV", "", local_rec.tax_dtype},
	{1, LIN, "TaxFlag",	13, 2, CHARTYPE,
		"U", "          ",
		" ", fatr_rec.tax_pa_flag, "Deprec. By (P/A)   : ", "Enter Depreciation calculation P(ercent) or A(mount) ",
		 NO, NO,  JUSTLEFT, "PA", "", famr_rec.tax_pa_flag},
	{1, LIN, "TaxPercent",	14, 2, FLOATTYPE,
		"NNN.NNNN", "          ",
		" ", " ", "Depreciation %     : ", "Enter Depreciation percent ",
		 ND, NO,  JUSTRIGHT, "0", "100", (char *)&famr_rec.tax_d_pc},
	{1, LIN, "TaxAmount",		14, 2, MONEYTYPE,
		"NNN,NNN,NNN.NN", "          ",
		" ", " ", "Depreciation amt.  : ","Enter Depreciation Amount ",
		 ND, NO,  JUSTRIGHT, "", "", (char *)&famr_rec.tax_d_amt},
	{1, LIN, "TaxClosingValue",	15, 2, MONEYTYPE,
		"NNN,NNN,NNN.NN", "          ",
		" ", "", "Book value         : ", "",
		 NA, NO,  JUSTRIGHT, "", "", (char *)&local_rec.TaxClosing},
	{1, LIN, "IntOpenValue",	11, 69, MONEYTYPE,
		"NNN,NNN,NNN.NN", "          ",
		" ", "", "Opening value      : ", "Enter Asset opening book value for internal. ",
		 NO, NO,  JUSTRIGHT, "", "", (char *)&famr_rec.int_open_val},
	{1, LIN, "IntType",	12, 69, CHARTYPE,
		"UU", "          ",
		" ", "", "Depreciation type  : ", "Enter Depreciation type. CP(Cost Price - straight line) DV(Diminishing value) ",
		 NO, NO,  JUSTLEFT, "CPDV", "", local_rec.int_dtype},
	{1, LIN, "IntFlag",	13, 69, CHARTYPE,
		"U", "          ",
		" ", fatr_rec.int_pa_flag, "Deprec. By (P/A)   : ", "Enter Depreciation calculation P(ercent) or A(mount) ",
		 NO, NO,  JUSTLEFT, "PA", "", famr_rec.int_pa_flag},
	{1, LIN, "IntPercent",	14, 69, FLOATTYPE,
		"NNN.NNNN", "          ",
		" ", " ", "Depreciation %     : ", "Enter Depreciation percent ",
		 ND, NO,  JUSTRIGHT, "0", "100", (char *)&famr_rec.int_d_pc},
	{1, LIN, "IntAmount",		14, 69, MONEYTYPE,
		"NNN,NNN,NNN.NN", "          ",
		" ", " ", "Depreciation amt.  : ","Enter Depreciation Amount ",
		 ND, NO,  JUSTRIGHT, "", "", (char *)&famr_rec.int_d_amt},
	{1, LIN, "IntClosingValue",	15, 69, MONEYTYPE,
		"NNN,NNN,NNN.NN", "          ",
		" ", "", "Book value         : ", "",
		 NA, NO,  JUSTRIGHT, "", "", (char *)&local_rec.IntClosing},
	{1, LIN, "crdacc",	 17, 2, CHARTYPE,
		"NNNNNNNNNNNNNNNN", "          ",
		"0", "", "Depr. Provn. A/c   : ", "Enter Depreciation Provision General Ledger Account Number ",
		YES, NO,  JUSTLEFT, "0123456789", "", famr_rec.gl_crd_acc},
	{1, LIN, "GlDesc1",	 17, 69, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Acc. Description   : ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.gl_desc [0]},
	{1, LIN, "dbtacc",	 18, 2, CHARTYPE,
		"NNNNNNNNNNNNNNNN", "          ",
		"0", "", "Depr. Exp. A/c     : ", "Enter Depreciation Expense General Ledger Account Number ",
		YES, NO,  JUSTLEFT, "0123456789", "", famr_rec.gl_dbt_acc},
	{1, LIN, "GlDesc2",	 18, 69, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Acc. Description   : ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.gl_desc [1]},
	{1, LIN, "assacc",	19, 2, CHARTYPE,
		"NNNNNNNNNNNNNNNN", "          ",
		"0", "", "Disposal Clear A/c : ", "Enter Disposal Clearing General Legder Account. ",
		YES, NO,  JUSTLEFT, "0123456789", "", famr_rec.gl_ass_acc},
	{1, LIN, "GlDesc3",	 19, 69, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Acc. Description   : ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.gl_desc [2]},
	{0, LIN, "",		 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, " ", " ", local_rec.dummy},
};

extern	int	TruePosition;
extern	int	EnvScreenOK;

char 	*Exptype 			(char *);
double 	CalcIntAmount 		(void);
double 	CalcIntPercent 		(void);
double 	CalcTaxAmount 		(void);
double 	CalcTaxPercent 		(void);
int 	CalcIntMvMonths 	(void);
int 	CalcTaxMvMonths 	(void);
int 	CheckFagl 			(void);
int 	FindAcc 			(char *);
int 	heading 			(int);
int	 	spec_valid 			(int);
void 	AddFagl 			(long);
void 	CalcClosing 		(void);
void 	CloseDB 			(void);
void 	DeleteFagl 			(void);
void 	OpenDB 				(void);
void 	ProcessFagl 		(void);
void 	shutdown_prog 		(void);
void 	SrchFamr 			(char *);
void 	SrchFatr 			(char *);
void 	Update 				(void);

/*===========================
| Main Processing Routine . |
===========================*/
int
main
 (
	int argc,
	char *argv []
)
{
	int		field;
	char	*GL_SetAccWidth (char *, int);

	TruePosition	=	TRUE;
	EnvScreenOK		=	FALSE;

	if (!strcmp (argv [0],"fa_display"))
		prog_type = DISPLAY;
	
	if (!strcmp (argv [0],"fa_maint"))
		prog_type = INPUT;
		
	SETUP_SCR (vars);
	init_scr ();
	set_tty ();
	_set_masks ("fa_maint.s");
	init_vars (1);

	if (FA_DISPLAY)
	{
		for (field = label ("AssetDesc1"); FIELD.scn != 0; field++)
			if (FIELD.required != ND)
				FIELD.required	=	NA;
	}
	OpenDB ();

	strcpy (local_rec.prev_asset, "00000");

	strcpy (systemDate, DateToString (TodaysDate ()));

	GL_SetAccWidth (comm_rec.co_no, FALSE);
	swide ();

	while (!prog_exit)
	{
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		search_ok = 1;
		init_ok = 1;
		init_vars (1);

		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		heading (1);
		scn_display (1);
		edit (1);

		if (restart)
			continue;
		
		if (FA_INPUT)
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

void
OpenDB (void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	abc_alias (fagl2, fagl);
	open_rec (fagl2,fagl_list, FAGL_NO_FIELDS, "fagl_famr_hash");
	open_rec (fagl, fagl_list, FAGL_NO_FIELDS, "fagl_id_no");
	open_rec (famr, famr_list, FAMR_NO_FIELDS, "famr_id_no");
	open_rec (fatr, fatr_list, FATR_NO_FIELDS, "fatr_id_no");
	OpenGlmr ();
}

void
CloseDB 
 (
	void
)
{
	abc_fclose (fagl2);
	abc_fclose (fagl);
	abc_fclose (famr);
	abc_fclose (fatr);
	GL_Close ();
	abc_dbclose (data);
}

int
spec_valid
 (
	int field
)
{
	/*----------------------------
	| Validate Asset group code. |
	----------------------------*/
	if (LCHECK ("AssetGroup"))
	{
		if (SRCH_KEY)
		{
			SrchFatr (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (fatr_rec.co_no,comm_rec.co_no);
		strcpy (fatr_rec.group,local_rec.AssetGroup);
		cc = find_rec (fatr, &fatr_rec, COMPARISON, "r");
		if (cc)
		{
			print_err (ML (mlFaMess003));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		DSP_FLD ("AssetGroupDesc");
		return (EXIT_SUCCESS);
	}
	/*------------------------------
	| Validate Fixed Asset number. |
	------------------------------*/
	if (LCHECK ("AssetNumber"))
	{
		if (SRCH_KEY)
		{
			SrchFamr (temp_str);
			return (EXIT_SUCCESS);
		}
		if (dflt_used)
		{
			strcpy (famr_rec.ass_no, "NEW  ");
			strcpy (local_rec.AssetNumber, "NEW  ");
			DSP_FLD ("AssetNumber");
			NewAsset = 1;
			return (EXIT_SUCCESS);
		}
		strcpy (famr_rec.co_no,comm_rec.co_no);
		strcpy (famr_rec.ass_group,	local_rec.AssetGroup);
		strcpy (famr_rec.ass_no,	local_rec.AssetNumber);
		cc = find_rec (famr, &famr_rec, COMPARISON, "r");
		if (cc)
		{
			print_err (ML (mlFaMess004));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		else
		{
			NewAsset = 0;
			entry_exit = 1;
		}
		
		/*------------------------------------------
		| 1st year rule not currently implemented. |
		------------------------------------------*/
		if (famr_rec.f_y_rule [0] == 'A')
		{
			if (FLD ("1styearamt") != ND)
				FLD ("1styearamt") = YES;
		}
		else
		{
			if (FLD ("1styearamt") != ND)
				FLD ("1styearamt") = NA;
			famr_rec.f_y_amt = 0.0;
		}
		strcpy (local_rec.tax_dtype, Exptype (famr_rec.tax_dtype));
		strcpy (local_rec.int_dtype, Exptype (famr_rec.int_dtype));
		
		FLD ("TaxFlag") 	= (FA_INPUT) ? NO : NA;
		FLD ("IntFlag") 	= (FA_INPUT) ? NO : NA;
		FLD ("TaxPercent") = (famr_rec.tax_pa_flag [0] == 'P') ?  NA : ND;
		FLD ("TaxAmount")  = (famr_rec.tax_pa_flag [0] == 'P') ?  ND : NA;
		FLD ("IntPercent") = (famr_rec.int_pa_flag [0] == 'P') ?  NA : ND;
		FLD ("IntAmount")  = (famr_rec.int_pa_flag [0] == 'P') ?  ND : NA;

		if (local_rec.tax_dtype [0] == 'D')
		{
			FLD ("TaxPercent") 	= (FA_INPUT) ? YES : NA;
			FLD ("TaxFlag") 	= NA;
		}

		if (local_rec.int_dtype [0] == 'D')
		{
			FLD ("IntPercent") 	= (FA_INPUT) ? YES : NA;
			FLD ("IntFlag") 	= NA;
		}
		sprintf (local_rec.AssYears, "%-3.3s", famr_rec.ass_life);
		sprintf (local_rec.AssMonth, "%-2.2s", famr_rec.ass_life + 5);
		
		if (CheckFagl () && FA_INPUT)
		{
			for (field = label ("PurchaseDate"); FIELD.scn != 0; field++)
			{
				if (FIELD.required != ND)
					FIELD.required	=	NA;
			}
		}
		/*----------------------
		| Asset has been sold. |
		----------------------*/
		if (famr_rec.disp_date > 0L || famr_rec.stat_flag [0] == 'D')
		{
			if (FA_INPUT)
			{
				sprintf (err_str,ML (mlFaMess016),
											local_rec.AssetNumber,
											DateToString (famr_rec.disp_date));
				print_err (err_str);
				return (EXIT_FAILURE);
			}
			sprintf (local_rec.DispStr, "Asset Sold on %s for %.2f", 
											DateToString (famr_rec.disp_date), 
											DOLLARS (famr_rec.disp_price));

			DSP_FLD ("DispStr");
		}
		CalcClosing ();

		spec_valid (label ("dbtacc"));
		spec_valid (label ("crdacc"));
		spec_valid (label ("assacc"));
		return (EXIT_SUCCESS);
	}
	/*------------------------------
	| Validate asset descriptions. |
	------------------------------*/
	if (LCHECK ("AssetDesc2") ||
		LCHECK ("AssetDesc3") ||
		LCHECK ("AssetDesc4"))
	{
		if (dflt_used)
			skip_entry 	=	goto_field (field, label ("PurchaseDate"));

		return (EXIT_SUCCESS);
	}
	/*-------------------------
	| Validate Purchase Date. |
	-------------------------*/
	if (LCHECK ("PurchaseDate"))
	{
		if (famr_rec.pur_date > StringToDate (systemDate))
		{
			print_mess (ML (mlStdMess111));
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("AssetLife"))
	{
		if (dflt_used)
		{
			sprintf (local_rec.AssYears, "%-3.3s", fatr_rec.ass_life);
			sprintf (local_rec.AssMonth, "%-2.2s", fatr_rec.ass_life + 5);
			DSP_FLD ("AssetLife");
			DSP_FLD ("AssetMonth");
		}
		if (prog_status != ENTRY)
			spec_valid (label ("DepRule"));
		return (EXIT_SUCCESS);
	}
	/*-----------------------------------------------------
	| Validate Asset life, Asset Month, Max Depreciation. |
	-----------------------------------------------------*/
	if (LCHECK ("AssetMonth") ||
		LCHECK ("MaxDeprec"))
	{
		if (prog_status != ENTRY)
			spec_valid (label ("DepRule"));
	
		return (EXIT_SUCCESS);
	}

	/*-----------------------------------------
	| Validate Credit General Ledger Account. |
	-----------------------------------------*/
	if (LCHECK ("crdacc"))
	{
		if (SRCH_KEY)
			SearchGlmr (comm_rec.co_no, temp_str, "F*P");
		else
		{
			cc = FindAcc (famr_rec.gl_crd_acc);
			if (cc)
				return (cc);
		}
		strcpy (local_rec.gl_desc [0], glmrRec.desc);
		DSP_FLD ("GlDesc1");
		return (EXIT_SUCCESS);
	}

	/*----------------------------------------
	| Validate Debit General Ledger Account. |
	----------------------------------------*/
	if (LCHECK ("dbtacc"))
	{
		if (SRCH_KEY)
			SearchGlmr (comm_rec.co_no, temp_str, "F*P");
		else
		{
			cc = FindAcc (famr_rec.gl_dbt_acc);
			if (cc)
				return (cc);
		}
		strcpy (local_rec.gl_desc [1], glmrRec.desc);
		DSP_FLD ("GlDesc2");
		return (EXIT_SUCCESS);
	}

	/*----------------------------------------
	| Validate Asset General Ledger Account. |
	----------------------------------------*/
	if (LCHECK ("assacc"))
	{
		if (SRCH_KEY)
			SearchGlmr (comm_rec.co_no, temp_str, "F*P");
		else
		{
			cc = FindAcc (famr_rec.gl_ass_acc);
			if (cc)
				return (cc);
		}
		strcpy (local_rec.gl_desc [2], glmrRec.desc);
		DSP_FLD ("GlDesc3");
		return (EXIT_SUCCESS);
	}

	/*---------------------------
	| Validate first year rule. |
	---------------------------*/
	if (LCHECK ("1styearrule"))
	{
		if (famr_rec.f_y_rule [0] == 'A')
		{
			if (FLD ("1styearamt") != ND)
				FLD ("1styearamt") = YES;
		}
		else
		{
			famr_rec.f_y_amt 	= 0.0;
			if (FLD ("1styearamt") != ND)
				FLD ("1styearamt") 	= NA;
			DSP_FLD ("1styearamt");
		}
		return (EXIT_SUCCESS);
	}
	/*-----------------------------
	| Validate Depreciation rule. |
	-----------------------------*/
	if (LCHECK ("DepRule"))
	{
		/*-------------------------------
		| default is value in tax table	|
		-------------------------------*/
		if (dflt_used)
			strcpy (famr_rec.dep_rule, fatr_rec.dep_rule);

		if (famr_rec.dep_rule [0] == ' ')
			strcpy (famr_rec.dep_rule, "M");
		
		if (famr_rec.dep_rule [0] == 'M')
		{
			strcpy (famr_rec.f_y_rule, "M");
			famr_rec.f_y_amt 	= 0.0;
			if (FLD ("1styearamt") != ND)
				FLD ("1styearamt") 	= NA;
			FLD ("AssetMonth")	= YES;
			DSP_FLD ("1styearamt");
			DSP_FLD ("1styearrule");
		}
		else
		{
			strcpy (local_rec.AssMonth, "00");
			DSP_FLD ("AssetMonth");
			FLD ("AssetMonth")	=	NA;
		}
		DSP_FLD ("DepRule");

		if (prog_status	!= ENTRY)
		{
			spec_valid (label ("TaxFlag"));
			spec_valid (label ("IntFlag"));
		}
		return (EXIT_SUCCESS);
	}
	/*---------------------------
	| Validate Max Depr Amount. |
	---------------------------*/
	if (LCHECK ("MaxDeprec"))
	{
		if (dflt_used)
		{
			famr_rec.max_deprec = fatr_rec.max_depr;
			return (EXIT_SUCCESS);
		}
		return (EXIT_SUCCESS);
	}

	/*-----------------------------
	| Validate Opening tax value. |
	-----------------------------*/
	if (LCHECK ("TaxOpenValue"))
	{
		if (dflt_used)
			famr_rec.tax_open_val	=	famr_rec.cost_price;

		if (prog_status != ENTRY)
			spec_valid (label ("DepRule"));

		CalcClosing ();
		return (EXIT_SUCCESS);
	}
	/*----------------------------------
	| Validate Opening Internal value. |
	----------------------------------*/
	if (LCHECK ("IntOpenValue"))
	{
		if (dflt_used)
			famr_rec.int_open_val	=	famr_rec.cost_price;

		if (prog_status != ENTRY)
			spec_valid (label ("DepRule"));

		CalcClosing ();
		return (EXIT_SUCCESS);
	}
	
	/*--------------------
	| Validate Tax Type. |
	--------------------*/
	if (LCHECK ("TaxType"))
	{
		if (dflt_used && prog_status == ENTRY)
		{
			strcpy (local_rec.tax_dtype, "CP");
			DSP_FLD ("TaxType");
			return (EXIT_SUCCESS);
		}
		if (strcmp (local_rec.tax_dtype, "CP") && 
			strcmp (local_rec.tax_dtype, "DV"))
		{
			print_mess (ML (mlFaMess015));
			return (EXIT_FAILURE);
		}
		if (local_rec.tax_dtype [0] == 'D')
		{
			strcpy (famr_rec.tax_pa_flag, "P");
			FLD ("TaxFlag") = NA;
			spec_valid (label ("TaxFlag"));
		}
		else
			FLD ("TaxFlag") = (FA_INPUT) ? YES : NA;
			
		return (EXIT_SUCCESS);
	}
	/*-------------------------
	| Validate Internal Type. |
	-------------------------*/
	if (LCHECK ("IntType"))
	{
		if (dflt_used && prog_status == ENTRY)
		{
			strcpy (local_rec.int_dtype, "CP");
			DSP_FLD ("IntType");
			return (EXIT_SUCCESS);
		}
		if (strcmp (local_rec.int_dtype, "CP") && strcmp (local_rec.int_dtype, "DV"))
		{
			print_mess (ML (mlFaMess015));
			return (EXIT_FAILURE);
		}
		if (local_rec.int_dtype [0] == 'D')
		{
			strcpy (famr_rec.int_pa_flag, "P");
			FLD ("IntFlag") = NA;
			spec_valid (label ("IntFlag"));
		}
		else
			FLD ("IntFlag") = (FA_INPUT) ? YES : NA;
		return (EXIT_SUCCESS);
	}
	/*--------------------
	| Validate Tax Flag. |
	--------------------*/
	if (LCHECK ("TaxFlag"))
	{
		famr_rec.tax_d_amt 	= CalcTaxAmount ();
		famr_rec.tax_d_pc 	= (float) CalcTaxPercent ();

		print_at (vars [field + 1].row, vars [field + 1].col,"                                    ");

		if (famr_rec.tax_pa_flag [0] == 'P')
		{
			FLD ("TaxPercent") 	= (local_rec.tax_dtype [0] == 'D') ? YES : NA;
			FLD ("TaxAmount") 	=	ND;
			display_prmpt (label ("TaxPercent"));
		}
		else
		{
			strcpy (famr_rec.tax_pa_flag, "A");
			FLD ("TaxPercent") 	=	ND;
			FLD ("TaxAmount") 	=	NA;
			display_prmpt (label ("TaxAmount"));
		}
		DSP_FLD ("TaxFlag");
		DSP_FLD ("TaxAmount");
		DSP_FLD ("TaxPercent");
		return (EXIT_SUCCESS);
	}
	/*--------------------
	| Validate Int Flag. |
	--------------------*/
	if (LCHECK ("IntFlag"))
	{

		famr_rec.int_d_amt = CalcIntAmount ();
		famr_rec.int_d_pc 	= (float) CalcIntPercent ();

		print_at (vars [field + 1].row, vars [field + 1].col,"                                    ");
		if (famr_rec.int_pa_flag [0] == 'P')
		{
			FLD ("IntPercent") 	= (local_rec.int_dtype [0] == 'D') ? YES : NA;
			FLD ("IntAmount") 	=	ND;
			display_prmpt (label ("IntPercent"));
		}
		else
		{
			strcpy (famr_rec.int_pa_flag, "A");
			FLD ("IntPercent") 	=	ND;
			FLD ("IntAmount") 	=	NA;
			display_prmpt (label ("IntAmount"));
		}
		DSP_FLD ("IntAmount");
		DSP_FLD ("IntPercent");
		DSP_FLD ("IntFlag");
		return (EXIT_SUCCESS);
	}

	/*--------------------------
	| Validate Tax Percentage. |
	--------------------------*/
	if (LCHECK ("TaxPercent"))
	{
		if (FLD ("TaxPercent") == ND)
		{
			famr_rec.tax_d_pc = 0.00;
			return (EXIT_SUCCESS);
		}
		if (dflt_used)
		{
			famr_rec.tax_d_pc = (float) CalcTaxPercent ();
			famr_rec.tax_d_amt = CalcTaxAmount ();
			DSP_FLD ("TaxPercent");
		}
		else
		{
			if (local_rec.tax_dtype [0] == 'D')
				famr_rec.tax_d_pc = (float) CalcTaxPercent ();
		}
			
		return (EXIT_SUCCESS);
	}
	/*-------------------------------
	| Validate Internal Percentage. |
	-------------------------------*/
	if (LCHECK ("IntPercent"))
	{
		if (FLD ("IntPercent") == ND)
		{
			famr_rec.int_d_pc = 0.00;
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			famr_rec.int_d_pc = (float) CalcIntPercent ();
			famr_rec.int_d_amt = CalcIntAmount ();
			DSP_FLD ("IntPercent");
		}
		else
		{
			if (local_rec.int_dtype [0] == 'D')
				famr_rec.int_d_pc = (float) CalcIntPercent ();
		}
		
		return (EXIT_SUCCESS);
	}
	/*----------------------
	| Validate Tax Amount. |
	----------------------*/
	if (LCHECK ("TaxAmount"))
	{
		if (dflt_used)
		{
			famr_rec.tax_d_amt = CalcTaxAmount ();
			famr_rec.tax_d_pc  = (float) CalcTaxPercent ();
		}
		
		DSP_FLD ("TaxAmount");
		return (EXIT_SUCCESS);
	}
	/*----------------------
	| Validate Int Amount. |
	----------------------*/
	if (LCHECK ("IntAmount"))
	{
		if (dflt_used)
		{
			famr_rec.int_d_amt = CalcIntAmount ();
			famr_rec.int_d_pc  = (float) CalcIntPercent ();
		}
		
		DSP_FLD ("IntAmount");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*========================
| Calculate tax percent. |
========================*/
double
CalcTaxPercent
 (
	void
)
{
	double	OpeningValue;
	double	TaxPercent;
	int		TotalMonths;


	TotalMonths = atoi (local_rec.AssYears) * 12;
	TotalMonths += atoi (local_rec.AssMonth);

	OpeningValue	= (famr_rec.max_deprec > 0.00) ? famr_rec.max_deprec
												   : famr_rec.tax_open_val;
	if (OpeningValue > famr_rec.tax_open_val)
		OpeningValue = famr_rec.tax_open_val;

	if (famr_rec.dep_rule [0] == 'H')
		TotalMonths /= 6;

	if (famr_rec.dep_rule [0] == 'F')
		TotalMonths /= 12;

	if (local_rec.tax_dtype [0] == 'D')
	{
		if (TotalMonths < CalcTaxMvMonths ())
		{
			sprintf (err_str, ML (mlFaMess017), CalcTaxMvMonths (), famr_rec.tax_d_pc, TotalMonths);

			print_mess (err_str);
			sleep (sleepTime);
		}
		return (famr_rec.tax_d_pc);
	}
	if (TotalMonths == 0.00 || OpeningValue == 0.00)
		return (0.00);

	TaxPercent = OpeningValue / TotalMonths;
	TaxPercent /= OpeningValue;
	TaxPercent *= 100;
	return (TaxPercent);
}

/*=============================
| Calculate internal percent. |
=============================*/
double
CalcIntPercent
 (
	void
)
{
	double	OpeningValue;
	double	IntPercent;
	int		TotalMonths;

	TotalMonths = atoi (local_rec.AssYears) * 12;
	TotalMonths += atoi (local_rec.AssMonth);

	OpeningValue	= (famr_rec.max_deprec > 0.00) ? famr_rec.max_deprec
												   : famr_rec.int_open_val;
	if (OpeningValue > famr_rec.int_open_val)
		OpeningValue = famr_rec.int_open_val;

	if (famr_rec.dep_rule [0] == 'H')
		TotalMonths /= 6;

	if (famr_rec.dep_rule [0] == 'F')
		TotalMonths /= 12;

	if (local_rec.int_dtype [0] == 'D')
	{
		if (TotalMonths < CalcIntMvMonths ())
		{
			sprintf (err_str, ML (mlFaMess017), CalcIntMvMonths (), famr_rec.int_d_pc, TotalMonths);

			print_mess (err_str);
			sleep (sleepTime);
		}
		return (famr_rec.int_d_pc);
	}
	if (TotalMonths == 0.00 || OpeningValue == 0.00)
		return (0.00);

	IntPercent = OpeningValue / TotalMonths;
	IntPercent /= OpeningValue;
	IntPercent *= 100;
	return (IntPercent);
}

/*=======================
| Calculate tax amount. |
=======================*/
double
CalcTaxAmount 
 (
	void
)
{
	double	OpeningValue;
	int		TotalMonths;

	TotalMonths = atoi (local_rec.AssYears) * 12;
	TotalMonths += atoi (local_rec.AssMonth);

	OpeningValue	= (famr_rec.max_deprec > 0.00) ? famr_rec.max_deprec
												   : famr_rec.tax_open_val;
	if (OpeningValue > famr_rec.tax_open_val)
		OpeningValue = famr_rec.tax_open_val;

	if (famr_rec.dep_rule [0] == 'H')
		TotalMonths /= 6;

	if (famr_rec.dep_rule [0] == 'F')
		TotalMonths /= 12;

	if (TotalMonths != 0.00)
		return (OpeningValue / TotalMonths);
	
	return (0.00);
}

/*============================
| Calculate internal amount. |
============================*/
double
CalcIntAmount
 (
	void
)
{
	double	OpeningValue;
	int		TotalMonths;

	TotalMonths = atoi (local_rec.AssYears) * 12;
	TotalMonths += atoi (local_rec.AssMonth);
	OpeningValue	= (famr_rec.max_deprec > 0.00) ? famr_rec.max_deprec
												   : famr_rec.int_open_val;
	if (OpeningValue > famr_rec.int_open_val)
		OpeningValue = famr_rec.int_open_val;

	if (famr_rec.dep_rule [0] == 'H')
		TotalMonths /= 6;

	if (famr_rec.dep_rule [0] == 'F')
		TotalMonths /= 12;

	if (TotalMonths != 0.00)
		return (OpeningValue / TotalMonths);
	
	return (0.00);
}

/*=================
| Update records. |
=================*/
void
Update
 (
	void
)
{
	clear ();

	/*-------------------------------
	| add or update database record	|
	-------------------------------*/
	sprintf (famr_rec.tax_dtype, "%-1.1s", local_rec.tax_dtype);
	sprintf (famr_rec.int_dtype, "%-1.1s", local_rec.int_dtype);
	sprintf (famr_rec.ass_life, "%-4.4s:%-2.2s", local_rec.AssYears,
												 local_rec.AssMonth);
	if (NewAsset)
	{
		strcpy (fatr_rec.co_no,comm_rec.co_no);
		strcpy (fatr_rec.group,local_rec.AssetGroup);
		cc = find_rec (fatr, &fatr_rec, COMPARISON, "u");
		if (cc)
			file_err (cc, fatr, "DBFIND");
		
		fatr_rec.nxt_asset++;

		cc = abc_update ("fatr", &fatr_rec);
		if (cc)
			file_err (cc, fatr, "DBUPDATE");
			
		strcpy (famr_rec.co_no, comm_rec.co_no);
		sprintf (famr_rec.ass_no, "%05ld", fatr_rec.nxt_asset);
		strcpy (famr_rec.ass_group, local_rec.AssetGroup);
		strcpy (famr_rec.stat_flag, "0");
		famr_rec.disp_date 	= 0L;
		famr_rec.disp_price = 0.0;
		cc = abc_add (famr, &famr_rec);
		if (cc)
			file_err (cc, famr, "DBADD");

		strcpy (famr_rec.co_no, comm_rec.co_no);
		sprintf (famr_rec.ass_no, "%05ld", fatr_rec.nxt_asset);
		strcpy (famr_rec.ass_group, local_rec.AssetGroup);
		cc = find_rec (famr, &famr_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, famr, "DBFIND");
	}
	else
	{
		cc = abc_update (famr, &famr_rec);
		if (cc)
			file_err (cc, famr, "DBUPDATE");
	}

	strcpy (local_rec.prev_asset, local_rec.curr_asset);
	abc_unlock (famr);

	if (!CheckFagl ())
		ProcessFagl ();
}

char
*Exptype
 (
	char *dtype
)
{
	if (dtype [0] == 'C')
		return ("CP");
	else
		return ("DV");
}

/*===================================
| Search for inventory master file. |
===================================*/
void
SrchFatr 
 (
	char	*key_val
)
{
	work_open ();
	save_rec ("#Asset group", "#Asset Group Description");
	strcpy (fatr_rec.co_no, comm_rec.co_no);
	sprintf (fatr_rec.group, "%-5.5s", key_val);
	cc = find_rec (fatr, &fatr_rec, GTEQ, "r");
	while (!cc && !strncmp (fatr_rec.group, key_val, strlen (key_val)) && 
				  !strcmp (fatr_rec.co_no, comm_rec.co_no))
	{
		cc = save_rec (fatr_rec.group, fatr_rec.group_desc);
		if (cc)
			break;

		cc = find_rec (fatr, &fatr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
	{
		sprintf (fatr_rec.group, "%-5.5s", " ");
		return;
	}
	strcpy (fatr_rec.co_no, comm_rec.co_no);
	sprintf (fatr_rec.group, "%-5.5s", temp_str);
	cc = find_rec (fatr, &fatr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, fatr, "DBFIND");
}

/*===================================
| Search for inventory master file. |
===================================*/
void
SrchFamr 
 (
	char	*key_val
)
{
	work_open ();
	save_rec ("#Asset No", "#Asset Description");
	strcpy (famr_rec.co_no, comm_rec.co_no);
	strcpy (famr_rec.ass_group, local_rec.AssetGroup);
	sprintf (famr_rec.ass_no, "%-5.5s", key_val);
	cc = find_rec (famr, &famr_rec, GTEQ, "r");
	while (!cc && !strcmp (famr_rec.co_no, comm_rec.co_no) &&
				  !strcmp (famr_rec.ass_group, local_rec.AssetGroup) &&
				  !strncmp (famr_rec.ass_no, key_val, strlen (key_val)))
	{
		cc = save_rec (famr_rec.ass_no, famr_rec.ass_desc1);
		if (cc)
			break;

		cc = find_rec (famr, &famr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
	{
		sprintf (famr_rec.ass_no, "%-5.5s", " ");
		return;
	}
	strcpy (famr_rec.co_no, comm_rec.co_no);
	strcpy (famr_rec.ass_group, local_rec.AssetGroup);
	sprintf (famr_rec.ass_no, "%-5.5s", temp_str);
	cc = find_rec (famr, &famr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, famr, "DBFIND");
}

/*===============================================
| Find account number, account description etc. |
===============================================*/
int
FindAcc 
 (
	char	*acc_no
)
{
	strcpy (glmrRec.acc_no, acc_no);

	if (!strncmp (glmrRec.acc_no, "0000000000000000",MAXLEVEL))
	{
		errmess (ML (mlStdMess024));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}
	strcpy (glmrRec.co_no, comm_rec.co_no);
	if ( (cc = find_rec (glmr, &glmrRec, COMPARISON, "r")))
	{
		print_err (ML (mlStdMess024));
		clear_mess ();
		return (EXIT_FAILURE);
	}

	if (glmrRec.glmr_class [2][0] != 'P')
		return print_err (ML (mlStdMess025));

	return (EXIT_SUCCESS);
}

/*===========================
| Calculate closing values. |
===========================*/
void
CalcClosing
 (
	void
)
{
	local_rec.IntClosing	= famr_rec.int_open_val;
	local_rec.TaxClosing	= famr_rec.tax_open_val;

	fagl2_rec.famr_hash 	=	famr_rec.famr_hash;
	cc = find_rec (fagl2, &fagl2_rec, GTEQ, "r");
	while (!cc && fagl2_rec.famr_hash 	==	famr_rec.famr_hash)
	{
		if (fagl2_rec.tran_date <= MonthEnd (comm_rec.crd_date))
		{
			local_rec.IntClosing	-= fagl2_rec.int_amt;
			local_rec.TaxClosing	-= fagl2_rec.tax_amt;
		}
		cc = find_rec (fagl2, &fagl2_rec, NEXT, "r");
	}
	if (local_rec.IntClosing < 0.00)
		local_rec.IntClosing = 0.00;

	if (local_rec.TaxClosing < 0.00)
		local_rec.TaxClosing = 0.00;

	DSP_FLD ("IntClosingValue");
	DSP_FLD ("TaxClosingValue");
}

/*=============================================
| Check if any fagl records have been posted. |
=============================================*/
int
CheckFagl
 (
	void
)
{
	fagl2_rec.famr_hash 	=	famr_rec.famr_hash;
	cc = find_rec (fagl2, &fagl2_rec, GTEQ, "r");
	while (!cc && fagl2_rec.famr_hash 	==	famr_rec.famr_hash)
	{
		if (fagl2_rec.posted [0] == 'Y')
			return (EXIT_FAILURE);

		cc = find_rec (fagl2, &fagl2_rec, NEXT, "r");
	}
	return (EXIT_SUCCESS);
}

/*======================================================================
| Delete fagl records as they will be re-posted based on new settings. |
======================================================================*/
void
DeleteFagl
 (
	void
)
{
	fagl2_rec.famr_hash 	=	famr_rec.famr_hash;
	cc = find_rec (fagl2, &fagl2_rec, GTEQ, "r");
	while (!cc && fagl2_rec.famr_hash 	==	famr_rec.famr_hash)
	{
		abc_delete (fagl2);

		fagl2_rec.famr_hash 	=	famr_rec.famr_hash;
		cc = find_rec (fagl2, &fagl2_rec, GTEQ, "r");
	}
}

/*======================
| Process fagl records |
======================*/
void
ProcessFagl
 (
	void
)
{
	int		dmy [3];
	long	StartDate		=	0L;
	long	CalcDate		=	0L;
	int		i				=	0;
	int		TotalMonths		=	0;
	int		TotalYears		=	0;
	int		TotalHalfYears	=	0;

	DV_TaxAmount	=	 (famr_rec.max_deprec > 0.00) ? famr_rec.max_deprec
												     : famr_rec.tax_open_val;
	DV_IntAmount	=	 (famr_rec.max_deprec > 0.00) ? famr_rec.max_deprec
												     : famr_rec.int_open_val;
	DV_TaxMin		=	DV_TaxAmount * 0.001;
	DV_IntMin		=	DV_IntAmount * 0.001;
												     
	DeleteFagl ();

	StartDate = famr_rec.pur_date;

	if (famr_rec.dep_rule [0] == 'M')
	{
		TotalMonths = atoi (local_rec.AssYears) * 12;
		TotalMonths += atoi (local_rec.AssMonth);

		CalcDate = MonthEnd (StartDate);

		for (i = 0; i < TotalMonths; i++)
		{
			AddFagl (CalcDate);
			CalcDate = MonthEnd (CalcDate + 1L);
		}
	}

	if (famr_rec.dep_rule [0] == 'F')
	{
		TotalYears = atoi (local_rec.AssYears);

		CalcDate = MonthEnd (StartDate);

		for (i = 0; i < TotalYears; i++)
		{
			AddFagl (CalcDate);

			DateToDMY (CalcDate, &dmy [0],&dmy [1],&dmy [2]);
			dmy [0]	=	1;
			dmy [2]++;

			CalcDate = DMYToDate (dmy [0],dmy [1],dmy [2]);
			CalcDate = MonthEnd (CalcDate);
		}
	}
	if (famr_rec.dep_rule [0] == 'H')
	{
		TotalHalfYears = atoi (local_rec.AssYears) * 2;

		CalcDate = MonthEnd (StartDate);

		for (i = 0; i < TotalHalfYears; i++)
		{
			AddFagl (CalcDate);

			DateToDMY (CalcDate, &dmy [0],&dmy [1],&dmy [2]);
			dmy [0]	=	1;
			dmy [1]	+=	6;
			if (dmy [1] > 12)
			{
				dmy [1]-= 12;
				dmy [2]++;
			}
			CalcDate = DMYToDate (dmy [0],dmy [1],dmy [2]);
			CalcDate = MonthEnd (CalcDate);
		}
	}
}

int
CalcTaxMvMonths
 (
	void
)
{
	int		NoMonths	=	0;
	double	TaxAmount	=	0.00;
	double	StartValue	=	0.00;
	double	MinValue	=	0.00;

	StartValue	=	 (famr_rec.max_deprec > 0.00) ? famr_rec.max_deprec
												 : famr_rec.tax_open_val;

	MinValue = StartValue * 0.001;

	while (1)
	{
		TaxAmount	=	DOLLARS (famr_rec.tax_d_pc);
		TaxAmount	*=	StartValue;
		StartValue 	-= no_dec (TaxAmount);
		if (TaxAmount < MinValue)
			break;
		else
			NoMonths++;
	}
	return (NoMonths);
}

int
CalcIntMvMonths
 (
	void
)
{
	int		NoMonths	=	0;
	double	IntAmount	=	0.00;
	double	StartValue	=	0.00;
	double	MinValue	=	0.00;

	StartValue	=	 (famr_rec.max_deprec > 0.00) ? famr_rec.max_deprec
												 : famr_rec.int_open_val;

	MinValue = StartValue * 0.001;

	while (1)
	{
		IntAmount	=	DOLLARS (famr_rec.int_d_pc);
		IntAmount	*=	StartValue;
		StartValue 	-= no_dec (IntAmount);
		if (IntAmount < MinValue)
			break;
		else
			NoMonths++;
	}
	return (NoMonths);
}

/*==================
| Add fagl records |
==================*/
void
AddFagl
 (
	long	CalcDate
)
{
	if (local_rec.tax_dtype [0] == 'D')
	{
		famr_rec.tax_d_amt	=	DOLLARS (famr_rec.tax_d_pc);
		famr_rec.tax_d_amt	*=	DV_TaxAmount;
		DV_TaxAmount -= famr_rec.tax_d_amt;
		if (famr_rec.tax_d_amt < DV_TaxMin)
		{
			famr_rec.tax_d_amt	+= DV_TaxAmount;
			DV_TaxAmount	=	0.00;
		}
	}
	if (local_rec.int_dtype [0] == 'D')
	{
		famr_rec.int_d_amt	=	DOLLARS (famr_rec.int_d_pc);
		famr_rec.int_d_amt	*=	DV_IntAmount;
		DV_IntAmount -= famr_rec.int_d_amt;
		if (famr_rec.int_d_amt < DV_IntMin)
		{
			famr_rec.int_d_amt	+= DV_IntAmount;
			DV_IntAmount	=	0.00;
		}
	}
		
	if (famr_rec.int_d_amt == 0.00 && famr_rec.tax_d_amt == 0.00)
		return;

	fagl_rec.famr_hash	=	famr_rec.famr_hash;
	fagl_rec.tran_date	=	CalcDate;
	fagl_rec.int_amt	=	famr_rec.int_d_amt;
	fagl_rec.tax_amt	=	famr_rec.tax_d_amt;
	fagl_rec.int_pc		=	famr_rec.int_d_pc;
	fagl_rec.tax_pc		=	famr_rec.tax_d_pc;
	strcpy (fagl_rec.posted, "N");
	strcpy (fagl_rec.dep_rule, famr_rec.dep_rule);
	strcpy (fagl_rec.crd_acc, famr_rec.gl_crd_acc);
	strcpy (fagl_rec.dbt_acc, famr_rec.gl_dbt_acc);
	strcpy (fagl_rec.stat_flag, "0");

	cc = abc_add (fagl, &fagl_rec);
	if (cc)
		file_err (cc, fagl, "DBADD");
}

/*==================
| Heading section. |
==================*/
int
heading
 (
	int scn
)
{
	if (!restart)
	{
		swide ();
		if (scn != cur_screen)
			scn_set (scn);

		clear ();
		pr_box_lines (scn);

		strcpy (err_str,ML (mlFaMess018));

		if (FA_DISPLAY)
			strcpy (err_str,ML (mlFaMess019));
		rv_pr (	err_str, (130 - strlen (err_str)) / 2,0, 1);

		us_pr (ML (mlFaMess011), 2,10, 1);

		us_pr (ML (mlFaMess012), 69,10, 1);

		print_at (0, 110,ML (mlFaMess007), local_rec.prev_asset);

		strcpy (err_str,ML (mlStdMess038));
		print_at (21, 0,err_str, comm_rec.co_no, comm_rec.co_name);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}

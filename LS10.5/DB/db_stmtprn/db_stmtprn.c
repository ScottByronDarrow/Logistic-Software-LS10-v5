/*=====================================================================
|  Copyright (C) 1988 - 1997 Logistic Software Limited.               |
|=====================================================================|
|  Program Name  : ( db_stmtprn.c )                                   |
|  Program Desc  : ( Customer's statement print.                    )   |
-----------------------------------------------------------------------
	$Log: db_stmtprn.c,v $
	Revision 5.2  2001/08/09 09:02:09  scott
	Updated to add FinishProgram () function
	
	Revision 5.1  2001/08/06 23:22:28  scott
	RELEASE 5.0
	
	Revision 5.0  2001/06/19 08:05:30  robert
	LS10-5.0 New Release as of 19 JUNE 2001
	
	Revision 4.0  2001/03/09 02:26:00  scott
	LS10-4.0 New Release as at 10th March 2001
	
	Revision 3.1  2001/02/02 04:39:59  scott
	Updated to correct small warning on gcc compiler
	
	Revision 3.0  2000/10/10 12:14:17  gerry
	Revision No. 3 Start
	<after Rel-10102000>
	
	Revision 2.0  2000/07/15 08:53:03  gerry
	Forced Revision No. Start 2.0 Rel-15072000
	
	Revision 1.14  2000/01/05 04:08:07  jonc
	Exclude printing child header/trailer for head-office account.
	
	Revision 1.13  1999/12/06 01:00:33  jonc
	Added yet another evaluation keyword. This time for GMS.
	
	Revision 1.12  1999/10/29 00:07:09  scott
	Updated for warnings.
	
	Revision 1.11  1999/10/11 22:09:36  jonc
	Added configuration file interface for optional output-to-filter
	
	Revision 1.10  1999/10/07 03:01:22  sunlei
	DPA 15294 Add evaluated keywords
	
	Revision 1.9  1999/10/05 07:34:40  scott
	Updated from ansi project.
	
	Revision 1.8  1999/07/16 00:19:29  jonc
	Adopted Logistic V10 db_stmtprn using format-p.
	
=====================================================================*/
#define CCMAIN
char    *PNAME = "$RCSfile: db_stmtprn.c,v $",
        *PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_stmtprn/db_stmtprn.c,v 5.2 2001/08/09 09:02:09 scott Exp $";

#define		NO_SCRGEN
#include	<pslscr.h>
#include	<dsp_screen.h>
#include	<dsp_process2.h>
#include	<FormatP.h>
#include	<twodec.h>

#include	<configfile.h>
#include	<dbgroup.h>

#include	"schema"

#define	OPEN_ITEM	(cumr_rec.stmt_type [0] == 'O')

FILE *	formatp;

int		envVarCoClose = TRUE;

static char	*data	= "data",
			*cumr2	= "cumr2",
			*cumr3	= "cumr3";

static const char 	*db_stmtprn_fp		= "db_stmtprn.fp";
static const char 	*CreditMarker		= "CR",
					*NullCreditMarker	= "  ";

/*
 *	Configuration file sections
 */
static const char
	*OptionSection				= "options";

/*
 *	Evaluated keywords
 */
static const char
	*EvalLineTranType			= "eval_line_tran_type",
	*EvalLineTranType3			= "eval_line_tran_type_3",
	*EvalLineDate				= "eval_line_date",
	*EvalLineReference			= "eval_line_reference",
	*EvalLineAmount				= "eval_line_amount",
	*EvalLineAmountDebit		= "eval_line_amount_debit",
	*EvalLineAmountCredit		= "eval_line_amount_credit",
	*EvalLineAmountAbsolute		= "eval_line_amount_abs",
	*EvalLineCreditMarker		= "eval_line_credit_marker",
	*EvalLineRunAmount			= "eval_line_run_value",
	*EvalLineForwardMarker		= "eval_line_forward_marker",

	*EvalStatementPeriod0Amt	= "eval_statement_period_0_amt",
	*EvalStatementPeriod1Amt	= "eval_statement_period_1_amt",
	*EvalStatementPeriod2Amt	= "eval_statement_period_2_amt",
	*EvalStatementPeriod3Amt	= "eval_statement_period_3_amt",
	*EvalStatementPeriod4Amt	= "eval_statement_period_4_amt",
	*EvalStatementPeriod34Amt	= "eval_statement_period_3_4_amt",
	*EvalStatementPeriod234Amt	= "eval_statement_period_2_3_4_amt",
	*EvalStatementOverdueAmt	= "eval_statement_overdue_amt",
	*EvalStatementDueAmt		= "eval_statement_due_amt",
	*EvalStatementPeriodFwdAmt	= "eval_statement_period_fwd_amt",
	*EvalStatementPeriod0FwdAmt	= "eval_statement_period_0_fwd_amt",
	*EvalStatementDueFwdAmt		= "eval_statement_due_fwd_amt",

	*EvalStatementPeriod0AmtAbs	= "eval_statement_period_0_amt_abs",
	*EvalStatementPeriod1AmtAbs	= "eval_statement_period_1_amt_abs",
	*EvalStatementPeriod2AmtAbs	= "eval_statement_period_2_amt_abs",
	*EvalStatementPeriod3AmtAbs	= "eval_statement_period_3_amt_abs",
	*EvalStatementPeriod4AmtAbs	= "eval_statement_period_4_amt_abs",
	*EvalStatementPeriod34AmtAbs= "eval_statement_period_3_4_amt_abs",
	*EvalStatementPeriod234AmtAbs= "eval_statement_period_2_3_4_amt_abs",
	*EvalStatementOverdueAmtAbs	= "eval_statement_overdue_amt_abs",
	*EvalStatementDueAmtAbs		= "eval_statement_due_amt_abs",
	*EvalStatementPeriodFwdAmtAbs= "eval_statement_period_fwd_amt_abs",
	*EvalStatementPeriod0FwdAmtAbs= "eval_statement_period_0_fwd_amt_abs",
	*EvalStatementDueFwdAmtAbs	= "eval_statement_due_fwd_amt_abs",

	*EvalStatementPeriod0Marker	= "eval_statement_period_0_credit_marker",
	*EvalStatementPeriod1Marker	= "eval_statement_period_1_credit_marker",
	*EvalStatementPeriod2Marker	= "eval_statement_period_2_credit_marker",
	*EvalStatementPeriod3Marker	= "eval_statement_period_3_credit_marker",
	*EvalStatementPeriod4Marker	= "eval_statement_period_4_credit_marker",
	*EvalStatementPeriod34Marker= "eval_statement_period_3_4_credit_marker",
	*EvalStatementPeriod234Marker= "eval_statement_period_2_3_4_credit_marker",
	*EvalStatementOverdueMarker	= "eval_statement_overdue_amt_credit_marker",
	*EvalStatementDueMarker		= "eval_statement_due_amt_credit_marker",
	*EvalStatementPeriodFMarker	= "eval_statement_period_fwd_marker",
	*EvalStatementPeriod0FwdMarker= "eval_statement_period_0_fwd_credit_marker",
	*EvalStatementDueFwdMarker	= "eval_statement_due_fwd_amt_credit_marker",

	*EvalPrintRunCount			= "eval_print_run_count",
	*EvalPrintRunValue			= "eval_print_run_value",

	*EvalChildDebtorNo			= "eval_child_dbt_no",
	*EvalChildDebtorName		= "eval_child_dbt_name",
	*EvalChildDebtorAcronym		= "eval_child_dbt_acronym";

/*
 *	Alternate bodies
 */
static const char
	*ChildDebtorBodyHeader		= "child-debtor-body-header",
	*ChildDebtorBodyTrailer		= "child-debtor-body-trailer";

/*
 *
 */
	struct commRecord	comm_rec;	/* System Common file */
	struct comrRecord	comr_rec;	/* Company Master File Base Record */
	struct cumrRecord	cumr_rec;	/* Customer Master Record */
	struct esmrRecord	esmr_rec;	/* Branch Master File Record */
	struct strmRecord	strm_rec;	/* Statement & Remmittance Messages */
	struct pocrRecord	pocr_rec;	/* Currency File Record */

/*=====================================================================
| Local Function prototypes.
=====================================================================*/
static void 	OpenDB 				(void);
static void 	CloseDB 			(void);
static void 	PrintGroup 			(FILE *, DGroup *, Money *);
static void 	PrintChildHeader 	(FILE *, long);
static void 	PrintChildTrailer 	(FILE *);
static int		Process 			(FILE *, long, int *, Money *);
static void		SubmitGroupTotal 	(FILE *,Money, const char *, const char *, const char *);

void ReadOthers (void);


/*=========================
| Here begins the program |
=========================*/
int
main (
 int                argc,
 char*              argv[])
{
	int		lpno;
	char	*sptr,
			stat_flag [10];
	long	hhcu_hash;
	char	pathname [128];
	ConfigFile *cf = NULL;

	/*
	 *	Read in config from stdin
	 */
	switch (scanf ("%d %s", &lpno, stat_flag))
	{
	case EOF	:
		return (EXIT_FAILURE);	/* believe it! - non-fatal error */
	case 2	:
		break;					/* everything's OK */
	default	:
		sys_err ("Failed to read in configuration", -1, PNAME);
	}

 	if ((sptr = chk_env ("CO_CLOSE")))
		envVarCoClose = (*sptr == '1') ? TRUE : FALSE;

	/*
	 * Open layout file
	 *
	 *	We look for:
	 *		1.	Current directory
	 *		2.	$PSL_MENU_PATH/LAYOUT
	 *		3.	$PROG_PATH/BIN/LAYOUT
	 */
	if (access (strcpy (pathname, db_stmtprn_fp), R_OK) != 0)
	{
		int		layout_exists = FALSE;
		char *	env;

		if ((env = getenv ("PSL_MENU_PATH")))
		{
			sprintf (pathname, "%s/LAYOUT/%s", env, db_stmtprn_fp);
			layout_exists = access (pathname, R_OK) == 0;
		}

		if ((!layout_exists && (env = getenv ("PROG_PATH"))))
		{
			sprintf (pathname, "%s/BIN/LAYOUT/%s", env, db_stmtprn_fp);
			layout_exists = access (pathname, R_OK) == 0;
		}

		if (!layout_exists)
			sys_err ("Layout file for db_stmtprn not found", errno, PNAME);
	}

	/*
	 *	Initialization
	 */
	cf = OpenConfig (argv [0]);

	OpenDB ();
	read_comm (comm_list, COMM_NO_FIELDS, (char *)&comm_rec);
	ReadOthers ();

	dsp_screen ("PRINTING STATEMENTS", comm_rec.co_no, comm_rec.co_name);

	if (OpenStatementGroups (comm_rec.dbt_date))
	{
		/*
		 *	Read in stream of hhcu_hash from stdin
		 *	For multiple prints (ie *stat_flag == 'M')
		 *		0
		 *		hashes..
		 *		0
		 *	For singular prints (ie *stat_flag == 'S')
		 *		hashes..
		 *		0
		*/
		char *	filter = NULL,
				filterpath [128];
		int		run_count = 0;
		Money	run_total = 0;

		if (cf)
		{
			filter = (char *) GetConfig (cf, OptionSection, "filter");
			if (filter)
				sprintf (filterpath, "|%s", filter);
		}

		if (stat_flag [0] == 'M')
		{
			if (scanf ("%ld", &hhcu_hash) != 1 || hhcu_hash)
				sys_err ("Expected leading 0 in MultiPrint", -1, PNAME);

			formatp = filter ?
				FormatPOpen (pathname, filterpath, NULL) :
				FormatPOpenLpNo (pathname, lpno, NULL);
		}

		/**	Keep on Process until last 0
		**/
		while (scanf ("%ld", &hhcu_hash) != EOF && hhcu_hash)
		{
			if (stat_flag [0] == 'S')
			{
				/*
				 *	One print-job per statement
				 */
				formatp = filter ?
					FormatPOpen (pathname, filterpath, NULL) :
					FormatPOpenLpNo (pathname, lpno, NULL);
			}
			assert (formatp);

			Process (formatp, hhcu_hash, &run_count, &run_total);

			if (stat_flag [0] == 'S')
				FormatPClose (formatp);
		}

		if (stat_flag [0] == 'M')
			FormatPClose (formatp);

		CloseStatementGroups ();
	}

	CloseDB (); 
	FinishProgram ();
	CloseConfig (cf);
	return (EXIT_SUCCESS);
}

static void
OpenDB (void)
{
	abc_dbopen (data);

	abc_alias (cumr2, cumr);
	abc_alias (cumr3, cumr);
	open_rec (comr,  comr_list, COMR_NO_FIELDS, "comr_co_no");
	open_rec (esmr,  esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (cuhd,  cuhd_list, CUHD_NO_FIELDS, "cuhd_hhcp_hash");
	open_rec (cuin,  cuin_list, CUIN_NO_FIELDS, "cuin_hhci_hash");
	open_rec (cumr,  cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (cumr2, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (cumr3, cumr_list, CUMR_NO_FIELDS, "cumr_ho_dbt_hash");
	open_rec (pocr,  pocr_list, POCR_NO_FIELDS, "pocr_id_no");
	open_rec (strm,  strm_list, STRM_NO_FIELDS, "strm_id_no");
}

static void
CloseDB (void)
{
	abc_fclose (comr);
	abc_fclose (esmr);
	abc_fclose (cumr);
	abc_fclose (cumr2);
	abc_fclose (cumr3);
	abc_fclose (cuhd);
	abc_fclose (cuin);
	abc_fclose (pocr);
	abc_fclose (strm);

	abc_dbclose (data);
}

static int
Process (
 FILE*              formatp,
 long               hhcu_hash,
 int*               run_count,
 Money*             run_total)
{
	/*
	 *	Print an actual statement
	 */
	int			i;
	int			base_submitted = FALSE;
	Date		cof_date;
	DGroupSet	groupSet;
	DGroup		*pGroup;
	Money		amtdue = 0,
				running = 0;
	long		curr_child = 0;

	/*
	 * find Debtor record, check statement flag
	 */
	if (find_hash (cumr, &cumr_rec, EQUAL, "r", hhcu_hash) ||
		cumr_rec.stmnt_flg[0] != 'Y')
	{
		return FALSE;
	}

	/*
	 * ignore debtors with head office; ie non-head office accounts.
	 */
	if (cumr_rec.ho_dbt_hash != 0L)
		return FALSE;

	if (OPEN_ITEM)
		cof_date = 0L;
	else
		cof_date = envVarCoClose ? comr_rec.stmt_date : esmr_rec.stmt_date;

	dsp_process ("Debtor : ", cumr_rec.dbt_acronym);

	if (OPEN_ITEM)
		LoadStatementGroups (&groupSet,
			cumr_rec.hhcu_hash,
			cof_date,
			DG_OPENITEM,
			NULL);
	else
		LoadStatementGroups (&groupSet,
			cumr_rec.hhcu_hash,
			cof_date,
			DG_BFORWARD,
			NULL);

	for (pGroup = GetFirstGroup (&groupSet);
		 pGroup;
		 pGroup = GetNextGroup (pGroup))
	{
		DGroupItem *	pItem = GetFirstGroupItem (pGroup);

		if (!twodec (DOLLARS (GetGroupValue (pGroup))))
		{
			/*
			 *	A resolved group
			 *
			 *	Check to see whether the stat flag of any of
			 *	the source have been marked as '9' or 'Z',
			 *	in which case we ignore the group (since it's
			 *	very likely to have been printed since the last time)
			 */
			struct cuhdRecord	cuhd_rec;
			struct cuinRecord	cuin_rec;

			switch (pItem -> source)
			{
			case DG_cuhd:
				if ((cc = find_hash (cuhd, &cuhd_rec, EQUAL, "r", pItem -> hhcp_hash)))
				{
					file_err (cc, (char *) cuhd, "FIND_HASH");
				}
				if (cuhd_rec.stat_flag [0] == '9' ||
					cuhd_rec.stat_flag [0] == 'Z')
				{
					continue;
				}
				break;

			case DG_cuin:
				if ((cc = find_hash (cuin, &cuin_rec, EQUAL, "r",
					pItem -> hhci_hash)))
				{
					file_err (cc, (char *) cuin, "FIND_HASH");
				}
				if (cuin_rec.stat_flag [0] == '9' ||
					cuin_rec.stat_flag [0] == 'Z')
				{
					continue;
				}
				break;

			case DG_null:
				/*
				 *	Brought forward value
				 */
				 break;

			default:
				sys_err ("Unknown group item source", -1, PNAME);
			}
		}

		/*
		 *	Worth continuing.
		 *
		 *	Submit base-info to formatp
		 */
		if (!base_submitted)
		{
			FormatPReset (formatp);

			FormatPSubmitTable (formatp, comr);
			FormatPSubmitTable (formatp, esmr);
			FormatPSubmitTable (formatp, strm);
			FormatPSubmitTable (formatp, cumr);

			/*
			 *	Submit totals:
			 *
			 *		groupSet.total [5] -> Forward transactions
			 */
			SubmitGroupTotal (formatp, groupSet.total [0],
				EvalStatementPeriod0Amt,
				EvalStatementPeriod0AmtAbs,
				EvalStatementPeriod0Marker);
			SubmitGroupTotal (formatp, groupSet.total[0] + groupSet.total[5],
				EvalStatementPeriod0FwdAmt,
				EvalStatementPeriod0FwdAmtAbs,
				EvalStatementPeriod0FwdMarker);
			SubmitGroupTotal (formatp, groupSet.total [1],
				EvalStatementPeriod1Amt,
				EvalStatementPeriod1AmtAbs,
				EvalStatementPeriod1Marker);
			SubmitGroupTotal (formatp, groupSet.total [2],
				EvalStatementPeriod2Amt,
				EvalStatementPeriod2AmtAbs,
				EvalStatementPeriod2Marker);
			SubmitGroupTotal (formatp, groupSet.total [3],
				EvalStatementPeriod3Amt,
				EvalStatementPeriod3AmtAbs,
				EvalStatementPeriod3Marker);
			SubmitGroupTotal (formatp, groupSet.total [4],
				EvalStatementPeriod4Amt,
				EvalStatementPeriod4AmtAbs,
				EvalStatementPeriod4Marker);

			/*
			 *	Since most people only have 3 periods displayed
			 */
			SubmitGroupTotal (formatp,
				groupSet.total [3] + groupSet.total [4],
				EvalStatementPeriod34Amt,
				EvalStatementPeriod34AmtAbs,
				EvalStatementPeriod34Marker);

			/*
			 *	For some of the others...
			 */
			SubmitGroupTotal (formatp,
				groupSet.total [2] + groupSet.total [3] + groupSet.total [4],
				EvalStatementPeriod234Amt,
				EvalStatementPeriod234AmtAbs,
				EvalStatementPeriod234Marker);

			/*
			 */
			SubmitGroupTotal (formatp, groupSet.total [5],
				EvalStatementPeriodFwdAmt,
				EvalStatementPeriodFwdAmtAbs,
				EvalStatementPeriodFMarker);

			/*
			 */
			for (i = 0; i < 5; i++)			/* exclude fwd trans (total [5]) */
				amtdue += groupSet.total [i];

			SubmitGroupTotal (formatp, amtdue,
				EvalStatementDueAmt,
				EvalStatementDueAmtAbs,
				EvalStatementDueMarker);
			SubmitGroupTotal (formatp, amtdue + groupSet.total [5],
				EvalStatementDueFwdAmt,
				EvalStatementDueFwdAmtAbs,
				EvalStatementDueFwdMarker);

			SubmitGroupTotal (formatp, amtdue - groupSet.total [0],
				EvalStatementOverdueAmt,
				EvalStatementOverdueAmtAbs,
				EvalStatementOverdueMarker);

			/*
			 *	Update run info
			 */
			(*run_count)++;
			*run_total += amtdue;

			FormatPSubmitInt (formatp, EvalPrintRunCount, *run_count);
			FormatPSubmitMoney (formatp, EvalPrintRunValue, *run_total);

			base_submitted = TRUE;
		}

		if (pItem -> child_hhcu != curr_child &&
			pItem -> child_hhcu != hhcu_hash)
		{
			if (curr_child)
				PrintChildTrailer (formatp);
			PrintChildHeader (formatp, curr_child = pItem -> child_hhcu);
		}

		PrintGroup (formatp, pGroup, &running);
	}

	FreeStatementGroups (&groupSet);

	return base_submitted;
}

void
ReadOthers ()
{
	strcpy (esmr_rec.co_no, comm_rec.co_no);
	strcpy (esmr_rec.est_no, comm_rec.est_no);
	if ((cc = find_rec (esmr, &esmr_rec, COMPARISON, "r")))
		file_err (cc, (char *) esmr, "find_rec");

	strcpy (comr_rec.co_no, comm_rec.co_no);
	if ((cc = find_rec (comr, &comr_rec, COMPARISON, "r")))
		file_err (cc, (char *) comr, "find_rec");

	strcpy (strm_rec.co_no, comm_rec.co_no);
	strcpy (strm_rec.est_no, comm_rec.est_no);
	strm_rec.mesg_type [0] = 'S';
	find_rec (strm,&strm_rec, EQUAL, "r");
}

static void
PrintGroup (
 FILE * formatp,
 DGroup * pGroup,
 Money * running)
{
	DGroupItem *	pItem;

	/*
	 * print list entry
	 */
	for (pItem = GetFirstGroupItem (pGroup);
		 pItem;
		 pItem = GetNextGroupItem (pItem))
	{
		char	*tr_type, *tr_type3;
		Money	value = pItem -> value,
				abs_value = fabs (value);

		switch (pItem -> type)
		{
		case 0:
			tr_type		= "RECEIPT";
			tr_type3	= "REC";
			break;
		case 1:
		case 4:
			tr_type		= "JOURNAL";
			tr_type3	=  "JNL";
			break;
		case 2:
			tr_type		= "INVOICE";
			tr_type3	= "INV";
			break;
		case 3:
			tr_type		= "CREDIT";
			tr_type3	= "CRD";
			break;
		case 5:
			tr_type		= "B/F BAL";
			tr_type3	= "B/F";
			break;
		default:
			tr_type		= "ERROR";
			tr_type3	= "ERR";
		}

		FormatPSubmitChars	(formatp, EvalLineTranType,		tr_type);
		FormatPSubmitChars	(formatp, EvalLineTranType3,	tr_type3);
		FormatPSubmitDate	(formatp, EvalLineDate,			pItem -> date);
		FormatPSubmitChars	(formatp, EvalLineReference,	pItem -> doc_no);
		FormatPSubmitMoney	(formatp, EvalLineAmount,		-value);
		FormatPSubmitMoney	(formatp, EvalLineAmountAbsolute, abs_value);

		/*
		 *	Submit an absolute value for the amount
		 *	as either a debit or credit value.
		 */
		if (pItem -> value < 0)
		{
			FormatPSubmitMoney (formatp, EvalLineAmountDebit, abs_value);
			FormatPSubmitChars (formatp, EvalLineAmountCredit, "");
			FormatPSubmitChars (formatp, EvalLineCreditMarker, NullCreditMarker);
		}
		else
		{
			FormatPSubmitChars (formatp, EvalLineAmountDebit, "");
			FormatPSubmitMoney (formatp, EvalLineAmountCredit, abs_value);
			FormatPSubmitChars (formatp, EvalLineCreditMarker, CreditMarker);
		}

		/*
		 */
		*running -= value;
		FormatPSubmitMoney (formatp, EvalLineRunAmount, *running);

		/*
		 */
		FormatPSubmitChars (formatp,
			EvalLineForwardMarker,
			pItem -> period == -1 ? "*" : " ");

		FormatPBatchEnd (formatp);
	}
}

static void
PrintChildHeader (
 FILE *             formatp,
 long               child_hhcu)
{
	struct cumrRecord	cumr2_rec;

	FormatPBody (formatp, ChildDebtorBodyHeader);

	if (find_hash (cumr2, &cumr2_rec, EQUAL, "r", child_hhcu))
	{
		/*
		 *	Child not found!
		 */
		const char *	Unknown = "***UNKNOWN***";

		FormatPSubmitChars (formatp, EvalChildDebtorNo, Unknown);
		FormatPSubmitChars (formatp, EvalChildDebtorName, Unknown);
		FormatPSubmitChars (formatp, EvalChildDebtorAcronym, Unknown);
	} else
	{
		FormatPSubmitChars (formatp,
			EvalChildDebtorNo, cumr2_rec.dbt_no);
		FormatPSubmitChars (formatp,
			EvalChildDebtorName, cumr2_rec.dbt_name);
		FormatPSubmitChars (formatp,
			EvalChildDebtorAcronym, cumr2_rec.dbt_acronym);
	}

	FormatPBatchEnd (formatp);				/* print the results */

	FormatPBody (formatp, NULL);			/* reset to default body */
}

static void
PrintChildTrailer (
 FILE*              formatp)
{
	/*
	 *	Print possible trailer
	 */
	FormatPBody (formatp, ChildDebtorBodyTrailer);
	FormatPBatchEnd (formatp);				/* print the results */
	FormatPBody (formatp, NULL);			/* reset to default body */
}

static void
SubmitGroupTotal (
 FILE*              formatp,
 Money              value,
 const char *       actual,
 const char *       absolute,
 const char *       crd_marker)
{
	FormatPSubmitMoney (formatp, actual, value);
	FormatPSubmitMoney (formatp, absolute, fabs (value));
	FormatPSubmitChars (formatp,
		crd_marker,
		value < 0 ? CreditMarker : NullCreditMarker);
}

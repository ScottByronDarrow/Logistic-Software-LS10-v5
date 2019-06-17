/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: ol_eod.c,v 5.2 2001/08/09 09:14:18 scott Exp $
|  Program Name  : (ol_eod.c) 
|  Program Desc  : (Online invoice system End of Day print.) 
|---------------------------------------------------------------------|
|  Author        : Huon Butterworth| Date Written  : 29/03/89         |
|---------------------------------------------------------------------|
| $Log: ol_eod.c,v $
| Revision 5.2  2001/08/09 09:14:18  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:32:45  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:09:47  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:30:59  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2001/02/05 09:18:40  scott
| Updated to add app.schema
|
| Revision 3.0  2000/10/10 12:16:48  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:02:23  gerry
| Forced Revision Start No 2.0 Rel-15072000
|
| Revision 1.19  2000/06/20 09:13:49  ramil
| SC#2959 LSANZ 16379 Modified to print department heading
|
| Revision 1.18  1999/12/06 01:47:31  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.17  1999/11/08 08:42:26  scott
| Updated due to warning errors when compiling using -Wall flag.
|
| Revision 1.16  1999/10/16 04:56:37  nz
| Updated for pjulmdy and pmdyjul routines.
|
| Revision 1.15  1999/09/29 10:11:24  scott
| Updated to be consistant on function names.
|
| Revision 1.14  1999/09/20 05:51:23  scott
| Updated from Ansi Project.
|
| Revision 1.13  1999/09/10 02:10:13  gerry
| SC 1772 - ANSI Compliance
|
| Revision 1.12  1999/06/15 09:39:17  scott
| Updated for log file.
|
=====================================================================*/
#define	CCMAIN
#define	CCMAIN
char	*PNAME = "$RCSfile: ol_eod.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/OL/ol_eod/ol_eod.c,v 5.2 2001/08/09 09:14:18 scott Exp $";

#include	<pslscr.h>
#include	<dsp_screen.h>
#include	<dsp_process.h>
#include	<pr_format3.h>
#include	<ml_std_mess.h>
#include	<ml_ol_mess.h>

#define		LINES	50
#define		TABSIZE	40
#define		INVOICE	 (cohr_rec.type [0] == 'I')
#define		CREDIT	 (cohr_rec.type [0] == 'C')
#define 	CASH    (!strcmp (cumr_rec.dbt_no, esmr_rec.sales_acc))
#define 	CHARGE  (strcmp (cumr_rec.dbt_no, esmr_rec.sales_acc))

#define		ITEM_NO		spcPtr->itemNumber
#define		GEN_NO		ITEM_NO
#define		ITEM_DESC	spcPtr->itemDesc
#define		ITEM_HASH	spcPtr->hhbrHash
#define		DP_DAY		spcPtr->dpDay
#define		DP_MTD		spcPtr->dpMtd
#define		ALL_DAY		spcPtr->allDay
#define		ALL_MTD		spcPtr->allMtd
#define		CURR_DP		cudp_rec.dp_no

#define		CHARGE_PRINT	0
#define		CREDIT_PRINT	1
#define		CASH_PRINT		2
#define		REFUND_PRINT	3
#define		SUMMARY_PRINT	4
#define		SPECIAL_PRINT	5

#define		TOTAL_PRINT	6

#define 	ALL_GEN		 (!strncmp (printFLags, "YYYYY", 5))

#define		D_DATE	&dateToday
#define		M_DATE	&summaryDate

#define		TOT_LABEL			genPtr->tot_label
#define		SUM_LABEL			genPtr->sum_label
#define		SRCH_TYPE			genPtr->inv_type
#define		START_DATE			*genPtr->start_date
#define		PROC_FUNC(x, y, z)	(*genPtr->proc_func) (x, y, z)
#define		TOT_FUNC(x)		 	(*genPtr->tot_func) (x)
#define		NETT_TOT			genPtr->nett_tot
#define		GROSS_TOT			genPtr->gross_tot
#define		MTD_TOT				genPtr->nett_mtd

#define		SUMM_TYPE	 		(*SRCH_TYPE == 'A')

#define		NET_INVOICE			GenTab [CHARGE_PRINT].nett_tot
#define		NET_CREDIT			GenTab [CREDIT_PRINT].nett_tot
#define		NET_CASH			GenTab [CASH_PRINT].nett_tot
#define		NET_REFUND			GenTab [REFUND_PRINT].nett_tot

#define		NET_CHARGE	 		(NET_INVOICE - NET_CREDIT)
#define		GROSS_CHARGE	 	(GenTab [CHARGE_PRINT].gross_tot - \
							  	 GenTab [CREDIT_PRINT].gross_tot)

#define		NET_CSH_SALES	 	(NET_CASH - NET_REFUND)

#define		GROSS_CSH_SALES	 	(GenTab [CASH_PRINT].gross_tot - \
								 GenTab [REFUND_PRINT].gross_tot)
		
#define		MTD_INVOICE			GenTab [CHARGE_PRINT].nett_mtd
#define		MTD_CREDIT			GenTab [CREDIT_PRINT].nett_mtd
#define		MTD_CASH			GenTab [CASH_PRINT].nett_mtd
#define		MTD_REFUND			GenTab [REFUND_PRINT].nett_mtd
#define		MTD_CHARGE	 		(MTD_INVOICE - MTD_CREDIT)
#define		MTD_CSH_SALES	 	(MTD_CASH - MTD_REFUND)
		

#include	"schema"

struct commRecord	comm_rec;
struct esmrRecord	esmr_rec;
struct cudpRecord	cudp_rec;
struct cumrRecord	cumr_rec;
struct inmrRecord	inmr_rec;
struct cohrRecord	cohr_rec;
struct colnRecord	coln_rec;

static	char	printFLags [10];
static	long	PV_kit_hash = 0L;


struct	genParms
{
char	*tot_label,		/*	Total format label.   */
		*sum_label,		/*	Summary format label. */
		*inv_type;		/*	Invoice type flag.    */
int		 (*proc_func) (struct genParms *, int, int);
						/*	Processing function.  */
void	 (*tot_func) (struct genParms *);
						/*	Total print function. */
long	*start_date;	/*	Start date for find.  */
double	nett_tot,		/*	Nett total.	      	  */
		gross_tot,		/*	Gross total.          */
		nett_mtd;		/*	Nett MTD.  	      	  */
};

	int		printerNumber 	= 1,
			pageNumber 		= 0,
			lineNumber 		= 0,
			spcTableSize	= 0,
			mtdDays 		= 0;

	long	summaryDate		= 0L,
			lastDate		= 0L;

	time_t	dateToday;

	FILE	*fin,
			*fout;
	
	char	findFlags [13];
	int		processStatus = FALSE;


	typedef	struct
	{							/*------------------------*/
		char	*itemNumber,	/*	Item no.	      	  */
				*itemDesc;		/*	Item desciption.      */
		long	hhbrHash;		/*	Item hash.     	      */
								/* Department :-	      */
		double	dpDay,			/*	Daily Nett total.     */
		      	dpMtd;			/*	MTD. Nett total.      */
								/* All Departments :-	  */
		double	allDay,			/*	Daily Nett total.     */
		      	allMtd;			/*	MTD. Nett total.      */
	}							/*------------------------*/
	SPC_PARMS, *SPC_PTR;

static	SPC_PARMS  spcTab [TABSIZE + 1];

/*
 *	Local functions
 */
SPC_PTR 		SelectItem 		(long);
int 			SpcLine 		(char *, char *, char *, double, double);
int 			WorkDay 		(long);
static	int		ReadCohr		(int);
static	int		ReadCudp 		(int);
static	int		SpcCheck		(void);
static	void	SpcIO 			(void);
static	void 	OpenIO 			(void);
static	void 	ReadInmr 		(SPC_PTR spcPtr);
static 	int		DailyTotals 	(struct genParms *, int);
static 	int		GenCheck 		(struct genParms *);
static 	int		GenInit 		(struct genParms *, long);
static 	int		MtdTotal 		(int);
static 	int		PrintSet 		(int);
static 	int		ProcessCash 	(struct genParms *, int, int);
static 	int		ProcessCharge 	(struct genParms *, int, int);
static 	int		ProcessSummary 	(struct genParms *, int, int);
static 	int		ReadRecords 	(struct genParms *, int);
static 	int		SkipCash 		(struct genParms *, int);
static 	int		SkipCharge 		(struct genParms *, int);
static 	int 	SpcInit 		(void);
static 	void	ReadEsmr		(void);
static 	void 	CashTotal 		(struct genParms *);
static 	void 	ChargeTotal 	(struct genParms *);
static 	void 	GenHeading 		(int);
static 	void 	GenerateTot 	(struct genParms *);
static 	void 	PageBreak 		(int);
static 	void 	PrintTotals 	(void);
static 	void 	ProcCudp 		(void);
static 	void 	SummaryLine 	(char *, double, double);
static 	void 	SummaryTot 		(struct genParms *);
static void 	AllTotals 		(void);
static void 	SpcHead 		(char *);
static void 	SpcTotals 		(void);
void			TotalSpace 		(int);
void 			CloseDB			(void);
void 			LineFeed 		(int, struct genParms *, int);
void 			OpenDB			(void);
void 			ProcColn 		(void);
void 			SetupTable 		(void);
void 			SpcPrint 		(void);
void 			SpcTotalLine	(int, char *, double, double);

static	struct	genParms GenTab [] =
{
	{"TOT_INV", "INV_SUM", "I", ProcessCharge, 	GenerateTot,  D_DATE, 0, 0, 0},
	{"TOT_CRD", "CRD_SUM", "C", ProcessCharge, 	ChargeTotal,  D_DATE, 0, 0, 0},
	{"TOT_CSH", "CSH_SUM", "I", ProcessCash, 	GenerateTot,  D_DATE, 0, 0, 0},
	{"TOT_RFD", "RFD_SUM", "C", ProcessCash, 	CashTotal,    D_DATE, 0, 0, 0},
	{" ",       "ALL_SUM", "A", ProcessSummary, SummaryTot,   M_DATE, 0, 0, 0},
	{NULL, NULL, NULL, NULL, NULL, 0L, 0, 0, 0}
};

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
	int		argc,
	char 	*argv [])
{
	int		ptrCnt, 
			findFlag;
	struct  genParms	*genPtr;
	struct	tm	*tm_ptr;

	if (argc < 3)
	{
		printf (mlOlMess002, argv [0]);
		return (EXIT_FAILURE);
	}

	pageNumber 		= lineNumber = 0;
	printerNumber 	= atoi (argv [1]);
	strcpy (printFLags, argv [2]);
	strcpy (findFlags, "            ");
	processStatus = FALSE;
	if (argc == 4)
	{
		processStatus = TRUE;
		sprintf (findFlags, "%-12.12s", argv [3]);
	}

	OpenDB ();
	OpenIO ();

	dateToday		=	time (NULL);
	tm_ptr = localtime (&dateToday);

	set_wdays (tm_ptr->tm_mday, tm_ptr->tm_wday);
	dateToday 	= (long) TodaysDate ();
	summaryDate = MonthStart (dateToday);
	
	dsp_screen ("Online End Of Day Print.",
				comm_rec.co_no,comm_rec.co_name);

	for (genPtr = GenTab, ptrCnt = 0; TOT_LABEL; genPtr++, ptrCnt++)
	{
		findFlag =	GenInit 
					(
						genPtr, 
						(processStatus) ? summaryDate : START_DATE
					);

		if (PrintSet (ptrCnt))
		{
			GenHeading (ptrCnt);
			while (!PROC_FUNC (genPtr, ptrCnt, findFlag))
				findFlag = NEXT;

			TOT_FUNC (genPtr);
			LineFeed (2, genPtr, ptrCnt);
		}
	}

	if (PrintSet (SPECIAL_PRINT))
		SpcPrint ();

	fprintf (fout,".EOF\n");
	pclose 	(fout);
	fclose 	(fin);
	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

static void
OpenIO (
	void)
{
	/*-----------------------------------------
	| Open format (.p) file & pipe to pformat | 
 	-----------------------------------------*/
	if (! (fin = pr_open ("ol_eod.p")))
		file_err (errno, "ol_eod.p", "pr_open");

	if ((fout = popen ("pformat","w")) == NULL)
		file_err (errno, "pformat", "POPEN");

	/*---------------------------------
	| Start output to standard print. |
	---------------------------------*/
	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (fout,".SO\n");
	fprintf (fout,".LP%d\n",printerNumber);
	fprintf (fout,".PL999\n");
	fprintf (fout,".7\n");
	fprintf (fout,".L106\n");
	fprintf (fout,".PI10\n");
	fprintf (fout,".E%s\n",clip (comm_rec.co_name));
	fprintf (fout,".E%s\n",clip (comm_rec.est_name));
	fprintf (fout,".B1\n");
	fprintf (fout,".E%s\n", "DAILY SALES SUMMARY");
	fprintf (fout,".EAS AT : %s.B1\n", SystemTime ()); 
	fprintf (fout,".B1\n");
}

static int
ProcessCharge (
	struct genParms *genPtr,
	int		tranType,
	int		findFlag)
{	
	if (!SkipCash (genPtr, findFlag))
		return (EXIT_FAILURE);

	return (DailyTotals (genPtr, tranType));
}

static int
ProcessCash (
	struct genParms *genPtr,
	int tranType, 
	int findFlag)
{	
	if (!SkipCharge (genPtr, findFlag))
		return (EXIT_FAILURE);

	return (DailyTotals (genPtr, tranType));
}

static int
DailyTotals (
	struct 	genParms	*genPtr,
	int 	tranType)
{
	int		validCohr = FALSE;
	double	nett 	= 0.00, 
			gross 	= 0.00;

	PageBreak (tranType);

	nett = ((cohr_rec.gross + cohr_rec.freight + cohr_rec.tax) - cohr_rec.disc);

	if (processStatus)
	{
		if (strstr (findFlags, cohr_rec.stat_flag))
			validCohr = TRUE;
	}
	else
	{
		if (cohr_rec.date_raised == (long) dateToday)
			validCohr = TRUE;
	}

	if (validCohr)
	{
		gross = ((cohr_rec.gross + cohr_rec.freight + 
			  	  cohr_rec.gst + cohr_rec.tax) - cohr_rec.disc);

		pr_format (fin, fout, "EOD_DETAIL", 1, cohr_rec.dp_no);
		pr_format (fin, fout, "EOD_DETAIL", 2, cohr_rec.inv_no);
		pr_format (fin, fout, "EOD_DETAIL", 3, nett);
		pr_format (fin, fout, "EOD_DETAIL", 4, gross);
		NETT_TOT += nett;
		GROSS_TOT += gross;
	}
	
	MTD_TOT += nett;

	return (EXIT_SUCCESS);
}

static int
ProcessSummary (
	struct 	genParms *genPtr,
	int 	tranType, 
	int 	findFlag)
{	
	if (!ReadRecords (genPtr, findFlag))
		return (EXIT_FAILURE);

	return (MtdTotal (tranType));
}

static int
MtdTotal (
	int		tranType)
{
	struct	genParms	*genPtr;
	int		printType;
	double	nett = 0.00;

	nett = ((cohr_rec.gross + cohr_rec.freight + cohr_rec.tax) - cohr_rec.disc);

	printType = (CASH && INVOICE)	? CASH_PRINT	:
		    	(CASH && CREDIT)	? REFUND_PRINT	:
		    	(CHARGE && CREDIT)	? CREDIT_PRINT	: CHARGE_PRINT;
	genPtr = &GenTab [printType];

	if (processStatus)
	{
		if (strstr (findFlags, cohr_rec.stat_flag))
			NETT_TOT += nett;
	}
	else
	{
		if (cohr_rec.date_raised == (long) dateToday)
			NETT_TOT += nett;
	}

	if (cohr_rec.date_raised >= summaryDate)
		MTD_TOT += nett;

	if (cohr_rec.type [0] == 'I')
		if (cohr_rec.date_raised > lastDate)
		{
			lastDate = cohr_rec.date_raised;
			if (WorkDay (lastDate))
				mtdDays++;
		}

	return (EXIT_SUCCESS);
}

static void
PageBreak (
	int tranType)
{
	if (lineNumber++ >= LINES)
		GenHeading (tranType);
}

static void
GenHeading (
	int tranType)
{
	char	tmp_str [128];

	if (lineNumber >= LINES)
	{
		fprintf (fout,".PA\n.B1\n");
		lineNumber = 0;
	}
	if (tranType == SUMMARY_PRINT)
	{
		dsp_process (": ", "SUMMARY");
		return;
	}

	strcpy (tmp_str, (tranType == CHARGE_PRINT) ? "CHARGE SALES" :
			 (tranType == CREDIT_PRINT) ? "CREDIT NOTES" :
			 (tranType == CASH_PRINT)   ? "CASH SALES"	  :
			 (tranType == REFUND_PRINT) ? "CASH REFUNDS" :
						   "TOTALS");
	dsp_process (": ", tmp_str);
	fprintf (fout, ".E%s\n",	tmp_str);
	fprintf (fout,".B1\n");

	lineNumber += 2;
	if (tranType == TOTAL_PRINT)
		return;

	pr_format (fin,fout,"COL_HEAD",0,0);
	pr_format (fin, fout, "DET_ULINE", 0, 0);
	lineNumber += 2;
}

static void
PrintTotals (
	void)
{
	if (!ALL_GEN || ! (GROSS_CSH_SALES + GROSS_CHARGE))
		return;

	TotalSpace (3);
	pr_format (fin, fout, "TOT_ULINE", 0, 0);
	pr_format (fin, fout, "ALL_TOT", 1, NET_CSH_SALES + NET_CHARGE);
	pr_format (fin, fout, "ALL_TOT", 2, GROSS_CSH_SALES + GROSS_CHARGE);
	pr_format (fin, fout, "TOT_ULINE", 0, 0);
	lineNumber += 3;
}

static void
SummaryTot (
	struct genParms *psUnused)
{
	TotalSpace (14);
	pr_format (fin, fout, "SUM_HEAD", 0, 0);
	pr_format (fin, fout, "SUM_ULINE", 0, 0);

	SummaryLine ("INV_SUM", NET_INVOICE, MTD_INVOICE);
	SummaryLine ("CRD_SUM", NET_CREDIT, MTD_CREDIT);

	fprintf (fout, ".B1\n");
	SummaryLine ("CHRG_SUM", NET_INVOICE - NET_CREDIT,
				  MTD_INVOICE - MTD_CREDIT);
	fprintf (fout, ".B1\n");

	SummaryLine ("CSH_SUM", NET_CASH, MTD_CASH);
	SummaryLine ("RFD_SUM", NET_REFUND, MTD_REFUND);

	fprintf (fout, ".B1\n");
	SummaryLine ("CASH_SUM", NET_CASH - NET_REFUND,
				  MTD_CASH - MTD_REFUND);

	pr_format (fin, fout, "SUM_ULINE", 0, 0);
	SummaryLine ("ALL_SUM", NET_CHARGE + NET_CSH_SALES,
				 MTD_CHARGE + MTD_CSH_SALES);
	pr_format (fin, fout, "SUM_ULINE", 0, 0);
}

static void
SummaryLine (
 char	*s_label,
 double s_tot1,
 double	s_tot2)
{
	pr_format (fin, fout, s_label, 1, s_tot1);
	pr_format (fin, fout, s_label, 2, s_tot2);
	if (mtdDays)
		pr_format (fin, fout, s_label, 3, s_tot2 / mtdDays);
	else
		pr_format (fin, fout, s_label, 3, 0.0);
	
}

static void
ChargeTotal (
	struct genParms *genPtr)
{
	GenerateTot (genPtr);

	if (!PrintSet (CHARGE_PRINT) || !PrintSet (CREDIT_PRINT))
		return;

	pr_format (fin, fout, "TOT_ULINE", 0, 0);
	pr_format (fin, fout, "CHARGE_TOT", 1, NET_CHARGE);
	pr_format (fin, fout, "CHARGE_TOT", 2, GROSS_CHARGE);
	pr_format (fin, fout, "TOT_ULINE", 0, 0);
	lineNumber += 3;
}

static void
CashTotal (
	struct genParms *genPtr)
{
	GenerateTot (genPtr);

	if (!PrintSet (CASH_PRINT) || !PrintSet (REFUND_PRINT))
		return;

	TotalSpace (3);
	pr_format (fin, fout, "TOT_ULINE", 0, 0);
	pr_format (fin, fout, "CASH_TOT", 1, NET_CSH_SALES);
	pr_format (fin, fout, "CASH_TOT", 2, GROSS_CSH_SALES);
	pr_format (fin, fout, "TOT_ULINE", 0, 0);
	lineNumber += 3;

	PrintTotals ();
}

static void
GenerateTot (
	struct genParms *genPtr)
{
	TotalSpace (3);
	pr_format (fin, fout, "TOT_ULINE", 0, 0);
	pr_format (fin, fout, TOT_LABEL, 1, NETT_TOT);
	pr_format (fin, fout, TOT_LABEL, 2, GROSS_TOT);
	pr_format (fin, fout, "TOT_ULINE", 0, 0);
	lineNumber += 3;
}

static int
PrintSet (
 int	ptrCnt)
{
	return (printFLags [ptrCnt] == 'Y');
}

static int
GenInit (
 struct	genParms 	*genPtr,
 long 				f_date)
{
	strcpy (cohr_rec.co_no, esmr_rec.co_no);
	strcpy (cohr_rec.br_no, esmr_rec.est_no);
	cohr_rec.date_raised = f_date;
	strcpy (cohr_rec.type, SRCH_TYPE);
	strcpy (cohr_rec.dp_no, "  ");
	strcpy (cohr_rec.inv_no, "        ");

	mtdDays = 0;
	lastDate = f_date - 1;

	if (SUMM_TYPE)
	{
		for (genPtr = GenTab; TOT_LABEL; genPtr++)
			NETT_TOT = GROSS_TOT = MTD_TOT = 0.0;
	}

	return (GTEQ);
}

static int
SkipCash (
 struct	genParms	*genPtr,
 int	findFlag)
{
	do {
		if (!ReadRecords (genPtr, findFlag))
			return (FALSE);

		findFlag = NEXT;
	} while (CASH);

	return (TRUE);
}

static int
SkipCharge (
 struct	genParms	*genPtr,
 int				findFlag)
{
	do {
		if (!ReadRecords (genPtr, findFlag))
			return (FALSE);

		findFlag = NEXT;
	}
	   while (!CASH);

	return (TRUE);
}

static int
ReadRecords (
 struct	genParms	*genPtr,
 int	findFlag)
{
	if (find_rec (cohr, &cohr_rec, findFlag, "r") || !GenCheck (genPtr))
		return (FALSE);

	cumr_rec.hhcu_hash	=	cohr_rec.hhcu_hash;
	if (find_rec (cumr, &cumr_rec, COMPARISON, "r"))
		strcpy (cumr_rec.dbt_no,"999999");

	return (TRUE);
}

static int
GenCheck (
 struct	genParms	*genPtr)
{
	if (strcmp (cohr_rec.co_no, comm_rec.co_no) ||
		strcmp (cohr_rec.br_no, comm_rec.est_no) ||
			cohr_rec.date_raised > (long) dateToday)
		return (FALSE);

	if ((!SUMM_TYPE && cohr_rec.type [0] != SRCH_TYPE [0]) ||
					 (SUMM_TYPE && (!INVOICE && !CREDIT)))
		return (FALSE);

	return (TRUE);
}

/*=======================================
| Specific part no. Processing Routine. |
=======================================*/
void
SpcPrint (
	void)
{
	int	findFlag = GTEQ;

	abc_selfield (cohr, "cohr_id_no6");
	SetupTable ();
	SpcIO ();

	while (ReadCudp (findFlag))
	{
		ProcCudp ();
		findFlag = NEXT;
	}
	AllTotals ();
}

static void
ProcCudp (
	void)
{
	if (!SpcInit ())
		return;
	do
	{
		if (INVOICE || CREDIT)
			ProcColn ();
	}
	  while (ReadCohr (NEXT));

	SpcTotals ();
}

void
ProcColn (
	void)
{
	SPC_PTR	spcPtr;
	int	findFlag = GTEQ;
	double	nett;

	coln_rec.hhco_hash = cohr_rec.hhco_hash;
	coln_rec.line_no = 0;

	while (!find_rec (coln, &coln_rec, findFlag, "r") &&
				coln_rec.hhco_hash == cohr_rec.hhco_hash)
	{
		findFlag = NEXT;
		if (coln_rec.hhbr_hash == PV_kit_hash)
			continue;

		spcPtr = SelectItem (coln_rec.hhbr_hash);

		nett = ((coln_rec.gross + coln_rec.amt_tax) - coln_rec.amt_disc);

		if (processStatus)
		{
			if (strstr (findFlags, cohr_rec.stat_flag))
			{
				DP_DAY  += (CREDIT) ? -nett : nett;
				ALL_DAY += (CREDIT) ? -nett : nett;
			}
		}
		else
		{
		     	if (cohr_rec.date_raised == (long) dateToday)
			{
				DP_DAY  += (CREDIT) ? -nett : nett;
				ALL_DAY += (CREDIT) ? -nett : nett;
			}
		}

		DP_MTD  += (CREDIT) ? -nett : nett;
		ALL_MTD += (CREDIT) ? -nett : nett;
	}
}

SPC_PTR
SelectItem (
	long hhbr_hash)
{
	SPC_PTR	spcPtr;

	for (spcPtr = &spcTab [1]; ITEM_NO; spcPtr++)
		if (hhbr_hash == ITEM_HASH)
			return (spcPtr);

	return (spcTab);
}

static void
SpcHead (
	char *title)
{
	if (lineNumber >= LINES || (spcTableSize + 8 + lineNumber) >= LINES)
	{
		fprintf (fout,".PA\n");
		lineNumber = 0;
	}
	fprintf (fout,".B1\n");

	if (title)
	{
		dsp_process (": ", title);
		fprintf (fout, ".EDEPARTMENT : %s\n", clip (title));
	}
	else
	{
		dsp_process (": ", "ALL DEPARTMENTS");
		fprintf (fout, ".EALL DEPARTMENTS\n");
	}
	fprintf (fout, ".B1\n");
	pr_format (fin, fout, "SPC_HEAD", 0, 0);
	pr_format (fin, fout, "SPC_RULE", 0, 0);
	lineNumber += 6;
}

static void
SpcTotals (
	void)
{
	SPC_PTR	spcPtr;
	double	day_tot = 0.0, 
		mtd_tot = 0.0;

	SpcHead (cudp_rec.dp_name);

	for (spcPtr = &spcTab [1]; ITEM_NO; spcPtr++)
	{
		SpcLine ("SPC_LINE", ITEM_NO, ITEM_DESC, DP_DAY, DP_MTD);
		day_tot += DP_DAY;
		mtd_tot += DP_MTD;
	}
	pr_format (fin, fout, "SPC_ULINE", 0, 0);
	SpcTotalLine (FALSE, "SPC_SUBTOT", day_tot, mtd_tot);
	spcPtr = spcTab;
	fprintf (fout, ".B1\n");
	SpcTotalLine (TRUE, GEN_NO, DP_DAY, DP_MTD);
	day_tot += DP_DAY;
	mtd_tot += DP_MTD;

	SpcTotalLine (TRUE, "SPC_TOTAL", day_tot, mtd_tot);
}

static void
AllTotals (
	void)
{
	SPC_PTR	spcPtr;
	double	day_tot = 0.0, 
		mtd_tot = 0.0;

	SpcHead (NULL);

	for (spcPtr = &spcTab [1]; ITEM_NO; spcPtr++)
	{
		if (SpcLine ("SPC_LINE", ITEM_NO, ITEM_DESC, ALL_DAY, ALL_MTD))
		{
			day_tot += ALL_DAY;
			mtd_tot += ALL_MTD;
		}
	}
	pr_format (fin, fout, "SPC_ULINE", 0, 0);
	SpcTotalLine (FALSE, "SPC_SUBTOT", day_tot, mtd_tot);
	spcPtr = spcTab;
	fprintf (fout, ".B1\n");
	SpcTotalLine (TRUE, GEN_NO, ALL_DAY, ALL_MTD);
	day_tot += ALL_DAY;
	mtd_tot += ALL_MTD;

	SpcTotalLine (TRUE, "SPC_TOTAL", day_tot, mtd_tot);
}

int
SpcLine (
	char *label, 
	char *item, 
	char *desc, 
	double dpDay, 
	double dpMtd)
{
	pr_format (fin, fout, label, 1, item);
	pr_format (fin, fout, label, 2, desc);
	pr_format (fin, fout, label, 3, dpDay);
	pr_format (fin, fout, label, 4, dpMtd);
	if (mtdDays)
		pr_format (fin, fout, label, 5, dpMtd / mtdDays);
	else
		pr_format (fin, fout, label, 5, 0.0);

	lineNumber ++;

	return (TRUE);
}

void
SpcTotalLine (
	int uline_flag, 
	char *label, 
	double tot_day, 
	double tot_mtd)
{
	pr_format (fin, fout, label, 1, tot_day);
	pr_format (fin, fout, label, 2, tot_mtd);
	if (mtdDays)
		pr_format (fin, fout, label, 3, tot_mtd / mtdDays);
	else
		pr_format (fin, fout, label, 3, 0.0);

	lineNumber++;
	if (uline_flag)
	{
		pr_format (fin, fout, "SPC_ULINE", 0, 0);
		lineNumber++;
	}
}

static int
SpcInit (
	void)
{
	SPC_PTR	spcPtr;

	strcpy (cohr_rec.co_no, comm_rec.co_no);
	strcpy (cohr_rec.br_no, comm_rec.est_no);
	strcpy (cohr_rec.dp_no, CURR_DP);
	cohr_rec.date_raised = summaryDate;
	strcpy (cohr_rec.type, "C");
	strcpy (cohr_rec.inv_print, " ");

	if (!ReadCohr (GTEQ))
		return (FALSE);

	for (spcPtr = spcTab; ITEM_NO; spcPtr++)
		DP_DAY = DP_MTD = 0;

	return (TRUE);
}

void
SetupTable (
	void)
{
	SPC_PTR	spcPtr;
	char	tmp_str [129];
	FILE	*ctl_in;
	int	tabsize;

	if ((ctl_in = pr_open ("ol_spc.ctl")) == NULL)
		file_err (errno, "ol_spc.ctl", "PR_OPEN"); 

	if (!(tabsize = atoi (fgets (tmp_str, 128, ctl_in))) || tabsize > TABSIZE)
		file_err (errno, "ol_spc.ctl", "no items exist in ol_spc.ctl");

	strcpy (inmr_rec.co_no, comm_rec.co_no);
	sprintf (inmr_rec.item_no, "%-16.16s", "\\\\");
	if ((cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r")))
		PV_kit_hash = 0L;
	else
		PV_kit_hash = inmr_rec.hhbr_hash;

	spcPtr = spcTab;
	if (! (ITEM_NO = strdup ("SPC_GEN")))
		file_err (errno, "SPC_GEN", "strdup");

	spcTableSize = tabsize;
	for (spcPtr = spcTab + 1; tabsize--; spcPtr++)
	{
		fgets (tmp_str, 128, ctl_in);

		tmp_str [strlen (tmp_str) - 1] = '\0';
		if (! (ITEM_NO = strdup (tmp_str)))
			file_err (errno, tmp_str, "strdup");

		ReadInmr (spcPtr);

		if (! (ITEM_DESC = strdup (inmr_rec.description)))
			file_err (errno, inmr_rec.description, "ITEM_DESC");

		ITEM_HASH = inmr_rec.hhbr_hash;
	}
	ITEM_NO = NULL;

	fclose (ctl_in);
}

static void
ReadInmr (
 SPC_PTR	spcPtr)
{
	char	tmp_str [81];

	sprintf (tmp_str, "inmr item (%s) during (SetupTable)", ITEM_NO);
	strcpy (inmr_rec.co_no, comm_rec.co_no);
	sprintf (inmr_rec.item_no, "%-16.16s", ITEM_NO);
	if ((cc = find_rec (inmr, &inmr_rec, COMPARISON, "r")))
		file_err (cc, tmp_str, "DBFIND");
}

static int
ReadCudp (
 int	findFlag)
{
	strcpy (cudp_rec.co_no, comm_rec.co_no);
	strcpy (cudp_rec.br_no, comm_rec.est_no);
	strcpy (cudp_rec.dp_no, "  ");

	if ((cc = find_rec (cudp, &cudp_rec, findFlag, "r") ||
		strcmp (cudp_rec.co_no, comm_rec.co_no) ||
			strcmp (cudp_rec.br_no, comm_rec.est_no)))
		return (FALSE);

	return (TRUE);
}

static int
ReadCohr (
 int	findFlag)
{
	if (find_rec (cohr, &cohr_rec, findFlag, "r") || !SpcCheck ())
		return (FALSE);

	if (strcmp (cohr_rec.dp_no, CURR_DP))
		return (FALSE);

	return (TRUE);
}

static int
SpcCheck (
	void)
{
	if (strcmp (cohr_rec.co_no, comm_rec.co_no) ||
		strcmp (cohr_rec.br_no, comm_rec.est_no) ||
				cohr_rec.date_raised > (long) dateToday)
		return (FALSE);

	return (TRUE);
}

static void
SpcIO (
	void)
{
	fprintf (fout, ".PA\n");
	lineNumber = 0;
	fflush (fout);
}

void
LineFeed (
	int noLines, 
	struct genParms *genPtr, int tranType)
{
	lineNumber += noLines;
	fprintf (fout,".B%d\n", noLines);
}

/*=====================================
| Get branch info from database file. |
=====================================*/
static void
ReadEsmr (
	void)
{
	strcpy (esmr_rec.co_no, comm_rec.co_no);
	strcpy (esmr_rec.est_no, comm_rec.est_no);
	cc = find_rec (esmr, &esmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, esmr, "DBFIND");
}

/*---------------------------
| Open main database files. |
---------------------------*/
void
OpenDB (
	void)
{		
	abc_dbopen ("data");
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (cudp, cudp_list, CUDP_NO_FIELDS, "cudp_id_no");
	open_rec (cohr, cohr_list, COHR_NO_FIELDS, "cohr_id_no5");
	open_rec (coln, coln_list, COLN_NO_FIELDS, "coln_id_no");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	ReadEsmr ();
}

void
CloseDB (
	void)
{
	abc_fclose (esmr);
	abc_fclose (cudp);
	abc_fclose (coln);
	abc_fclose (cohr);
	abc_fclose (cumr);
	abc_fclose (inmr);
	abc_dbclose ("data");
}

void
TotalSpace (
	int n_lines)
{
	if (lineNumber + n_lines > LINES)
	{
		lineNumber = LINES;
		GenHeading (TOTAL_PRINT);
	}
}

int
WorkDay (
	long int t_date)
{
	int		dayPeriod;

	DateToDMY (t_date, &dayPeriod, NULL, NULL);

	return (is_weekday (dayPeriod));
}

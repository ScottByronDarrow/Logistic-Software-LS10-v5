/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: _reorder.c,v 5.4 2002/11/25 03:16:39 scott Exp $
|  Program Name  : (lrp_reorder.c)                                    |
|  Program Desc  : (Print Reorder Review Report.               )     |
|---------------------------------------------------------------------|
|  Date Written  : (18/08/1998)    | Author      : Scott B Darrow     |
|---------------------------------------------------------------------|
| $Log: _reorder.c,v $
| Revision 5.4  2002/11/25 03:16:39  scott
| Updated to use chk_env instead of get_env when applicable.
|
| Revision 5.3  2002/09/16 05:04:18  scott
| Updated to allow days demend to be shown on screen
|
| Revision 5.2  2001/08/09 09:29:57  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:27:42  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:07:37  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:28:45  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.2  2001/01/26 05:11:40  scott
| Updated to remove code not being used.
|
| Revision 3.1  2001/01/26 01:53:59  scott
| Updated after code checking.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: _reorder.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/LRP/lrp_reorder/_reorder.c,v 5.4 2002/11/25 03:16:39 scott Exp $";

#include 	<pslscr.h>
#include 	<ml_std_mess.h>
#include 	<ml_lrp_mess.h>
#include 	<twodec.h>

#define	MAX_CCMR	100
#define	MAX_SUPP	9

#define	PRINTER		0
#define	DATABASE	1

#define	SUPPLIER	0
#define	GROUP		1
#define	ITEM_NO		2
#define	PROPOSED	3

#define	MANUFACTURED_ITEM 	(inmr_rec.source [0] == 'M')

#define	MAX_MONTHS	36

#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<pr_format3.h>

#define	CO_INV	 		(envCoClose [2] == '1')

#include	"schema"

#define	INVALID_WO	(pcwo_rec.order_status [0] == 'Z' || \
					 pcwo_rec.order_status [0] == 'D')

struct bmmsRecord	bmms_rec;
struct ccmrRecord	ccmr_rec;
struct ccmrRecord	ccmr2_rec;
struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct esmrRecord	esmr_rec;
struct ffprRecord	ffpr_rec;
struct ffwkRecord	ffwk_rec;
struct inccRecord	incc_rec;
struct ineiRecord	inei_rec;
struct inisRecord	inis_rec;
struct inisRecord	inis2_rec;
struct inisRecord	inis3_rec;
struct inldRecord	inld_rec;
struct inmrRecord	inmr_rec;
struct inumRecord	inum_rec;
struct itlnRecord	itln_rec;
struct pcwoRecord	pcwo_rec;
struct pocrRecord	pocr_rec;
struct podtRecord	podt_rec;
struct pocfRecord	pocf_rec;
struct pohrRecord	pohr_rec;
struct polnRecord	poln_rec;
struct rghrRecord	rghr_rec;
struct sumrRecord	sumr_rec;
struct sumrRecord	sumr2_rec;
struct ffdmRecord	ffdm_rec;


float	*ffwk_cons	=	&ffwk_rec.cons_1;

char	*ccmr2	= "ccmr2",
		*inis2	= "inis2",
		*inis3	= "inis3",
		*sumr2	= "sumr2";

	struct	{
		long	hhccHash;
	} whouse [MAX_CCMR];

	float	calcDemand			= 0.00,
			calcSafety			= 0.00,
			calcAvailable		= 0.00,
			calcOnOrder 		= 0.00,
			reviewPeriod		= 0.00,
			defaultReview		= 0.00;
	float	demand_value [MAX_MONTHS];

	double	costingFactor 		= 0.00,
  			intoStoreCost		= 0.00,
			alternateIntoStore	= 0.00;
		
	int		envFFInpZero 	= TRUE,
			byCompany 		= FALSE,
			byBranch 		= FALSE,
			byWarehouse 	= FALSE,
			printerNumber 	= 1,
			outputDevice	= PRINTER,
			sortingKey		= SUPPLIER,
			printAllItems	= TRUE,
			reorderFlag		= TRUE,
			summaryReport	= FALSE,
			proposedReport	= TRUE;

	char	envSupOrdRound 		[2],
			envLrpShowDemand	[2],
			upperLimit 			[13],
			lowerLimit 			[13],
			fileName 			[15],
			envCoClose 			[6];

	long	hhccHash	=	0L,
			startDate	=	0L;


	FILE	*fin,
			*fout;

	char	*envSkIvalClass,
			*result;

	static	struct	date_rec 
	{
		long	StartDate;
		long	EndDate;
		float	QtySold;
	} store_dates [37];

#include <RealCommit.h>

/*============================
| Local Function Prototypes. |
============================*/
double 	CalcIntoStore 			(void);
double 	FreightDefault 			(void);
float 	CalculateValues 		(void);
float 	FindReviewPeriod 		(void);
float 	GetLeadTime 			(long, long);
float 	GetOnOrder 				(long, long);
float 	TrigMax 				(float, float, float);
int 	CheckDates 				(void);
int 	CheckInmr 				(void);
int 	FindBestSupplier 		(long, int);
int 	FindBmms 				(long);
int 	FindInei 				(long);
int 	FindInis 				(long, long);
int 	FindRghr 				(long);
int 	ValidateWarehouse 		(long);
int 	check_page 				(void);
static 	float RoundMultiple 	(float, char *, float, float);
void 	AddEntry 				(float, float, float);
void 	CalcDates 				(long);
void 	CalcDays 				(long);
void 	CloseDB 				(void);
void 	FindFfwk 				(void);
void 	FindGroup 				(float);
void 	FindPartNumber 			(float);
void 	FindSupplier 			(float);
void 	HeadingOutput 			(void);
void 	LoadConsumption 		(void);
void 	LoadHhcc 				(void);
void 	LoadMonthsHistory 		(long, long, long, char *);
void 	LoadDaysHistory			(long, long, long, long, char *);
void 	OpenDB 					(void);
void 	PrintEntry 				(float);
void 	Process 				(float);
void 	ReadMisc 				(void);
void 	shutdown_prog 			(void);

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int	    argc,
 char*      argv [])
{
	char	*sptr;

	byCompany 	= FALSE;
	byBranch 	= FALSE;
	byWarehouse = FALSE;

	if (argc != 3 && argc != 11)
	{
		print_at (0,0,"Usage : %s <printerNumber> <review pd> <output dev, S (creen), P(rinter), D(atabase)> <fileName> <start group> <end group> <sort, S(upplier Group), I(tem Number)> <all items> <no reorder items> <S(ummary) D(etailed)",argv [0]);
		print_at (3,0,"OR    : %s <printerNumber> <fileName> ",argv [0]);
		return (EXIT_FAILURE);
	}

	proposedReport = (argc == 3);

	sptr = chk_env ("CO_CLOSE");
	if (sptr == (char *)0)
		sprintf (envCoClose,"%-5.5s","11111");
	else
		sprintf (envCoClose,"%-5.5s",sptr);

	sptr = strrchr (argv [0], '/');
	if (sptr == (char *) 0)
		sptr = argv [0];
	else
		sptr++;
	if (!strncmp (sptr, "lrp_creor",8))
		byCompany = TRUE;

	if (!strncmp (sptr, "lrp_breor",8))
		byBranch = TRUE;

	if (!strncmp (sptr, "lrp_reord",8))
		byWarehouse = TRUE;

	sptr = chk_env ("LRP_INP_ZERO");
	if (sptr == (char *) 0)
		envFFInpZero = TRUE;
	else
		envFFInpZero = atoi (sptr);

	sptr = chk_env ("LRP_COST_PVAR");
	costingFactor = (sptr == (char *) 0) ? 1.00 : atof (sptr) / 100 + 1.00;

	sptr = chk_env ("SUP_ORD_ROUND");
	if (sptr == (char *) 0)
		sprintf (envSupOrdRound, "B");
	else
	{
		switch (*sptr)
		{
		case	'U':
		case	'u':
			sprintf (envSupOrdRound, "U");
			break;

		case	'D':
		case	'd':
			sprintf (envSupOrdRound, "D");
			break;

		default:
			sprintf (envSupOrdRound, "B");
			break;
		}
	}

	sptr = chk_env ("LRP_SHOW_DEMAND");
	if (sptr == (char *) 0)
		sprintf (envLrpShowDemand, "M");
	else
	{
		switch (*sptr)
		{
		case	'D':
		case	'd':
			sprintf (envLrpShowDemand, "D");
			break;

		case	'W':
		case	'w':
			sprintf (envLrpShowDemand, "W");
			break;

		default:
			sprintf (envLrpShowDemand, "M");
			break;
		}
	}

	sptr = chk_env ("LRP_DFLT_REVIEW");
	if (sptr == (char *) 0)
		defaultReview = 4;
	else
		defaultReview = atof (sptr);

	sptr = chk_env ("SK_IVAL_CLASS");
	envSkIvalClass = (sptr == (char *)0) ? "ZKPN" : strdup (sptr);
	upshift (envSkIvalClass); 

	/*-------------------
	| Printer Number	|
	-------------------*/
	printerNumber = atoi (argv [1]);

	if (!proposedReport)
	{
		/*-----------------------------------
		| Number of weeks until next review	|
		-----------------------------------*/
		reviewPeriod = (float) atof (argv [2]);
		switch (argv [3] [0])
		{
		case	'P':
		case	'p':
			outputDevice = PRINTER;
			break;

		case	'D':
		case	'd':
			outputDevice = DATABASE;
			break;

		default:
			print_at (0,0,"Usage : %s <printerNumber> <review pd> <output dev, S(creen), P(rinter), D(atabase)> <fileName> <start group> <end group> <sort, S(upplier Group), I(tem Number)> <all items> <no reorder items> <S(ummary) D(etailed)");
			return (EXIT_FAILURE);
		}
		sprintf (fileName,"%-14.14s",argv [4]);
		sprintf (lowerLimit,"%-12.12s",argv [5]);
		sprintf (upperLimit,"%-12.12s",argv [6]);
	
		switch (argv [7] [0])
		{
		case	'S':
		case	's':
			sortingKey = SUPPLIER;
			break;

		case	'G':
		case	'g':
			sortingKey = GROUP;
			break;

		case	'I':
		case	'i':
			sortingKey = ITEM_NO;
			break;

		default:
			print_at (0,0,"Usage : %s <printerNumber> <review pd> <output dev, S(creen), P(rinter), D(atabase)> <fileName> <start group> <end group> <sort, S(upplier Group), I(tem Number)> <all items> <no reorder items> <S(ummary) D(etailed)");
			return (EXIT_FAILURE);
		}

		printAllItems	= (argv [8][0] == 'Y'  || argv [8][0] == 'y');
		reorderFlag 	= (argv [9][0] == 'Y'  || argv [9][0] == 'Y');
		summaryReport 	= (argv [10][0] == 'S' || argv [10][0] == 's');
	}
	else
	{
		sprintf (fileName,"%-14.14s",argv [2]);
		outputDevice 	= PRINTER;
		sortingKey 		= PROPOSED;
		printAllItems 	= TRUE;
		reorderFlag 	= TRUE;
		summaryReport	= FALSE;
		reviewPeriod 	= 0.00;
		byCompany 		= FALSE;
		byBranch 		= FALSE;
		byWarehouse 	= FALSE;
	}
	init_scr ();

	OpenDB ();

	ReadMisc ();

	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no,comm_rec.est_no);
	strcpy (ccmr_rec.cc_no, comm_rec.cc_no);
	cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
	if (cc)
		sys_err ("Error in ccmr During (DBFIND)", cc, PNAME);

	hhccHash = ccmr_rec.hhcc_hash;

	if (proposedReport)
	{
		ffwk_rec.hhcc_hash = hhccHash;
		sprintf (ffwk_rec.filename, "%-14.14s", fileName);
		sprintf (ffwk_rec.sort, "%-34.34s", " ");
		cc = find_rec (ffwk, &ffwk_rec, GTEQ, "r");
		if (cc || strcmp (ffwk_rec.filename, fileName))
		{
			errmess (ML ("Cannot locate file or Wrong warehouse."));
			sleep (sleepTime);
			clear_mess ();
			shutdown_prog ();
            return (EXIT_FAILURE);
		}

		switch (ffwk_rec.source [0])
		{
		case	'W':
			byWarehouse = TRUE;
			break;

		case	'B':
			byBranch = TRUE;
			break;

		default:
			byCompany = TRUE;
			break;
		}
	}

	if (byCompany)
    {
		if (CheckDates () == 1)
            return (EXIT_FAILURE);
    }

	if (outputDevice == PRINTER)
	{
		if (!summaryReport)
		{
			if (proposedReport)
			{
				if (! (fin = pr_open ("lrp_propose.p")))
					file_err (errno, "lrp_propose.p", "PR_OPEN");
			}
			else
			{
				if (! (fin = pr_open ("lrp_reorder.p")))
					file_err (errno, "lrp_reorder.p", "PR_OPEN");
			}
		}

		if ((fout = popen ("pformat","w")) == NULL)
			file_err (errno, "pformat", "POPEN");
			
		HeadingOutput ();
	}
	
	Process (reviewPeriod);

	if (outputDevice == PRINTER)
	{
		fprintf (fout, ".EOF\n");
		pclose (fout);
	}

	/*-------------------------------------------------------------------
	| Print Report Detailing Items with no BOM or Route Record       	|
	-------------------------------------------------------------------*/
	if (!proposedReport && outputDevice == PRINTER)
	{
		sprintf 
		(
			err_str, 
			"lrp_any_bmms %s %s %s", 
			argv [1],	/* printerNumber	*/
			argv [5],	/* start_group		*/
			argv [6]	/* end_group		*/
		);
		sys_exec (err_str);
	}

	/*-------------------------------------------------------------------
	| Print Report Detailing Items with no inis record in current group	|
	-------------------------------------------------------------------*/
	if (!proposedReport && outputDevice == PRINTER)
	{
		sprintf 
		(
			err_str, 
			"lrp_any_inis %s %s %s", 
			argv [1],	/* printerNumber	*/
			argv [5],	/* start_group		*/
			argv [6]	/* end_group		*/
		);
		sys_exec (err_str);
	}
	CloseDB (); 
	FinishProgram ();
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*========================
| Program exit sequence. |
========================*/
void
shutdown_prog (void)
{
	if (proposedReport || outputDevice != PRINTER)
    	CloseDB (); 
	
	FinishProgram ();
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (void)
{
	abc_dbopen ("data");

    abc_alias (inis2, inis);
	abc_alias (inis3, inis);
	abc_alias (ccmr2, ccmr);
	abc_alias (sumr2, sumr);

	switch (sortingKey)
	{
	case	SUPPLIER:
		open_rec (inis,  inis_list, INIS_NO_FIELDS, "inis_id_no");
		open_rec (inmr,  inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
		open_rec (inis2, inis_list, INIS_NO_FIELDS, "inis_id_no3");
		open_rec (sumr,  sumr_list, SUMR_NO_FIELDS, "sumr_id_no3");
		break;

	case	GROUP:
		open_rec (inis, inis_list, INIS_NO_FIELDS, "inis_id_no2");
		open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no_3");
		open_rec (sumr, sumr_list, SUMR_NO_FIELDS, "sumr_hhsu_hash");
		break;

	case	ITEM_NO:
		open_rec (inis, inis_list, INIS_NO_FIELDS, "inis_id_no2");
		open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
		open_rec (sumr, sumr_list, SUMR_NO_FIELDS, "sumr_hhsu_hash");
		break;

	case	PROPOSED:
		open_rec (inis, inis_list, INIS_NO_FIELDS, "inis_id_no");
		open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
		open_rec (sumr,sumr_list,SUMR_NO_FIELDS,"sumr_hhsu_hash");
		break;
	}
	open_rec (bmms,  bmms_list, BMMS_NO_FIELDS, "bmms_id_no");
	open_rec (ccmr,  ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	open_rec (ccmr2, ccmr_list, CCMR_NO_FIELDS, "ccmr_hhcc_hash");
	open_rec (esmr,  esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (ffdm,  ffdm_list, FFDM_NO_FIELDS, "ffdm_id_no2");
	open_rec (ffpr,  ffpr_list, FFPR_NO_FIELDS, "ffpr_id_no");
	open_rec (ffwk,  ffwk_list, FFWK_NO_FIELDS, 
							(proposedReport) ? "ffwk_sort_id" : "ffwk_id_no");
	open_rec (incc,  incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (inei,  inei_list, INEI_NO_FIELDS, "inei_id_no");
	open_rec (inis3, inis_list, INIS_NO_FIELDS, "inis_id_no2");
	open_rec (inld,  inld_list, INLD_NO_FIELDS, "inld_id_no");
	open_rec (inum,  inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	open_rec (itln,  itln_list, ITLN_NO_FIELDS, "itln_id_no2");
	open_rec (pcwo,  pcwo_list, PCWO_NO_FIELDS, "pcwo_id_date");
	open_rec (pocf,  pocf_list, POCF_NO_FIELDS, "pocf_id_no");
	open_rec (pocr,  pocr_list, POCR_NO_FIELDS, "pocr_id_no");
	open_rec (podt,  podt_list, PODT_NO_FIELDS, "podt_id_no");
	open_rec (pohr,  pohr_list, POHR_NO_FIELDS, "pohr_hhpo_hash");
	open_rec (poln,  poln_list, POLN_NO_FIELDS, "poln_id_date");
	open_rec (rghr,  rghr_list, RGHR_NO_FIELDS, "rghr_id_no");
	open_rec (soic,  soic_list, soic_no_fields, "soic_id_no2");
	open_rec (sumr2, sumr_list, SUMR_NO_FIELDS, "sumr_hhsu_hash");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (inis);
	abc_fclose (inmr);
	abc_fclose (sumr);
	abc_fclose (bmms);
	abc_fclose (ccmr);
	abc_fclose (ccmr2);
	abc_fclose (esmr);
	abc_fclose (ffdm);
	abc_fclose (ffpr);
	abc_fclose (ffwk);
	abc_fclose (incc);
	abc_fclose (inei);
	abc_fclose (inis3);
	abc_fclose (inld);
	abc_fclose (inum);
	abc_fclose (itln);
	abc_fclose (pcwo);
	abc_fclose (pocf);
	abc_fclose (pocr);
	abc_fclose (podt);
	abc_fclose (pohr);
	abc_fclose (poln);
	abc_fclose (rghr);
	abc_fclose (soic);
	abc_fclose (sumr2);
	if (sortingKey == SUPPLIER)
		abc_fclose (inis2);

	abc_dbclose ("data");
}

/*============================================
| Get common info from commom database file. |
============================================*/
void
ReadMisc (void)
{

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (comr,  comr_list, COMR_NO_FIELDS, "comr_co_no");

	strcpy (comr_rec.co_no, comm_rec.co_no);
	cc = find_rec (comr, &comr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "comr", "DBFIND");

	abc_fclose (comr);
}

/*===================================
| Start Out Put To Standard Print . |
===================================*/
void
HeadingOutput (void)
{
	fprintf (fout, ".START%s<%s>\n",DateToString (TodaysDate ()), PNAME);
	fprintf (fout, ".LP%d\n",printerNumber);
	if (summaryReport)
		fprintf (fout, ".15\n");
	else
		fprintf (fout, ".%d\n", (byCompany) ? 12 : 14);
	fprintf (fout, ".14\n");
	fprintf (fout, ".PI16\n");
	fprintf (fout, ".L160\n");
	fprintf (fout, ".B1\n");
	if (byCompany)
	{
	    fprintf (fout, ".E%s REPORT BY COMPANY\n",
		 (proposedReport) ? "LRP - PROPOSED PURCHASE ORDERS AND WORKS ORDERS" 
				   : "LRP - SUGGESTED REORDER");
	}
	if (byBranch)
	{
	    fprintf (fout,".E%s REPORT BY BRANCH\n",
		 (proposedReport) ? "LRP - PROPOSED PURCHASE ORDERS AND WORKS ORDERS" 
				   : "LRP - SUGGESTED REORDER");
	}
	if (byWarehouse)
	{
	    fprintf (fout,".E%s REPORT BY WAREHOUSE\n",
		 (proposedReport) ? "LRP - PROPOSED PURCHASE ORDERS AND WORKS ORDERS" 
				   : "LRP - SUGGESTED REORDER");
	}
	fprintf (fout, ".B1\n");
	fprintf (fout, ".E%s\n", clip (comm_rec.co_name));
	fprintf (fout, ".B1\n");
	if (!byCompany)
	{
		if (byBranch)
			fprintf (fout, ".E%s\n", clip (comm_rec.est_name));
		else
			fprintf (fout, ".E%s-%s\n", clip (comm_rec.est_name),
						clip (comm_rec.cc_name));
	
		fprintf (fout, ".B1\n");
	}
	fprintf (fout, ".EAS AT %-24.24s\n",SystemTime ());

	if (summaryReport)
	{
		fprintf (fout, ".R==========================");
		fprintf (fout, "===========");
		fprintf (fout, "=====================");
		fprintf (fout, "====================");
		fprintf (fout, "====================");
		fprintf (fout, "====================");
		fprintf (fout, "====================");
		fprintf (fout, "=============\n");

		fprintf (fout, "==========================");
		fprintf (fout, "===========");
		fprintf (fout, "=====================");
		fprintf (fout, "====================");
		fprintf (fout, "====================");
		fprintf (fout, "====================");
		fprintf (fout, "====================");
		fprintf (fout, "=============\n");


		fprintf (fout, "|   ITEM NUMBER    | UOM. ");
		fprintf (fout, "|   WEEKS  ");
		fprintf (fout, "|    SUPPLIER        ");
		fprintf (fout, "|     AVAILABLE     ");
		fprintf (fout, "|   STOCK ON ORDER  ");
		fprintf (fout, "|   TOTAL COVER     ");
		fprintf (fout, "|  REQUIRED  COVER  ");
		fprintf (fout, "|RECOMMENDED|\n");

		fprintf (fout, "|                  |      ");
		fprintf (fout, "|  DEMAND  ");
		fprintf (fout, "| NUMBER ");
		fprintf (fout, "|  ACRONYM  ");
		fprintf (fout, "|  QTY.    | WEEKS  ");
		fprintf (fout, "|  QTY.    | WEEKS  ");
		fprintf (fout, "|  QTY.    | WEEKS  ");
		fprintf (fout, "|  QTY.    | WEEKS  ");
		fprintf (fout, "| QUANTITY  |\n");

		fprintf (fout, "|------------------|------");
		fprintf (fout, "|----------");
		fprintf (fout, "|--------");
		fprintf (fout, "|-----------");
		fprintf (fout, "|----------|--------");
		fprintf (fout, "|----------|--------");
		fprintf (fout, "|----------|--------");
		fprintf (fout, "|----------|--------");
		fprintf (fout, "|-----------|\n");
	}
	else
	{
		pr_format (fin, fout, "RULER", 0, 0);
		pr_format (fin, fout, "COL_HEAD0", 0, 0);
		pr_format (fin, fout, "COL_HEAD1", 0, 0);
		pr_format (fin, fout, "COL_HEAD2", 0, 0);
	}
	fflush (fout);
}

int
check_page (void)
{
	return (EXIT_SUCCESS);
}

void
Process (
 float      reviewPeriod)
{
	int		first = 1;

	LoadHhcc ();

	if (outputDevice == DATABASE)
	{
		abc_selfield (ffwk, "ffwk_id_no");
		ffwk_rec.hhcc_hash = hhccHash;
		strcpy (ffwk_rec.filename,fileName);
	
		cc = find_rec (ffwk,&ffwk_rec,GTEQ,"u");
		/*---------------------------
		| Delete Existing Records	|
		---------------------------*/
		while (!cc && ffwk_rec.hhcc_hash == hhccHash && 
	                !strcmp (fileName,ffwk_rec.filename))
		{
			if (first)
				dsp_screen ("Processing : Clearing Work File.", comm_rec.co_no, comm_rec.co_name);

			first = 0;

			dsp_process ("Clear ",ffwk_rec.sort);

			abc_delete (ffwk);

			cc = find_rec (ffwk,&ffwk_rec,GTEQ,"u");
		}
		abc_unlock (ffwk);
	}

	if (proposedReport)
		dsp_screen ("Processing : Proposed Reorder Report.", 
				comm_rec.co_no, comm_rec.co_name);
	else
		dsp_screen ("Processing : Suggested Reorder Report.", 
				comm_rec.co_no, comm_rec.co_name);

	if (proposedReport)
	{
		abc_selfield (ffwk, "ffwk_sort_id");
		FindFfwk ();
		return;
	}

	switch (sortingKey)
	{
	case	SUPPLIER:
		FindSupplier (reviewPeriod);
		break;
		
	case	GROUP:
		FindGroup (reviewPeriod);
		break;
		
	case	ITEM_NO:
		FindPartNumber (reviewPeriod);
		break;
	}

}

/*===============================================
| Load hhcc_hash table with valid warehouses	|
===============================================*/
void
LoadHhcc (void)
{
	int		indx;

	for (indx = 0;indx < MAX_CCMR;indx++)
		whouse [indx].hhccHash = 0L;

	indx = 0;

	strcpy (ccmr_rec.co_no,comm_rec.co_no);
	strcpy (ccmr_rec.est_no, (byCompany) ? "  " : comm_rec.est_no);
	strcpy (ccmr_rec.cc_no, (byWarehouse) ? comm_rec.cc_no : "  ");
	cc = find_rec (ccmr,&ccmr_rec,GTEQ,"r");
	while (!cc && indx < MAX_CCMR && 
		   !strcmp (ccmr_rec.co_no,comm_rec.co_no))
	{
		if (ccmr_rec.lrp_ok [0] == 'N')
		{
			cc = find_rec (ccmr,&ccmr_rec, NEXT,"r");
			continue;
		}
		if (!byCompany)
		{
			if (strcmp (ccmr_rec.est_no,comm_rec.est_no))
			{
				cc = find_rec (ccmr,&ccmr_rec,NEXT,"r");
				continue;
			}
		}
		if (byWarehouse)
		{
			if (strcmp (ccmr_rec.cc_no,comm_rec.cc_no))
			{
				cc = find_rec (ccmr,&ccmr_rec,NEXT,"r");
				continue;
			}
		}
		whouse [indx++].hhccHash = ccmr_rec.hhcc_hash;
		cc = find_rec (ccmr,&ccmr_rec,NEXT,"r");
	}
}

void
FindFfwk (void)
{
	ffwk_rec.hhcc_hash = hhccHash;
	strcpy (ffwk_rec.filename,fileName);
	sprintf (ffwk_rec.sort,"%-34.34s"," ");
	cc = find_rec (ffwk,&ffwk_rec,GTEQ,"r");
	
	while (!cc && ffwk_rec.hhcc_hash == hhccHash && 
		      !strcmp (fileName,ffwk_rec.filename))
	{
		/*---------------------
		| Find Branch record. |
		---------------------*/
		cc = FindInei (ffwk_rec.hhbr_hash);
		if (cc)
		{
			cc = find_rec (ffwk, &ffwk_rec, NEXT,"r");
			continue;
		}
		/*--------------------------
		| Find Item master record. |
		--------------------------*/
		inmr_rec.hhbr_hash	=	ffwk_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (ffwk, &ffwk_rec, NEXT,"r");
			continue;
		}
		
		if (!MANUFACTURED_ITEM)
		{
			/*-----------------------
			| Find Supplier record. |
			-----------------------*/
			sumr_rec.hhsu_hash	=	ffwk_rec.hhsu_hash;
			cc = find_rec (sumr, &sumr_rec, COMPARISON, "r");
			if (cc)
			{
				cc = find_rec (ffwk, &ffwk_rec, NEXT,"r");
				continue;
			}
		}

		/*---------------------------------------------------------
		| For manufactured product, check it has a bom and route. |
		---------------------------------------------------------*/
		if (MANUFACTURED_ITEM)
		{
			cc = FindBmms (ffwk_rec.hhbr_hash);
			if (!cc)
			{
				cc = FindRghr(ffwk_rec.hhbr_hash);
			}
		}
		else
		{
			cc = FindInis
				(
					ffwk_rec.hhbr_hash,
					ffwk_rec.hhsu_hash
				);
		}
		if (!cc)
			PrintEntry (ffwk_rec.review_pd);

		cc = find_rec (ffwk,&ffwk_rec,NEXT,"r");
	}
}

int
FindInis (
 long       hhbrHash,
 long       hhsuHash)
{
	inis_rec.hhbr_hash	=	hhbrHash;
	inis_rec.hhsu_hash	=	hhsuHash;
	strcpy (inis_rec.co_no, "  ");
	strcpy (inis_rec.br_no, "  ");
	strcpy (inis_rec.wh_no, "  ");
	cc = find_rec (inis, &inis_rec, GTEQ, "r");
	if (!cc && inis_rec.hhbr_hash == hhbrHash && 
			   inis_rec.hhsu_hash == hhsuHash)
		return (EXIT_SUCCESS);
	
	return (EXIT_FAILURE);
}

int
FindBmms (
 long		hhbr_hash)
{
	long bom;

	if (byCompany)
		bom = inmr_rec.dflt_bom;
	else
		bom = inei_rec.dflt_bom;

	strcpy (bmms_rec.co_no, comm_rec.co_no);
	bmms_rec.hhbr_hash = hhbr_hash;
	bmms_rec.alt_no = bom;
	bmms_rec.line_no = 0;

	cc = find_rec (bmms, &bmms_rec, GTEQ, "r");
	if (!cc && bmms_rec.hhbr_hash == hhbr_hash && 
			   bmms_rec.alt_no    == bom)
		return (EXIT_SUCCESS);

	return 1;
}

int
FindRghr (
 long		hhbr_hash)
{
	long alt_no;

	if (byCompany)
		alt_no = inmr_rec.dflt_rtg;
	else
		alt_no = inei_rec.dflt_rtg;

	strcpy (rghr_rec.co_no, comm_rec.co_no);
	strcpy (rghr_rec.br_no, comm_rec.est_no);
	rghr_rec.hhbr_hash = hhbr_hash;
	rghr_rec.alt_no = alt_no;

	cc = find_rec (rghr, &rghr_rec, GTEQ, "r");
	if (!cc && rghr_rec.hhbr_hash == hhbr_hash && 
			   rghr_rec.alt_no    == alt_no)
		return 0;

	return 1;
}

void
FindSupplier (
 float      reviewPeriod)
{
	strcpy (sumr_rec.co_no,comm_rec.co_no);
	strcpy (sumr_rec.crd_no,"      ");

	/*---------------------------------------
	| Go thru All valid Sumr records	|
	---------------------------------------*/
	cc = find_rec (sumr, &sumr_rec, GTEQ, "r");
	while (!cc && !strcmp (sumr_rec.co_no,comm_rec.co_no))
	{
		/*-------------------
		| if wrong est_no 	|
		-------------------*/
		if (strcmp (sumr_rec.est_no,comm_rec.est_no) && 
		     strcmp (sumr_rec.est_no," 0"))
		{
			cc = find_rec (sumr, &sumr_rec, NEXT, "r");
			continue;
		}

		/*-------------------------------------------
		| Go thru all parts supplied by supplier	|
		-------------------------------------------*/
		inis2_rec.hhsu_hash = sumr_rec.hhsu_hash;
		inis2_rec.hhbr_hash = 0L;
		strcpy (inis2_rec.co_no, "  ");
		strcpy (inis2_rec.br_no, "  ");
		strcpy (inis2_rec.wh_no, "  ");
		cc = find_rec (inis2, &inis2_rec, GTEQ, "r");
		while (!cc && inis2_rec.hhsu_hash == sumr_rec.hhsu_hash)
		{
			/*---------------------------
			| Get inmr record for inis	|
			---------------------------*/
			inmr_rec.hhbr_hash	=	inis2_rec.hhbr_hash;
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
			if (cc)
			{
				cc = find_rec (inis2, &inis2_rec, NEXT, "r");
				continue;
			}
			inis_rec.hhsu_hash = inis2_rec.hhsu_hash;
			abc_selfield(inis, "inis_id_no2");
			cc = FindBestSupplier (inis2_rec.hhbr_hash, MAX_SUPP);
			if (cc || (!cc 
					&& memcmp(&inis2_rec, &inis_rec, sizeof inis_rec)))
			{
				cc = find_rec (inis2, &inis2_rec, NEXT, "r");
				continue;
			}

			/*----------------------------------------
			| Item Is in Subrange ... and found inei |
			----------------------------------------*/
			abc_selfield(inis, "inis_id_no");
			if (!CheckInmr () && !FindInei (inis2_rec.hhbr_hash))
			{
				memcpy 
				(
					(char *) &inis_rec, 
					(char *) &inis2_rec,
					sizeof (struct inisRecord)
				);
				PrintEntry (reviewPeriod);
			}
			cc = find_rec (inis2, &inis2_rec, NEXT, "r");
		}
		cc = find_rec (sumr, &sumr_rec, NEXT, "r");
	}
}

void
FindGroup (
 float      reviewPeriod)
{
	strcpy (inmr_rec.co_no,comm_rec.co_no);
	sprintf (inmr_rec.inmr_class,"%-1.1s",lowerLimit);
	sprintf (inmr_rec.category,"%-11.11s",lowerLimit + 1);
	sprintf (inmr_rec.item_no,"%-16.16s"," ");
	cc = find_rec (inmr,&inmr_rec,GTEQ,"r");

	/*-----------------------
	| For all valid inmr's	|
	-----------------------*/
	while (!cc)
	{
	    cc = CheckInmr ();
	    if (cc >= 2)
			break;

	    if (cc)
	    {
			cc = find_rec (inmr, &inmr_rec, NEXT, "r");
			continue;
	    }
		if (!FindBestSupplier (inmr_rec.hhbr_hash, MAX_SUPP))
	    {
			if (!MANUFACTURED_ITEM)
			{
				sumr_rec.hhsu_hash	=	inis_rec.hhsu_hash;
				cc = find_rec (sumr, &sumr_rec, COMPARISON, "r");
			}
			if (!cc)
			{
				if (!FindInei (inmr_rec.hhbr_hash))
		    		PrintEntry (reviewPeriod);
			}
				
	    }
	    cc = find_rec (inmr, &inmr_rec, NEXT, "r");
	}
}

int
FindBestSupplier (
 long       hhbrHash,
 int        NoLevels)
{
	int		i;

	/*----------------------------------------------------------
	| Manufactured product - look for BOM/Route, not supplier. |
	----------------------------------------------------------*/
	if (MANUFACTURED_ITEM)
	{
		cc = FindBmms (inmr_rec.hhbr_hash);
		if (!cc)
		{
			cc = FindRghr(inmr_rec.hhbr_hash);
		}
	}
	else
	{
		for (i = 0; i < NoLevels; i++)
		{
			inis_rec.hhbr_hash = hhbrHash;
			sprintf (inis_rec.sup_priority, "W%1d", i);
			strcpy (inis_rec.co_no, comm_rec.co_no);
			strcpy (inis_rec.br_no, comm_rec.est_no);
			strcpy (inis_rec.wh_no, comm_rec.cc_no);
			cc = find_rec ("inis", &inis_rec, COMPARISON, "r");
			if (cc)
			{
				inis_rec.hhbr_hash = hhbrHash;
				sprintf (inis_rec.sup_priority, "B%1d", i);
				strcpy (inis_rec.co_no, comm_rec.co_no);
				strcpy (inis_rec.br_no, comm_rec.est_no);
				strcpy (inis_rec.wh_no, "  ");
				cc = find_rec ("inis", &inis_rec, COMPARISON, "r");
				if (cc)
				{
					inis_rec.hhbr_hash = hhbrHash;
					sprintf (inis_rec.sup_priority, "C%1d", i);
					strcpy (inis_rec.co_no, comm_rec.co_no);
					strcpy (inis_rec.br_no, "  ");
					strcpy (inis_rec.wh_no, "  ");
					cc = find_rec ("inis", &inis_rec, COMPARISON, "r");
				}
			}
			if (!cc)
				return (EXIT_SUCCESS);
		}
	}
	return (cc);
}

void
FindPartNumber (
	float      reviewPeriod)
{
	strcpy (inmr_rec.co_no,comm_rec.co_no);
	sprintf (inmr_rec.item_no,"%-16.16s"," ");
	cc = find_rec (inmr,&inmr_rec,GTEQ,"r");

	/*---------------
	| All inmr's	|
	---------------*/
	while (!cc && !strcmp (inmr_rec.co_no ,comm_rec.co_no))
	{
	    cc = CheckInmr ();
	    if (cc >= 3)	/* Invalid company		*/
			break;

	    if (cc >= 1)	/* Invalid group or record	*/
	    {
			cc = find_rec (inmr, &inmr_rec, NEXT, "r");
			continue;
	    }
		if (!FindBestSupplier (inmr_rec.hhbr_hash, MAX_SUPP))
	    {
			if (!MANUFACTURED_ITEM)
			{
				sumr_rec.hhsu_hash	=	inis_rec.hhsu_hash;
				cc = find_rec (sumr, &sumr_rec, COMPARISON, "r");
			}
			if (!cc)
			{
				if (!FindInei (inmr_rec.hhbr_hash))
		    		PrintEntry (reviewPeriod);
			}
	    }
	    cc = find_rec (inmr,&inmr_rec,NEXT,"r");
	}
}

int
FindInei (
 long   hhbrHash)
{
	strcpy (inei_rec.est_no,inis_rec.br_no);
	inei_rec.hhbr_hash = hhbrHash;
	cc = find_rec (inei,&inei_rec,COMPARISON,"r");
	if (cc)
	{
		strcpy (inei_rec.est_no,comm_rec.est_no);
		inei_rec.hhbr_hash = hhbrHash;
		cc = find_rec (inei,&inei_rec,COMPARISON,"r");
	}
	return (cc);
}

/*==================================================
| Check the current inmr record and return one of: |
|	0	- Valid record                             |
|	1	- Invalid record                           |
|	2	- Outside valid group range                |
|	3	- Company invalid                          |
==================================================*/
int
CheckInmr (void)
{
	/*------------------------------------------
	| Exclude all recordes with invalid class. |
	------------------------------------------*/
	if ((result = strstr (envSkIvalClass, inmr_rec.inmr_class)))
		return (EXIT_FAILURE);

	/*-------------------------------
	| Ignore Discontinued Products. |
	-------------------------------*/
	if (inmr_rec.active_status [0] == 'D')
		return (EXIT_FAILURE);

	if (strcmp (inmr_rec.co_no, comm_rec.co_no))
		return (3);

	if (inmr_rec.inmr_class [0] < lowerLimit [0] || 
	    inmr_rec.inmr_class [0] > upperLimit [0])
		return (2);

	if (upperLimit [0] == inmr_rec.inmr_class [0] &&
	    strcmp (inmr_rec.category, upperLimit + 1) > 0)
		return (2);

	if (lowerLimit [0] == inmr_rec.inmr_class [0] &&
	    strcmp (inmr_rec.category, lowerLimit + 1) < 0)
		return (2);

	return (EXIT_SUCCESS);
}

/*===================
| Calculate Demand. |
===================*/
float
CalculateValues (void)
{
	int		indx;
	float	realCommitted = 0.00;
	float	minStockQty = 0.0;
	float	minStockWks	= 0.0;

	calcDemand   	= 0.00;
	calcSafety   	= 0.00;
	calcAvailable  	= 0.00;

	for (indx = 0;indx < MAX_CCMR && whouse [indx].hhccHash != 0L;indx++)
	{
		incc_rec.hhcc_hash = whouse [indx].hhccHash;
		incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
		cc = find_rec (incc,&incc_rec,COMPARISON,"r");
		if (!cc)
		{
			/*---------------------------------
			| Calculate Actual Qty Committed. |
			---------------------------------*/
			realCommitted = RealTimeCommitted (incc_rec.hhbr_hash,
												incc_rec.hhcc_hash);

			calcDemand   	+= 	incc_rec.wks_demand;
			calcSafety   	+= 	incc_rec.safety_stock;
			calcAvailable   += 	incc_rec.closing_stock -
			  	        		incc_rec.committed - 
								realCommitted - 
								incc_rec.backorder - 
								incc_rec.forward;
		}
	}
	if (byBranch)
	{
		calcSafety = inei_rec.safety_stock;
		minStockQty = inei_rec.min_stock;
	}
	if (byCompany)
	{
		calcSafety = inmr_rec.safety_stock;
		minStockQty = inmr_rec.min_quan;
	}

	if (proposedReport)
	{
		if (ffwk_rec.wks_demand != 0.0)
			minStockWks = minStockQty / ffwk_rec.wks_demand;
	}
	else
	{
		if (calcDemand != 0.0)
			minStockWks = minStockQty / calcDemand;
	}

	return (minStockWks);
}

int
ValidateWarehouse (
 long       hhccHash)
{
	int		indx;

	for (indx = 0;indx < MAX_CCMR && whouse [indx].hhccHash != 0L;indx++)
		if (hhccHash == whouse [indx].hhccHash)
			return (TRUE);

	return (FALSE);
}

/*===============================
| return maximum of a,b, or c	|
===============================*/
float
TrigMax (
 float      a,
 float      b,
 float      c)
{
	if (a > b)
		if (a > c)
			return (a);
		else
			return ((c > b) ? c : b);
	else
		if (b > c)
			return (b);
		else
			return ((c > a) ? c : a);
				
}

void
AddEntry (
	float  qty_recommended,
	float  reviewPeriod,
	float  wks_demand)
{
	ffwk_rec.hhcc_hash = hhccHash;
	ffwk_rec.hhbr_hash = inmr_rec.hhbr_hash;
	ffwk_rec.hhsu_hash = sumr_rec.hhsu_hash;
	ffwk_rec.hhpo_hash = 0L;
	strcpy (ffwk_rec.filename,fileName);
	if (sortingKey == SUPPLIER)
	{
		sprintf (ffwk_rec.sort,"%-6.6s%-16.16s%-12.12s",
				sumr_rec.crd_no,
				inmr_rec.item_no," ");
	}
	if (sortingKey == GROUP)
	{
		sprintf (ffwk_rec.sort,"%-1.1s%-11.11s%-16.16s%-6.6s",
				inmr_rec.inmr_class,
				inmr_rec.category,
				inmr_rec.item_no," ");
	}
	if (sortingKey == ITEM_NO)
	{
		sprintf (ffwk_rec.sort,"%-16.16s%-18.18s",
				inmr_rec.item_no," ");
	}
				
	strcpy (ffwk_rec.crd_no,sumr_rec.crd_no);
	ffwk_rec.wks_demand = wks_demand;
	ffwk_rec.review_pd 	= reviewPeriod;
	ffwk_rec.sugg_qty 	= qty_recommended;
	ffwk_rec.order_qty 	= 0.00;
	strcpy (ffwk_rec.source, "C");
	if (byBranch)
		strcpy (ffwk_rec.source, "B");
	if (byWarehouse)
		strcpy (ffwk_rec.source, "W");

	LoadConsumption ();

	ffwk_rec.alt_supp	=	0;

	memcpy 
	(
		(char *) &inis3_rec, 
		(char *) &inis_rec,
		sizeof (struct inisRecord)
	);
  	intoStoreCost = CalcIntoStore ();

	inis3_rec.hhbr_hash 	= inis_rec.hhbr_hash;
	strcpy (inis3_rec.sup_priority,"  ");
	strcpy (inis3_rec.co_no, "  ");
	strcpy (inis3_rec.br_no, "  ");
	strcpy (inis3_rec.wh_no, "  ");
	cc = find_rec (inis3, &inis3_rec, GTEQ,"r");
	while (!cc && inis3_rec.hhbr_hash == inis_rec.hhbr_hash)
	{
		if (inis3_rec.hhsu_hash	==	inis_rec.hhsu_hash)
		{
			cc = find_rec (inis, &inis3_rec, NEXT,"r");
			continue;
		}
	
		alternateIntoStore = CalcIntoStore ();
		if (alternateIntoStore * costingFactor < intoStoreCost &&
			alternateIntoStore * costingFactor > 0.00001 &&
			strcmp (inis3_rec.sup_priority,"1"))
		{
			ffwk_rec.alt_supp	=	1;
		}
		cc = find_rec (inis3, &inis3_rec, NEXT, "r");
	}

	strcpy (ffwk_rec.stat_flag, " ");
	cc = abc_add (ffwk,&ffwk_rec);
	if (cc)
		sys_err ("Error in ffwk During (DBADD)", cc, PNAME);
}

void
LoadConsumption (void)
{
	int		i, 
			j,
			indx;
	int		currentMonth,
			currentDay;

	long	lsystemDate;

	for (i = 0; i < 12; i++)
		ffwk_cons [i] = 0.00;

	startDate	=	MonthEnd (comm_rec.inv_date) + 1;
	DateToDMY (comm_rec.inv_date, NULL, &currentMonth, NULL);

	lsystemDate = TodaysDate ();
	currentDay = lsystemDate % 7;  

	indx = 0;
	while (indx <= MAX_CCMR && whouse [indx].hhccHash != 0L)
	{
		incc_rec.hhbr_hash = ffwk_rec.hhbr_hash;
		incc_rec.hhcc_hash = whouse [indx].hhccHash;
		indx++;
		cc = find_rec (incc, &incc_rec, EQUAL, "r");
		if (cc)
			continue;

		if (envLrpShowDemand [0] == 'D')
		{
			startDate	=	comm_rec.inv_date;
			LoadDaysHistory
			(
				incc_rec.hhbr_hash, 
				incc_rec.hhcc_hash, 
				startDate - MAX_MONTHS, 
				startDate, 
				"1"
			);
			printf ("\n\r [%ld][%ld]", incc_rec.hhbr_hash, incc_rec.hhcc_hash);
			for (i = 0; i < MAX_MONTHS; i++)
			{
				printf ("Values [%d][%f], ",i, demand_value [i]);
			}
			getchar();
			for (i = 0; i < 12 ; i++)
			{
				j = (i + currentDay) % 7;
				ffwk_cons [j] += demand_value [i + 24];
			}
		}
		else
		{
			LoadMonthsHistory
			(
				incc_rec.hhbr_hash, 
				incc_rec.hhcc_hash, 
				startDate, 
				"1"
			);
			for (i = 0; i < 12 ; i++)
			{
				j = (i + currentMonth) % 12;
				ffwk_cons [j] += demand_value [i + 24];
			}
		}
	}
}

/*===================================
| Calculate into_store (supplier)   |
===================================*/
double 
CalcIntoStore (void)
{
	double	fob_cost    = 0.00;
	double	cif_cost    = 0.00;
	double	contingency = 0.00;
	double	duty        = 0.00;
	float	duty_pc     = 0.00;

	sumr_rec.hhsu_hash	=	inis3_rec.hhsu_hash;
	cc = find_rec (sumr,&sumr_rec,COMPARISON,"r");
	if (cc)
		return (0.00);

	fob_cost = inis3_rec.fob_cost;

	strcpy (pocr_rec.co_no,comm_rec.co_no);
	strcpy (pocr_rec.code,sumr_rec.curr_code);
	cc = find_rec (pocr,&pocr_rec,COMPARISON,"r");
	if (cc || pocr_rec.ex1_factor == 0.00)
		pocr_rec.ex1_factor = 1.00;

	fob_cost /= pocr_rec.ex1_factor;

	strcpy (podt_rec.co_no,comm_rec.co_no);
	strcpy (podt_rec.code,inis3_rec.duty);
	cc = find_rec (podt,&podt_rec,COMPARISON,"r");
	if (!cc)
	{
		if (podt_rec.duty_type [0] == 'P')
		{
			duty_pc = podt_rec.im_duty;
			duty = DOLLARS (duty_pc) * fob_cost;
		}
		else
		{
			duty = podt_rec.im_duty;
			if (duty + fob_cost != 0.00)
			    duty_pc = duty / (duty + fob_cost);
		}
	}
	else
	{
		duty = 0.00;
		duty_pc = 0.00;
	}

	cif_cost = fob_cost + duty + FreightDefault ();

	contingency = DOLLARS (comr_rec.contingency);
	contingency *= cif_cost;
	
	return (cif_cost + contingency);
}

double
FreightDefault (void)
{
	double	frt_conv = 0.00;

	double	freight = 0.00;

	strcpy (pocf_rec.co_no,comm_rec.co_no);
	strcpy (pocf_rec.code,sumr_rec.ctry_code);
	cc = find_rec (pocf,&pocf_rec,COMPARISON,"r");
	if (cc)
		return (0.00);

	/*-------------------
	| Calculate Freight	|
	-------------------*/
	frt_conv = pocf_rec.freight_load;

	if (pocf_rec.load_type [0] == 'U')
		freight = frt_conv;

	if (pocf_rec.load_type [0] == 'P')
		freight = (inis3_rec.fob_cost * frt_conv) / 100;

	freight /= pocr_rec.ex1_factor;

	return (freight);
}

/*===============================================
| print an entry for a item_no / supplier	|
===============================================*/
void
PrintEntry (
 float      reviewPeriod)
{
	int		manufactItem	= TRUE;
	int		useMinStockWks	= FALSE;
	double	ex_rate			= 0.00,
			val_recommended	= 0.00,
			val_proposed	= 0.00;
	
	float	minStockWks		=	0.00;

	long	end_cover;
	float	TrigMax (float a, float b, float c),
			cover			= 0.00,
			on_order		= 0.00,
			qty_avail		= 0.00,
			qty_cover_reqd	= 0.00,
			qty_net_reqt	= 0.00,
			qty_on_order	= 0.00,
			qty_outstand	= 0.00,
			qty_recommended	= 0.00,
			qty_tot_cover	= 0.00,
			safety_stock	= 0.00,
			wk_review		= 0.00,
			wks_avail		= 0.00,
			wks_cover_reqd	= 0.00,
			wks_demand		= 0.00,
			wks_net_reqt	= 0.00,
			wks_on_order	= 0.00,
			wks_tot_cover	= 0.00,
			lead_days		= 0.00,
			lead_weeks		= 0.00;

	/*-------------------------------------------------------
	| Temporarily, disable print/generation of info where	|
	| the inis record references a warehouse rather than a	|
	| specific supplier. (ie: inis_hhsu_hash = 0)			|
	|														|
	| If the product is manufactured rather than purchased	|
	| there will be no inis so don't check it.				|
	-------------------------------------------------------*/
	if (!MANUFACTURED_ITEM && inis_rec.hhsu_hash == 0L)
		return;

	wk_review = FindReviewPeriod ();
	
	reviewPeriod += wk_review;
	
	if (proposedReport)
		reviewPeriod = ffwk_rec.review_pd;

	minStockWks =	CalculateValues ();

	wks_demand = (proposedReport) ? ffwk_rec.wks_demand : calcDemand;
	if (wks_demand <= 0.00)
		return;

	strcpy (pocr_rec.co_no,comm_rec.co_no);
	strcpy (pocr_rec.code,sumr_rec.curr_code);
	cc = find_rec (pocr,&pocr_rec,COMPARISON,"r");
	ex_rate = (cc || pocr_rec.ex1_factor == 0.00) ? 1.00 : 
		         pocr_rec.ex1_factor;

	safety_stock = calcSafety;

	/*-------------------------------------------------
	| Manufactured product - get lead time from incc. |
	-------------------------------------------------*/
	if (MANUFACTURED_ITEM)
	{
		strcpy(ccmr_rec.co_no, comm_rec.co_no);
		strcpy(ccmr_rec.est_no, comm_rec.est_no);
		strcpy(ccmr_rec.cc_no, comm_rec.cc_no);

		cc = find_rec (ccmr, &ccmr_rec, EQUAL, "r");
		if (cc)
			sys_err("error reading ccmr", cc, PNAME);

		incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
		incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
		cc = find_rec (incc, &incc_rec, EQUAL, "r");
		if (cc)
			sys_err("error reading incc", cc, PNAME);

		lead_days  = incc_rec.acc_mlt;
		lead_weeks = incc_rec.acc_mlt / 7;
	}
	/*--------------------------------------------------
	| Purchased product - get lead time from inis/inld |
	--------------------------------------------------*/
	else
	{
		if (inis_rec.lead_time == 0.00)
			inis_rec.lead_time = 
				GetLeadTime (inis_rec.hhis_hash, comm_rec.inv_date);

		lead_days  = inis_rec.lead_time;
		lead_weeks = inis_rec.lead_time / 7;
	}
	/*-------------------------------------
	| Use the greater of Minimum Stock    |
	| Weeks (minStockWks) or Safety Stock |
	-------------------------------------*/
	if (minStockWks > safety_stock)
	{
		cover = reviewPeriod + lead_weeks + minStockWks;
		useMinStockWks = TRUE;
	}
	else
		cover = reviewPeriod + lead_weeks + safety_stock;

	end_cover 		= (cover * 7) + comm_rec.inv_date;
	calcOnOrder 	= GetOnOrder (inmr_rec.hhbr_hash, (long) end_cover);

	qty_avail		= calcAvailable;
	qty_on_order	= calcOnOrder;
	qty_tot_cover	= qty_avail + calcOnOrder;

	qty_cover_reqd	= cover;
	qty_cover_reqd	*= wks_demand;

	qty_net_reqt	= qty_cover_reqd - qty_tot_cover;

	if (qty_net_reqt < 0.00)
		qty_net_reqt = 0.00;

	wks_avail		= qty_avail / wks_demand;
	wks_on_order	= qty_on_order / wks_demand;
	wks_tot_cover	= qty_tot_cover / wks_demand;
	wks_cover_reqd	= cover;
	wks_net_reqt	= qty_net_reqt / wks_demand;

	if (twodec (qty_net_reqt) == 0.00)
		qty_recommended = 0.00;
	else
	{
		qty_recommended	= 	TrigMax 
							(	
								qty_net_reqt,
								inis_rec.min_order,
								inis_rec.norm_order
							);

		if (MANUFACTURED_ITEM)
		{
			qty_recommended = 	RoundMultiple 
								(
									qty_recommended, 
									envSupOrdRound, 
									inei_rec.prd_multiple, 
									inei_rec.min_batch
								);
		}
		else
		{
			qty_recommended = 	RoundMultiple 
								(
									qty_recommended, 
									envSupOrdRound, 
									inis_rec.ord_multiple, 
									inis_rec.min_order
								);
		}
	}

	/*-------------------------------------------
	| Manufactured product - get cost from inei |
	-------------------------------------------*/
	if (MANUFACTURED_ITEM)
	{
		val_recommended = (double) (qty_recommended * inei_rec.std_cost);
		val_proposed = (double) (ffwk_rec.order_qty * inei_rec.std_cost);
	}
	/*-----------------------------------
	| Get cost for a purchased product. |
	-----------------------------------*/
	else
	{
		val_recommended	= (double) (qty_recommended * inis_rec.fob_cost);
		val_recommended	/= ex_rate;
		val_proposed = (double) (ffwk_rec.order_qty * inis_rec.fob_cost);
		val_proposed /= ex_rate;
	}
	if (!proposedReport && !printAllItems && qty_net_reqt == 0.00)
		return;

	if (inmr_rec.reorder [0] == 'N' && !reorderFlag)
		return;

	dsp_process ("Item No: ",inmr_rec.item_no);

	if (outputDevice == DATABASE)
	{
		if (qty_recommended != 0.00 || envFFInpZero)
			AddEntry (qty_recommended,reviewPeriod,wks_demand);
		return;
	}
	if (qty_recommended == 0.00 && !printAllItems)
		return;

	/*--------------------------------------------------------------------
	| Check if item is a manufactured item, if not read purchase orders. |
	--------------------------------------------------------------------*/
	if (strcmp (inmr_rec.source, "BP") &&
		strcmp (inmr_rec.source, "BM") &&
		strcmp (inmr_rec.source, "MC") &&
		strcmp (inmr_rec.source, "MP"))
			manufactItem	= FALSE;

	if (manufactItem	== FALSE)
	{
		poln_rec.hhbr_hash	= inmr_rec.hhbr_hash;
		poln_rec.due_date	= 0L;
		cc = find_rec (poln, &poln_rec, GTEQ, "r");
		while (!cc && poln_rec.hhbr_hash == inmr_rec.hhbr_hash)
		{
			on_order = poln_rec.qty_ord - poln_rec.qty_rec;
			if (ValidateWarehouse (poln_rec.hhcc_hash) && on_order > 0.0)
			{
				pohr_rec.hhpo_hash	=	poln_rec.hhpo_hash;
				cc = find_rec (pohr, &pohr_rec, EQUAL, "r");
				if (!cc && pohr_rec.drop_ship [0] != 'Y')
				{
					qty_outstand += on_order;
					break;
				}
			}
			cc = find_rec (poln, &poln_rec, NEXT, "r");
		}
		if (cc || poln_rec.hhbr_hash != inmr_rec.hhbr_hash)
		{
			pohr_rec.hhsu_hash = inis_rec.hhsu_hash;
			poln_rec.due_date = 0L;
			qty_outstand = (float) 0.00;
		}
		else
		{
			pohr_rec.hhpo_hash	=	poln_rec.hhpo_hash;
			cc = find_rec (pohr, &pohr_rec, EQUAL, "r");
		}
	}
	else
	{
		pcwo_rec.hhbr_hash	= inmr_rec.hhbr_hash;
		pcwo_rec.reqd_date	= 0L;
		cc = find_rec (pcwo, &pcwo_rec, GTEQ, "r");
		while (!cc && pcwo_rec.hhbr_hash == inmr_rec.hhbr_hash)
		{
			/*-----------------------------------------
			| Ignore closed and deleted works orders. |
			-----------------------------------------*/
			if (INVALID_WO)
			{
				cc = find_rec (pcwo, &pcwo_rec, NEXT, "r");
				continue;
			}
			on_order = pcwo_rec.prod_qty - 
				  	  (pcwo_rec.act_prod_qty + pcwo_rec.act_rej_qty);
			if (ValidateWarehouse (pcwo_rec.hhcc_hash) && on_order > 0.0)
			{
				qty_outstand += on_order;
				break;
			}
			cc = find_rec (pcwo, &pcwo_rec, NEXT, "r");
		}
		if (cc || pcwo_rec.hhbr_hash != inmr_rec.hhbr_hash)
		{
			pcwo_rec.reqd_date = 0L;
			qty_outstand = (float) 0.00;
		}
	}

	/*-------------------------------------------------------------------
	| for manufactured Products and Components, display "ManProd" or    |
	| "ManCom" rather than Supplier                                 	|
	|------------------------------------------------------------------*/
	if (!strcmp(inmr_rec.source, "MP"))
		strcpy(sumr_rec.crd_no, "ManPrd");
	else if (!strcmp(inmr_rec.source, "MC"))
		strcpy(sumr_rec.crd_no, "ManCom");

	if (summaryReport)
	{
		fprintf (fout, "| %-16.16s | %4.4s ",
								inmr_rec.item_no, inmr_rec.sale_unit);
		fprintf (fout, "|%9.2f ", 			wks_demand);
		fprintf (fout, "| %6.6s | %9.9s ",	sumr_rec.crd_no,sumr_rec.acronym);
		fprintf (fout, "|%10.1f|%8.2f",		qty_avail,		wks_avail);
		fprintf (fout, "|%10.1f|%8.2f",		qty_on_order,	wks_on_order);
		fprintf (fout, "|%10.1f|%8.2f",		qty_cover_reqd,	wks_cover_reqd);
		fprintf (fout, "|%10.1f|%8.2f",		qty_net_reqt,	wks_net_reqt);
		fprintf (fout, "|%11.1f|\n",	qty_recommended);
		return;
	}
	pr_format (fin, fout, "COL_HEAD3", 0, 0);
	pr_format (fin, fout, "ITEM_LIN1", 1, inmr_rec.item_no);
	pr_format (fin, fout, "ITEM_LIN1", 2, inmr_rec.sale_unit);
	pr_format (fin, fout, "ITEM_LIN1", 3, wks_demand);
	if (inis_rec.hhsu_hash == 0L)
	{
		sprintf (err_str, "Branch:   %2.2s", ccmr2_rec.est_no);
		pr_format (fin, fout, "ITEM_LIN1", 4, err_str);
	}
	else
	{
		sprintf (err_str, "Supp: %6.6s", sumr_rec.crd_no);
		pr_format (fin, fout, "ITEM_LIN1", 4, err_str);
	}
	pr_format (fin, fout, "ITEM_LIN1", 5, inis_rec.ord_multiple);
	if (manufactItem)
		pr_format (fin, fout, "ITEM_LIN1", 6, "W");
	else
	{
		pr_format (fin, fout, "ITEM_LIN1", 6,
	    (inis_rec.hhsu_hash == pohr_rec.hhsu_hash) ? " " : "*");
	}
	pr_format (fin, fout, "ITEM_LIN1", 7, (manufactItem) ? pcwo_rec.reqd_date
														 : poln_rec.due_date);
	pr_format (fin, fout, "ITEM_LIN1", 8, qty_outstand);
	pr_format (fin, fout, "ITEM_LIN1", 9, qty_avail);
	pr_format (fin, fout, "ITEM_LIN1", 10, qty_on_order);
	pr_format (fin, fout, "ITEM_LIN1", 11, qty_tot_cover);
	pr_format (fin, fout, "ITEM_LIN1", 12, qty_cover_reqd);
	pr_format (fin, fout, "ITEM_LIN1", 13, qty_net_reqt);
	pr_format (fin, fout, "ITEM_LIN1", 14, qty_recommended);
	if (proposedReport)
		pr_format (fin, fout, "ITEM_LIN1", 15, ffwk_rec.order_qty);

	if (manufactItem	== FALSE)
	{
		cc = find_rec (poln, &poln_rec, NEXT, "r");
		while (!cc && poln_rec.hhbr_hash == inmr_rec.hhbr_hash)
		{
			on_order = poln_rec.qty_ord - poln_rec.qty_rec;

			if (ValidateWarehouse (poln_rec.hhcc_hash) && on_order > 0.0)
			{
				pohr_rec.hhpo_hash	=	poln_rec.hhpo_hash;
				cc = find_rec (pohr, &pohr_rec, EQUAL, "r");
				if (pohr_rec.drop_ship [0] != 'Y')
				{
					qty_outstand = on_order;
					break;
				}
			}
			cc = find_rec (poln, &poln_rec, NEXT, "r");
		}

		if (cc || poln_rec.hhbr_hash != inmr_rec.hhbr_hash)
		{
			pohr_rec.hhsu_hash = inis_rec.hhsu_hash;
			poln_rec.due_date = 0L;
			qty_outstand = (float) 0.00;
		}
		else
		{
			pohr_rec.hhpo_hash	=	poln_rec.hhpo_hash;
			cc = find_rec (pohr, &pohr_rec, EQUAL, "r");
		}
	}
	else
	{
		cc = find_rec (pcwo, &pcwo_rec, NEXT, "r");
		while (!cc && pcwo_rec.hhbr_hash == inmr_rec.hhbr_hash)
		{
			/*-----------------------------------------
			| Ignore closed and deleted works orders. |
			-----------------------------------------*/
			if (INVALID_WO)
			{
				cc = find_rec (pcwo, &pcwo_rec, NEXT, "r");
				continue;
			}
			on_order = pcwo_rec.prod_qty - 
				  	  (pcwo_rec.act_prod_qty + pcwo_rec.act_rej_qty);
			if (ValidateWarehouse (pcwo_rec.hhcc_hash) && on_order > 0.0)
			{
				qty_outstand += on_order;
				break;
			}
			cc = find_rec (pcwo, &pcwo_rec, NEXT, "r");
		}
		if (cc || pcwo_rec.hhbr_hash != inmr_rec.hhbr_hash)
		{
			pcwo_rec.reqd_date = 0L;
			qty_outstand = (float) 0.00;
		}
	}

	pr_format (fin, fout, "ITEM_LIN2", 1, inmr_rec.description);
	if (inis_rec.hhsu_hash == 0L)
	{
		sprintf (err_str, "W/House:  %2.2s", ccmr2_rec.cc_no);
		pr_format (fin, fout, "ITEM_LIN2", 2, err_str);
	}
	else
		pr_format (fin, fout, "ITEM_LIN2", 2, "            ");
	pr_format (fin, fout, "ITEM_LIN2", 3, inis_rec.min_order);
	if (manufactItem)
		pr_format (fin, fout, "ITEM_LIN2", 4, "W");
	else
	{
		pr_format (fin, fout, "ITEM_LIN2", 4,
	    	(inis_rec.hhsu_hash == pohr_rec.hhsu_hash) ? " " : "*");
	}
	pr_format (fin, fout, "ITEM_LIN2", 5, (manufactItem) ? pcwo_rec.reqd_date
														 : poln_rec.due_date);
	pr_format (fin, fout, "ITEM_LIN2", 6, qty_outstand);
	pr_format (fin, fout, "ITEM_LIN2", 7, wks_avail);
	pr_format (fin, fout, "ITEM_LIN2", 8, wks_on_order);
	pr_format (fin, fout, "ITEM_LIN2", 9, wks_tot_cover);
	pr_format (fin, fout, "ITEM_LIN2", 10, wks_cover_reqd);
	pr_format (fin, fout, "ITEM_LIN2", 11, wks_net_reqt);
	pr_format (fin, fout, "ITEM_LIN2", 12, val_recommended);
	if (proposedReport)
		pr_format (fin, fout, "ITEM_LIN2", 13, val_proposed);

	if (manufactItem == FALSE)
	{
		cc = find_rec (poln, &poln_rec, NEXT, "r");
		while (!cc && poln_rec.hhbr_hash == inmr_rec.hhbr_hash)
		{
			on_order = poln_rec.qty_ord - poln_rec.qty_rec;
			if (ValidateWarehouse (poln_rec.hhcc_hash) && on_order > 0.0)
			{
				pohr_rec.hhpo_hash	=	poln_rec.hhpo_hash;
				cc = find_rec (pohr, &pohr_rec, EQUAL, "r");
				if (pohr_rec.drop_ship [0] != 'Y')
				{
					qty_outstand = on_order;
					break;
				}
			}
			cc = find_rec (poln, &poln_rec, NEXT, "r");
		}
		if (cc || poln_rec.hhbr_hash != inmr_rec.hhbr_hash)
		{
			pohr_rec.hhsu_hash = inis_rec.hhsu_hash;
			poln_rec.due_date = 0L;
			qty_outstand = (float) 0.00;
		}
		else
		{
			pohr_rec.hhpo_hash	=	poln_rec.hhpo_hash;
			cc = find_rec (pohr, &pohr_rec, EQUAL, "r");
		}
	}
	else
	{
		cc = find_rec (pcwo, &pcwo_rec, NEXT, "r");
		while (!cc && pcwo_rec.hhbr_hash == inmr_rec.hhbr_hash)
		{
			/*-----------------------------------------
			| Ignore closed and deleted works orders. |
			-----------------------------------------*/
			if (INVALID_WO)
			{
				cc = find_rec (pcwo, &pcwo_rec, NEXT, "r");
				continue;
			}
			on_order = pcwo_rec.prod_qty - 
				  	  (pcwo_rec.act_prod_qty + pcwo_rec.act_rej_qty);

			if (ValidateWarehouse (pcwo_rec.hhcc_hash) && on_order > 0.0)
			{
				qty_outstand += on_order;
				break;
			}
			cc = find_rec (pcwo, &pcwo_rec, NEXT, "r");
		}
		if (cc || pcwo_rec.hhbr_hash != inmr_rec.hhbr_hash)
		{
			pcwo_rec.reqd_date = 0L;
			qty_outstand = (float) 0.00;
		}
	}
	inum_rec.hhum_hash	=	inis_rec.sup_uom;
	cc = find_rec (inum, &inum_rec, COMPARISON, "r");
	if (cc)
		strcpy (inum_rec.uom, inmr_rec.sale_unit);

	pr_format (fin, fout, "ITEM_LIN3", 1, reviewPeriod);
	pr_format (fin, fout, "ITEM_LIN3", 2, lead_weeks);
	pr_format (fin, fout, "ITEM_LIN3", 3, (useMinStockWks) ? "MN" : "SS");
	pr_format (fin, fout, "ITEM_LIN3", 4, (useMinStockWks) ? minStockWks : safety_stock);
	pr_format (fin, fout, "ITEM_LIN3", 5, cover);
	pr_format (fin, fout, "ITEM_LIN3", 6, inum_rec.uom);
	pr_format (fin, fout, "ITEM_LIN3", 7, inis_rec.norm_order);
	if (manufactItem)
		pr_format (fin, fout, "ITEM_LIN3", 8, "W");
	else
	{
		pr_format (fin, fout, "ITEM_LIN3", 8,
	    	(inis_rec.hhsu_hash == pohr_rec.hhsu_hash) ? " " : "*");
	}
	pr_format (fin, fout, "ITEM_LIN3", 9, (manufactItem) ? pcwo_rec.reqd_date
														 : poln_rec.due_date);
	pr_format (fin, fout, "ITEM_LIN3", 10, qty_outstand);
	pr_format (fin, fout, "ITEM_LIN3", 11, inei_rec.date_lcost);
	pr_format (fin, fout, "ITEM_LIN3", 12, inei_rec.lpur_qty);

	if (manufactItem == FALSE)
	{
		cc = find_rec (poln, &poln_rec, NEXT, "r");
		while (!cc &&  poln_rec.hhbr_hash == inmr_rec.hhbr_hash)
		{
			if (!ValidateWarehouse (poln_rec.hhcc_hash))
			{
				cc = find_rec (poln, &poln_rec, NEXT, "r");
				continue;
			}
			qty_outstand = poln_rec.qty_ord - poln_rec.qty_rec;

			if (qty_outstand > 0.00)
			{
				pohr_rec.hhpo_hash	=	poln_rec.hhpo_hash;
				cc = find_rec (pohr, &pohr_rec, EQUAL, "r");
				if (pohr_rec.drop_ship [0] != 'Y')
				{
					pr_format (fin, fout, "PO_LINE", 1,
			    	 	(inis_rec.hhsu_hash == pohr_rec.hhsu_hash) ? " " : "*");
					pr_format (fin, fout, "PO_LINE", 2, poln_rec.due_date);
					pr_format (fin, fout, "PO_LINE", 3, qty_outstand);
				}
			}
			cc = find_rec (poln, &poln_rec, NEXT, "r");
		}
	}
	else
	{
		cc = find_rec (pcwo, &pcwo_rec, NEXT, "r");
		while (!cc &&  pcwo_rec.hhbr_hash == inmr_rec.hhbr_hash)
		{
			/*-----------------------------------------
			| Ignore closed and deleted works orders. |
			-----------------------------------------*/
			if (INVALID_WO)
			{
				cc = find_rec (pcwo, &pcwo_rec, NEXT, "r");
				continue;
			}
			if (!ValidateWarehouse (pcwo_rec.hhcc_hash))
			{
				cc = find_rec (pcwo, &pcwo_rec, NEXT, "r");
				continue;
			}
			qty_outstand = pcwo_rec.prod_qty - 
						  (pcwo_rec.act_prod_qty + pcwo_rec.act_rej_qty);
			if (qty_outstand > 0.00)
			{
				pr_format (fin, fout, "PO_LINE", 1, "W");
				pr_format (fin, fout, "PO_LINE", 2, pcwo_rec.reqd_date);
				pr_format (fin, fout, "PO_LINE", 3, qty_outstand);
			}
			cc = find_rec (pcwo, &pcwo_rec, NEXT, "r");
		}
	}
	fprintf (fout, ".LRP4\n");

	return;
}

/*===============================
| Get poln (&itln) qty on order	|
| between 0/0/0 and last.		|
===============================*/
float	
GetOnOrder (
 long   hhbrHash,
 long   EndCvr)
{
	float	tmp_qty		= 0.00,
			tot_qty		= 0.00;
	int		idx,
			valid;

	/*----------------------------
	| Read purchase order lines. |
	----------------------------*/
	poln_rec.hhbr_hash	=	hhbrHash;
	poln_rec.due_date	=	0L;
	cc = find_rec (poln, &poln_rec, GTEQ, "r");
	while (!cc && poln_rec.hhbr_hash == hhbrHash)
	{
	    valid = FALSE;
	    for (idx = 0; idx < MAX_CCMR && whouse [idx].hhccHash != 0L; idx++)
	    {
			if (poln_rec.hhcc_hash == whouse [idx].hhccHash)
			{
		    	valid = TRUE;
		    	break;
			}
	    }
	    tmp_qty = poln_rec.qty_ord - poln_rec.qty_rec;
	    if (valid && tmp_qty > 0.00)
		{	
			pohr_rec.hhpo_hash	=	poln_rec.hhpo_hash;
			cc = find_rec (pohr, &pohr_rec, EQUAL, "r");
			if (pohr_rec.drop_ship [0] != 'Y')
				tot_qty += tmp_qty;
		}	
	    cc = find_rec (poln, &poln_rec, NEXT, "r");
	}
	/*--------------------
	| Read works orders. |
	--------------------*/
	pcwo_rec.hhbr_hash	=	hhbrHash;
	pcwo_rec.reqd_date	=	0L;
	cc = find_rec (pcwo, &pcwo_rec, GTEQ, "r");
	while (!cc && pcwo_rec.hhbr_hash == hhbrHash)
	{
		/*-----------------------------------------
		| Ignore closed and deleted works orders. |
		-----------------------------------------*/
		if (INVALID_WO)
		{
			cc = find_rec (pcwo, &pcwo_rec, NEXT, "r");
			continue;
		}
	    valid = FALSE;
	    for (idx = 0; idx < MAX_CCMR && whouse [idx].hhccHash != 0L; idx++)
	    {
			if (pcwo_rec.hhcc_hash == whouse [idx].hhccHash)
			{
		    	valid = TRUE;
		    	break;
			}
	    }
		tmp_qty = pcwo_rec.prod_qty - 
				  (pcwo_rec.act_prod_qty + pcwo_rec.act_rej_qty);

	    if (valid && tmp_qty > 0.00)
			tot_qty	+=	tmp_qty;
		
	    cc = find_rec (pcwo, &pcwo_rec, NEXT, "r");
	}

	/*----------------------
	| Read transfer lines. |
	----------------------*/
	itln_rec.hhbr_hash	=	hhbrHash;
	itln_rec.due_date	=	0L;
	cc = find_rec (itln, &itln_rec, GTEQ, "r");
	while (!cc && itln_rec.hhbr_hash == hhbrHash && 
				   itln_rec.due_date <= EndCvr)
	{
	    valid = FALSE;
	    for (idx = 0; idx < MAX_CCMR && whouse [idx].hhccHash != 0L; idx++)
	    {
			if (itln_rec.r_hhcc_hash == whouse [idx].hhccHash)
			{
		    	valid = TRUE;
		    	break;
			}
	    }
	    if (valid)
	    {
			switch (itln_rec.status [0])
			{
				case	'B':
				case	'M':
				case	'U':
					if (byCompany)
						break;
				case	'T':
					if (byCompany)
						tot_qty += itln_rec.qty_order;
					else
			   			if (itln_rec.r_hhcc_hash == incc_rec.hhcc_hash)
			   			{
							tot_qty += itln_rec.qty_order;
							tot_qty += itln_rec.qty_border;
			   			}
					break;

				default:
					break;
			}
	    }
	    cc = find_rec (itln, &itln_rec, NEXT, "r");
	}
	return (tot_qty);
}

/*===============================================================
| Process review period for category, Later review periods for  |
| Item or supplier may be added.                                |
===============================================================*/
float
FindReviewPeriod (void)
{
	/*---------------------------------------
	| Find out what the review-period       |
	| is for this product. Firstly, try for	|
	| a match on branch/item. Then try for	|
	| a match on item. Then try for a match	|
	| on branch/category. If this fails,	|
	| then use LRP_DFLT_REVIEW environment-	|
	| value. If not found, dflt to 4 weeks.	|
	---------------------------------------*/
	ffpr_rec.hhbr_hash = inmr_rec.hhbr_hash;
	strcpy (ffpr_rec.br_no, comm_rec.est_no);
	cc = find_rec (ffpr, &ffpr_rec, EQUAL, "r");
	if (cc)
	{
	    strcpy (ffpr_rec.br_no, "  ");
	    cc = find_rec (ffpr, &ffpr_rec, EQUAL, "r");
	    if (cc)
	    {
			abc_selfield (ffpr, "ffpr_id_no_1");
			strcpy (ffpr_rec.category, inmr_rec.category);
			strcpy (ffpr_rec.br_no, comm_rec.est_no);
			cc = find_rec (ffpr, &ffpr_rec, EQUAL, "r");
			if (cc)
			{
		    	strcpy (ffpr_rec.br_no, "  ");
		    	cc = find_rec (ffpr, &ffpr_rec, EQUAL, "r");
		    	if (cc)
				ffpr_rec.review_prd = defaultReview;
			}
			abc_selfield (ffpr, "ffpr_id_no");
	    }
	}
	return (ffpr_rec.review_prd);
}

/*===============================================
| Get the number of weeks between 'date' & the	|
| next available inld_sup_date.				    |
| Return 0 if none found.				        |
===============================================*/
float	
GetLeadTime (
 long   HHIS_HASH,
 long   ORD_DATE)
{
	float	days;

	inld_rec.hhis_hash	=	HHIS_HASH;
	inld_rec.ord_date	=	ORD_DATE;

	cc = find_rec (inld, &inld_rec, GTEQ, "r");
	if (cc)
		return ((float) 0.00);

	days = inld_rec.sup_date - ORD_DATE;
	return (days);
}

/*===============================
| Check that all of the valid	|
| branches for the selected co.	|
| are in the same inv. month	|
===============================*/
int
CheckDates (void)
{
	int		hold_date = -1L;
	int		tmp_month,
			tmp_year;
	int		tmp_dmy [3];

	if (CO_INV)
		return (EXIT_SUCCESS);

	strcpy (esmr_rec.co_no, comm_rec.co_no);
	strcpy (esmr_rec.est_no, "  ");
	cc = find_rec (esmr, &esmr_rec, GTEQ, "r");
	while (!cc && !strcmp (esmr_rec.co_no, comm_rec.co_no))
	{
		if (hold_date == -1L)
		{
			DateToDMY (esmr_rec.inv_date, NULL, &tmp_month, &tmp_year);
			hold_date = esmr_rec.inv_date;
		}
		else
		{
			DateToDMY(esmr_rec.inv_date,&tmp_dmy [0],&tmp_dmy [1],&tmp_dmy [2]);
			if (tmp_dmy [1] != tmp_month ||
				tmp_dmy [2] != tmp_year)
			{
				print_mess (ML ("Not all branches is in the same inventory month."));
				sleep (sleepTime);
				shutdown_prog ();
                return (EXIT_FAILURE);
			}
		}
		cc = find_rec (esmr, &esmr_rec, NEXT, "r");
	}
    return (EXIT_SUCCESS);
}

static 
float
RoundMultiple (
	float  orderQty,
	char*  roundType,
	float  orderMultiple,
	float  minimumQty)
{
	double	wrk_qty;
	double	up_qty;
	double	down_qty;

	if (orderQty == 0.00)
		return (0.00);

	if (orderMultiple == 0.00)
		return ((orderQty < minimumQty) ? minimumQty : orderQty);

	orderQty -= minimumQty;
	if (orderQty < 0.00)
		orderQty = 0.00;

	/*---------------------------
	| Already An Exact Multiple |
	---------------------------*/
	wrk_qty = (double) (orderQty / orderMultiple);
	if (ceil (wrk_qty) == wrk_qty)
		return (orderQty + minimumQty);

	/*------------------
	| Perform Rounding |
	------------------*/
	switch (roundType [0])
	{
	case 'U':
		/*------------------------------
		| Round Up To Nearest Multiple |
		------------------------------*/
		wrk_qty = (double) (orderQty / orderMultiple);
		wrk_qty = ceil (wrk_qty);
		orderQty = (float) (wrk_qty * orderMultiple);
		break;

	case 'D':
		/*--------------------------------
		| Round Down To Nearest Multiple |
		--------------------------------*/
		wrk_qty = (double) (orderQty / orderMultiple);
		wrk_qty = floor (wrk_qty);
		orderQty = (float) (wrk_qty * orderMultiple);
		break;

	case 'B':
		/*--------------------------
		| Find Value If Rounded Up |
		--------------------------*/
		up_qty = (double) orderQty;
		wrk_qty = (up_qty / (double)orderMultiple);
		wrk_qty = ceil (wrk_qty);
		up_qty = (float) (wrk_qty * orderMultiple);

		/*----------------------------
		| Find Value If Rounded Down |
		----------------------------*/
		down_qty = (double) orderQty;
		wrk_qty = (down_qty / (double) orderMultiple);
		wrk_qty = floor (wrk_qty);
		down_qty = (float) (wrk_qty * orderMultiple);

		/*-----------------------------------
		| Round Up/Down To Nearest Multiple |
		-----------------------------------*/
		if ((up_qty - (double) orderQty) <= ((double) orderQty - down_qty))
			orderQty = (float) up_qty;
		else
			orderQty = (float) down_qty;

		break;

	default:
		break;
	}

	return (minimumQty + orderQty);
}

/*===============================================
| Read the ffdm record for the appropriate year	|
| if record doesn't exist and year is valid		|
| then get data from incc record				|
===============================================*/
void
LoadMonthsHistory (
	long	hhbrHash,
	long	hhccHash,
	long	StartDate,
	char	*type)
{
	int		i;

	CalcDates (StartDate);

	for (i = 0; i < MAX_MONTHS; i++)
	    demand_value [i] = 0.00;

	ffdm_rec.hhbr_hash	=	hhbrHash;
	ffdm_rec.hhcc_hash	=	hhccHash;
	sprintf (ffdm_rec.type, "%1.1s", type);
	ffdm_rec.date	=	store_dates [0].StartDate;
	cc = find_rec (ffdm, &ffdm_rec, GTEQ, "r");

	while (!cc && ffdm_rec.hhbr_hash	==	hhbrHash &&
				  ffdm_rec.hhcc_hash	==	hhccHash &&
				  ffdm_rec.type [0] 	==  type [0] &&
				  ffdm_rec.date 		<= store_dates [MAX_MONTHS - 1].EndDate)
	{
		for (i = 0; i < MAX_MONTHS; i++)
		{
			if (ffdm_rec.date >= store_dates [i].StartDate &&
			    ffdm_rec.date <= store_dates [i].EndDate)
			{
				store_dates [i].QtySold	+= twodec (ffdm_rec.qty);
			}
		}
		cc = find_rec (ffdm, &ffdm_rec, NEXT, "r");
	}
	for (i = 0; i < MAX_MONTHS; i++)
		demand_value [i] = store_dates [i].QtySold;
}

/*===============================================
| Calculate start and end dates for each month. |
===============================================*/
void
CalcDates (
 long	StartDate)
{
	long	CalcStartDate;
	int		i;
	int		tmp_dmy [3];

	DateToDMY (StartDate, &tmp_dmy [0], &tmp_dmy [1], &tmp_dmy [2]);
	tmp_dmy [2] -= 3;
	CalcStartDate = DMYToDate (tmp_dmy [0], tmp_dmy [1], tmp_dmy [2]);

	for (i = 0; i < MAX_MONTHS; i++)
	{
		store_dates [i].StartDate	=	MonthStart (CalcStartDate);
		store_dates [i].EndDate		=	MonthEnd (CalcStartDate);
		store_dates [i].QtySold		=	0.00;
		CalcStartDate	=	MonthEnd (CalcStartDate) + 1;
	}
}
/*===============================================
| Read the ffdm record for the appropriate year	|
| if record doesn't exist and year is valid		|
| then get data from incc record				|
===============================================*/
void
LoadDaysHistory (
	long	hhbrHash,
	long	hhccHash,
	long	startDate,
	long	endDate,
	char	*type)
{
	int		i;

	for (i = 0; i < MAX_MONTHS; i++)
	    demand_value [i] = 0.00;

print_at (0,0, "Start = [%s]", DateToString (startDate));
print_at (1,1, "Start = [%s]", DateToString (endDate));getchar();
	ffdm_rec.hhbr_hash	=	hhbrHash;
	ffdm_rec.hhcc_hash	=	hhccHash;
	sprintf (ffdm_rec.type, "%1.1s", type);
	ffdm_rec.date	=	startDate;
	cc = find_rec (ffdm, &ffdm_rec, GTEQ, "r");

	while (!cc && ffdm_rec.hhbr_hash	==	hhbrHash &&
				  ffdm_rec.hhcc_hash	==	hhccHash &&
				  ffdm_rec.type [0] 	==  type [0] &&
				  ffdm_rec.date 		<=  endDate)
	{
		i = ffdm_rec.date - startDate;
		demand_value [i] += twodec (ffdm_rec.qty);
		cc = find_rec (ffdm, &ffdm_rec, NEXT, "r");
	}
}

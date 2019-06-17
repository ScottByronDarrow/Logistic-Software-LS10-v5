/*=====================================================================
|  Copyright (C) 1986 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : (lrp_demand.c )                                    |
|  Program Desc  : (Print Demand Forecast Report                )     |
|                  (                                            )     |
|---------------------------------------------------------------------|
|  Date Written  : (15/08/1998)    | Author      : Scott B Darrow     |
|---------------------------------------------------------------------|
|  Date Modified : (DD/MM/YYYY)    | Modified By :                    |
|   Comments     :                                                    |
|  (DD/MM/YYYY)  :                                                    |
|                :                                                    |
|                :                                                    |
|                                                                     |
| $Log: lrp_demand.c,v $
| Revision 5.2  2001/08/09 09:29:41  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:27:22  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:07:12  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:28:16  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:15:21  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 08:58:32  gerry
| Forced Revision No. Start 2.0 Rel-15072000
|
| Revision 1.36  2000/06/13 05:47:05  scott
| S/C LSANZ 16400
| Updated to allow for demand type '6' related to production issues.
| NOTE : Please see release notes on new search.
| sk_mrmaint, sk_delete, psl_sr_gen and sch.srsk must be installed/rebuilt.
|
| Revision 1.35  2000/06/13 05:28:58  scott
| S/C LSANZ 16400
| Updated to allow for demand type '6' related to production issues.
| NOTE : Please see release notes on new search.
| sk_mrmaint, sk_delete, psl_sr_gen and sch.srsk must be installed/rebuilt.
|
| Revision 1.34  2000/05/31 04:23:45  scott
| Updated to remove unused fields from inis. No effect on program.
|
| Revision 1.33  2000/04/08 04:09:49  gerry
| Corrected read lock to write lock of incc
|
| Revision 1.32  2000/03/16 06:38:07  scott
| Updated to re-select method 'D' when over percent or erratic rather than predicting zero.
|
| Revision 1.31  2000/01/18 09:08:03  scott
| Updated to remove some debug comments.
|
| Revision 1.30  2000/01/18 09:01:16  scott
| Not sure
|
| Revision 1.29  1999/12/10 04:09:17  scott
| Updated to remove the space between @ and (#) as this prevended version from being displayed correctly. Reported by sunlei
|
| Revision 1.28  1999/12/08 05:40:58  scott
| Updated to add new printing functions and also add loging of LRP history information.
|
| Revision 1.27  1999/12/06 20:35:08  cam
| Changes for GVision compatibility.  Changed name of inmr_rec.class variable to
| inmr_rec.inmr_class.  class is a C++ keyword.
|
| Revision 1.26  1999/12/06 01:34:12  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.25  1999/11/12 07:59:34  scott
| Updated to modify testing of erratic.
|
| Revision 1.24  1999/11/04 05:47:43  scott
| Updated to allow a warehouse to be excluded from LRP.
|
| Revision 1.23  1999/11/03 00:22:10  scott
| Updated to change environment FF_ to LRP_
|
| Revision 1.22  1999/10/27 07:32:52  scott
| Updated for -Wall warnings + modifications for ASL on percentage error.
|
| Revision 1.21  1999/10/13 06:47:33  scott
| Updated for lrp_demand.c
|
| Revision 1.20  1999/10/11 22:38:39  scott
| Updated for Date Routines
|
| Revision 1.19  1999/09/29 10:10:41  scott
| Updated to be consistant on function names.
|
| Revision 1.18  1999/09/17 07:30:33  scott
| Ansi
|
| Revision 1.17  1999/09/17 07:26:31  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.16  1999/09/16 09:20:35  scott
| Updated from Ansi Project
|
| Revision 1.15  1999/06/23 05:53:42  scott
| Updated as test version being used.
|
| Revision 1.14  1999/06/22 09:19:56  scott
| Updated to change weeks demand calculation to remove strange code related to lead times and future buckets. Also updated to calculate weeks based on 4.348 not 4.29 days per week.
|
| Revision 1.13  1999/06/16 04:12:25  scott
| Updated for possible cause why some items are not updated.
|
| Revision 1.12  1999/06/15 07:26:58  scott
| Updated to add log file.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: lrp_demand.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/LRP/lrp_demand/lrp_demand.c,v 5.2 2001/08/09 09:29:41 scott Exp $";

#include <pslscr.h>
#include <ml_lrp_mess.h>
#include <ml_std_mess.h>

#include <dsp_screen.h>
#include <dsp_process.h>
#include <twodec.h>

#define	MAX_CCMR	100

#define	CO_INV		 (companyClose[2] == '1')

#define	METH_A		1
#define	METH_B		2
#define	METH_C		3
#define	METH_D		4

#define	MAX_LOOPS	100

#include	"schema"

struct ccmrRecord	ccmr_rec;
struct ccmrRecord	ccmr2_rec;
struct commRecord	comm_rec;
struct esmrRecord	esmr_rec;
struct ffprRecord	ffpr_rec;
struct inccRecord	incc_rec;
struct lrphRecord	lrph_rec;
struct inisRecord	inis_rec;
struct inldRecord	inld_rec;
struct inmrRecord	inmr_rec;
struct ineiRecord	inei_rec;

	struct	fcast_type
	{
		char	f_method[2];
		int		f_method_no;
		float	f_actual;
		float	f_forecast;
		float	f_wks_demand;
		float	f_sqr_err;
		float	f_pc_err;
	} store [6];

	struct
	{
		long	hhcc_hash;
		char	br_no[3];
		long	br_inv_date;
		char	wh_no[3];
		char	wh_name[10];
	} whouse[MAX_CCMR];

	int		byCompany 		= FALSE,
			byBranch 		= FALSE,
			byWarehouse 	= FALSE,
			maxMethods,
			firstWarehouse,
			firstPartNo,
			printerNumber,
			printReport,
			updateData,
			curr_ccmr_ix,
			negativeDemandOk, 
			biasFlag,
			LoopingWarehouse,
			HistoryMonths;

	char	DemandInclude[6];
	char	companyClose[6];
	long	commHhccHash;

	char	lower [13],
			upper [13],
			validMethods [5],
			abcCodes [5],
			selected [2],
			manual[2];

	FILE	*fout;

	float	leadTime;
	float	defaultReview = 4;

	double	errorThreshold;

	char	*ccmr2	= "ccmr2";

#include	<LRPFunctions.h>

#define	BY_CO	0
#define	BY_BR	1
#define	BY_WH	2

/*============================
| Local Function Prototypes. |
============================*/
float 	GetLeadDate 		(long, long);
float 	GetLeadTime 		(long, char *, char *);
int		BestMethod 			(int, int);
int 	CalcPercentError 	(int);
int 	ValidMethods 		(void);
int 	CalculateErratic 	(int);
int 	check_page 			(void);
void 	CalculateDemand 	(int, int);
void 	CalcMethods 		(int);
void 	CheckDates 			(void);
void 	ClearMethods 		(void);
void 	Forecast 			(long);
void 	ForecastCompany 	(long);
void 	HeadingOutput 		(void);
void 	LoadHhcc 			(void);
void 	Method_A 			(int);
void 	Method_B 			(int);
void 	Method_D 			(int);
void 	Method_C 			(int);
void 	PrintMethod 		(int, int, int, int, int);
void 	Process 			(void);
void 	UpdateIncc 			(int);
void 	CloseDB 			(void);
void 	OpenDB 				(void);
void 	shutdown_prog 		(void);

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int    argc,
 char*  argv[])
{
	char	*sptr;

	if (argc != 12)
	{
		print_at (0,0, "Usage : %s <printerNumber> <start_group> <end_group> <print> <update> <manual> <selected> <validMethods> <val_abc> <no_months> <demand_included>\007\n\r", argv[0]);
		return (EXIT_FAILURE);
	}

	sptr = chk_env ("CO_CLOSE");
	if (sptr == (char *)0)
		sprintf (companyClose,"%-5.5s","11111");
	else
		sprintf (companyClose,"%-5.5s",sptr);

	/*----------------------------------
	| Check if negative demand allowed |
	----------------------------------*/
	sptr = chk_env ("LRP_DMND_NEG");
	negativeDemandOk	= (sptr == (char *) 0) ? FALSE : atoi (sptr);

	sptr = chk_env ("LRP_ERRATIC");
	errorThreshold	= (sptr == (char *) 0) ? 0.00 : atof (sptr);

	sptr = chk_env ("LRP_BIAS");
	biasFlag	= (sptr == (char *) 0) ? FALSE : atoi (sptr);

	byCompany 		= FALSE;
	byBranch 		= FALSE;
	byWarehouse 	= FALSE;

	sptr = strrchr (argv[0], '/');
	if (sptr == (char *) 0)
		sptr = argv[0];
	else
		sptr++;

	if (!strncmp (sptr, "lrp_cd", 6))
		byCompany 	= TRUE;

	if (!strncmp (sptr, "lrp_bd", 6))
		byBranch 	= TRUE;

	if (!strncmp (sptr, "lrp_de", 6))
		byWarehouse = TRUE;

	sptr = chk_env ("LRP_DFLT_REVIEW");
	if (sptr == (char *) 0)
		defaultReview = 4;
	else
		defaultReview = atof (sptr);

	/*-------------------
	| Printer Number	|
	-------------------*/
	printerNumber 	= atoi (argv[1]);
	sprintf (lower, "%-13.13s", argv[2]);
	sprintf (upper, "%-13.13s", argv[3]);
	printReport 		= (argv[4][0] == 'Y');
	updateData 			= (argv[5][0] == 'Y');
	sprintf (manual, 	"%-1.1s",	argv[6]);
	sprintf (selected, 	"%-1.1s",	argv[7]);
	strcpy (validMethods, argv[8]);
	maxMethods 	= strlen (validMethods);
	sprintf (abcCodes, "%-4.4s",	argv[9]);
	HistoryMonths	=	atoi (argv[10]);
	sprintf (DemandInclude, "%-5.5s", argv[11]);

	init_scr ();

	OpenDB ();

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	strcpy (ccmr_rec.co_no, comm_rec .co_no);
	strcpy (ccmr_rec.est_no, comm_rec .est_no);
	strcpy (ccmr_rec.cc_no, comm_rec .cc_no);
	if (!find_rec (ccmr, &ccmr_rec, EQUAL, "r"))
		commHhccHash = ccmr_rec.hhcc_hash;
	else
	{
		print_mess ("Current Warehouse not Found");
		sleep (sleepTime);
		printReport = FALSE;
		shutdown_prog ();
        return (EXIT_SUCCESS);
	}

	if (byCompany)
		CheckDates ();

	dsp_screen ("Processing : Demand Forecast Report.", comm_rec .co_no, comm_rec .co_name);

	HeadingOutput ();

	Process ();

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*========================
| Program exit sequence. |
========================*/
void
shutdown_prog (void)
{
	if (printReport)
	{
		fprintf (fout, ".EOF\n");
		pclose (fout);
	}

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

	abc_alias (ccmr2, ccmr);
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	open_rec (ccmr2,ccmr_list, CCMR_NO_FIELDS, "ccmr_hhcc_hash");
	open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (ffpr, ffpr_list, FFPR_NO_FIELDS, "ffpr_id_no");
	open_rec (inei, inei_list, INEI_NO_FIELDS, "inei_id_no");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (inis, inis_list, INIS_NO_FIELDS, "inis_id_no2");
	open_rec (inld, inld_list, INLD_NO_FIELDS, "inld_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no_3");
	open_rec (lrph, lrph_list, LRPH_NO_FIELDS, "lrph_hhwh_hash");
	LSA_open ();
}

/*=========================
| Close data base files . |
=========================*/
void 
CloseDB (void)
{
	abc_fclose (ccmr);
	abc_fclose (ccmr2);
	abc_fclose (esmr);
	abc_fclose (inei);
	abc_fclose (ffpr);
	abc_fclose (incc);
	abc_fclose (inis);
	abc_fclose (inld);
	abc_fclose (inmr);
	abc_fclose (lrph);
	LSA_close ();
	abc_dbclose ("data");
}

/*===============================
| Check that all of the valid	|
| branches for the selected co.	|
| are in the same inv. month	|
===============================*/
void
CheckDates (void)
{
	int		hold_date = -1L;
	int		tmp_month,
			tmp_year;
	int		tmpDmy[3];

	if (CO_INV)
		return;

	strcpy (esmr_rec.co_no, comm_rec .co_no);
	strcpy (esmr_rec.est_no, "  ");
	cc = find_rec (esmr, &esmr_rec, GTEQ, "r");
	while (!cc && !strcmp (esmr_rec.co_no, comm_rec .co_no))
	{
		if (hold_date == -1L)
		{
			DateToDMY (esmr_rec.inv_date, NULL, &tmp_month, &tmp_year);
			hold_date = esmr_rec.inv_date;
		}
		else
		{
			DateToDMY (esmr_rec.inv_date, NULL, &tmpDmy[1], &tmpDmy[2]);
			if (tmpDmy [1] != tmp_month ||
				tmpDmy [2] != tmp_year)
			{
				print_mess (ML (mlLrpMess040));
				sleep (sleepTime);
				printReport = FALSE;
				shutdown_prog ();
			}
		}
		cc = find_rec (esmr, &esmr_rec, NEXT, "r");
	}
}

/*===================================
| Start Out Put To Standard Print . |
===================================*/
void
HeadingOutput (void)
{
	char	RunDesc[256];
	int		cur_mth;

	if (!printReport)
		return;

	DateToDMY (comm_rec .inv_date, NULL, &cur_mth, NULL);

	if ((fout = popen ("pformat", "w")) == NULL)
		sys_err ("Error in opening pformat During (POPEN)", errno, PNAME);
	
	fprintf (fout, ".START%s\n", DateToString (comm_rec .inv_date));
	fprintf (fout, ".LP%d\n", printerNumber);

	fprintf (fout, ".%d\n", (byCompany) ? 15 : 17);
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L158\n");
	fprintf (fout, ".B1\n");
	if (byCompany)
		fprintf (fout, ".EDEMAND FORECAST REPORT BY COMPANY FOR MONTH %02d\n", cur_mth + 1);
	if (byBranch)
		fprintf (fout, ".EDEMAND FORECAST REPORT BY BRANCH FOR MONTH %02d\n", cur_mth + 1);
	if (byWarehouse)
		fprintf (fout, ".EDEMAND FORECAST REPORT BY WAREHOUSE FOR MONTH %02d\n", cur_mth + 1);

	fprintf (fout, ".B1\n");

	strcpy (RunDesc, "Demand = Sales ");
	if (strchr (DemandInclude, PLUS_TRANSFERS) != (char *)0)
		strcat (RunDesc, "+ Transfers ");
	if (strchr (DemandInclude, PLUS_LOSTSALES) != (char *)0)
		strcat (RunDesc, "+ Lost Sales ");
	if (strchr (DemandInclude, PLUS_PC_ISSUES) != (char *)0)
		strcat (RunDesc, "+ Production issues");

	fprintf (fout, ".CUpdate Method (%s) /  Forecast option (%s) / Forecast methods (%s) / ABC codes (%s) / No Months History (%d) / %s\n", 
					 (updateData) ? "YES" : "NO ",
					manual,
					validMethods,
					abcCodes,
					HistoryMonths, 
					RunDesc);

	fprintf (fout, ".E%s\n", clip (comm_rec .co_name));
	if (!byCompany)
	{
		fprintf (fout, ".B1\n");
		if (byBranch)
			fprintf (fout, ".E%s\n", clip (comm_rec .est_name));
		else
			fprintf (fout, ".E%s-%s\n", clip (comm_rec .est_name),
						    clip (comm_rec .cc_name));
	}
	fprintf (fout, ".B1\n");
	fprintf (fout, ".EAS AT %s\n", SystemTime ());

	fprintf (fout, ".R===================");
	fprintf (fout, "===========================================");
	fprintf (fout, "======");
	fprintf (fout, "============");
	fprintf (fout, "=========");
	fprintf (fout, "=========");
	fprintf (fout, "============");
	fprintf (fout, "============");
	fprintf (fout, "==============================\n");

	fprintf (fout, "===================");
	fprintf (fout, "===========================================");
	fprintf (fout, "======");
	fprintf (fout, "============");
	fprintf (fout, "=========");
	fprintf (fout, "=========");
	fprintf (fout, "============");
	fprintf (fout, "============");
	fprintf (fout, "==============================\n");

	fprintf (fout, "|   ITEM  NUMBER   ");
	fprintf (fout, "|        D E S C R I P T I O N             ");
	fprintf (fout, "| UOM ");
	fprintf (fout, "| WAREHOUSE ");
	fprintf (fout, "| F/CAST ");
	fprintf (fout, "| F/CAST ");
	fprintf (fout, "|  FORECAST ");
	fprintf (fout, "|  FORECAST ");
	fprintf (fout, "|         EXCEPTION          |\n");

	fprintf (fout, "|                  ");
	fprintf (fout, "|                                          ");
	fprintf (fout, "|     ");
	fprintf (fout, "|           ");
	fprintf (fout, "| OPTION ");
	fprintf (fout, "| METHOD ");
	fprintf (fout, "| WKS DEMAND");
	fprintf (fout, "| MTH DEMAND");
	fprintf (fout, "|DESCRIPTION |  PC  |  SDev. |\n");

	fprintf (fout, "|------------------");
	fprintf (fout, "|------------------------------------------");
	fprintf (fout, "|-----");
	fprintf (fout, "|-----------");
	fprintf (fout, "|--------");
	fprintf (fout, "|--------");
	fprintf (fout, "|-----------");
	fprintf (fout, "|-----------");
	fprintf (fout, "|------------|------|--------|\n");
	fflush (fout);
}

int
check_page (void)
{
	return (EXIT_SUCCESS);
}

void
Process (void)
{
	char	currentGroup [13];

	LoadHhcc ();

	if (byCompany)
	{
		abc_selfield ("ccmr", "ccmr_id_no");
		for (curr_ccmr_ix = 0; curr_ccmr_ix < MAX_CCMR && 
		   	whouse[curr_ccmr_ix].hhcc_hash != 0L; curr_ccmr_ix++)
		{
			if (whouse [curr_ccmr_ix].hhcc_hash == commHhccHash)
				break;
		}
	}
	else
		abc_selfield ("ccmr", "ccmr_hhcc_hash");

	strcpy (inmr_rec.co_no, comm_rec .co_no);
	sprintf (inmr_rec.inmr_class, "%-1.1s", lower);
	sprintf (inmr_rec.category, "%-11.11s", lower + 1);
	sprintf (inmr_rec.item_no, "%-16.16s", " ");
	cc = find_rec ("inmr", &inmr_rec, GTEQ, "r");

	while (!cc && !strcmp (inmr_rec.co_no, comm_rec .co_no))
	{
		sprintf (currentGroup, "%-1.1s%-11.11s", 
					inmr_rec.inmr_class, inmr_rec.category);

		if (strncmp (currentGroup, upper, 12) > 0)
			break;

		if (inmr_rec.inmr_class[0] != 'N' && inmr_rec.inmr_class[0] != 'Z')
		{
			if (byCompany)
				ForecastCompany (inmr_rec.hhbr_hash);
			else
				Forecast (inmr_rec.hhbr_hash);
		}
		cc = find_rec ("inmr", &inmr_rec, NEXT, "r");
	}
}

/*===============================================
| Load hhcc_hash table with valid warehouses	|
===============================================*/
void
LoadHhcc (void)
{
	int		indx;

	for (indx = 0; indx < MAX_CCMR; indx++)
	{
		whouse[indx].hhcc_hash = 0L;
		sprintf (whouse[indx].br_no, "%-2.2s", " ");
		sprintf (whouse[indx].wh_no, "%-2.2s", " ");
		sprintf (whouse[indx].wh_name, "%-10.10s", " ");
	}

	indx = 0;

	strcpy (esmr_rec.co_no, comm_rec .co_no);
	strcpy (ccmr_rec.co_no, comm_rec .co_no);
	strcpy (ccmr_rec.est_no, (byCompany)  ? "  " : comm_rec .est_no);
	strcpy (ccmr_rec.cc_no, (byWarehouse) ? comm_rec .cc_no : "  ");
	cc = find_rec ("ccmr",&ccmr_rec, GTEQ, "r");
	while (!cc && indx < MAX_CCMR && 
		   !strcmp (ccmr_rec.co_no, comm_rec .co_no))
	{
		if (ccmr_rec.lrp_ok[0] == 'N')
		{
			cc = find_rec ("ccmr", &ccmr_rec, NEXT, "r");
			continue;
		}
		if (strcmp (ccmr_rec.co_no, esmr_rec.co_no) ||
		    strcmp (ccmr_rec.est_no, esmr_rec.est_no))
		{
			strcpy (esmr_rec.co_no, ccmr_rec.co_no);
			strcpy (esmr_rec.est_no, ccmr_rec.est_no);
			cc = find_rec (esmr, &esmr_rec, EQUAL, "r");
			if (cc)
			{
				cc = find_rec ("ccmr", &ccmr_rec, NEXT, "r");
				continue;
			}
		}
		if (byBranch)
		{
			if (strcmp (ccmr_rec.est_no, comm_rec .est_no))
			{
				cc = find_rec ("ccmr", &ccmr_rec, NEXT, "r");
				continue;
			}
		}
		if (byWarehouse)
		{
			if (strcmp (ccmr_rec.est_no, comm_rec .est_no) ||
			    strcmp (ccmr_rec.cc_no, comm_rec .cc_no))
			{
				cc = find_rec ("ccmr", &ccmr_rec, NEXT, "r");
				continue;
			}
		}
		whouse[indx].hhcc_hash = ccmr_rec.hhcc_hash;
		sprintf (whouse[indx].br_no, "%-2.2s", ccmr_rec.est_no);
		whouse[indx].br_inv_date = (CO_INV) ? comm_rec .inv_date 
					            		    : esmr_rec.inv_date;
		sprintf (whouse[indx].wh_no, "%-2.2s", ccmr_rec.cc_no);
		sprintf (whouse[indx++].wh_name, "%-10.10s", ccmr_rec.acronym);

		cc = find_rec ("ccmr", &ccmr_rec, NEXT, "r");
	}
	abc_selfield ("ccmr", "ccmr_hhcc_hash");
}
void
ForecastCompany (
 long    hhbrHash)
{
	char	abcCode[2];

	strcpy (abcCode, inmr_rec.abc_code);

	if (strchr (abcCodes, abcCode[0]) == (char *) 0)
		return;

	setup_LSA (0, comm_rec .co_no, "  ", "  ");

	incc_rec.hhcc_hash = commHhccHash;
	incc_rec.hhbr_hash = hhbrHash;
	cc = find_rec ("incc", &incc_rec, COMPARISON, "u");
	if (cc) 
		return;
	
	/*---------------------------
	| Item Exists in Warehouse	|
	---------------------------*/
	leadTime = incc_rec.safety_stock + 
				GetLeadTime 
				(
					hhbrHash, 
					comm_rec .est_no,
					comm_rec .cc_no
				);
	calc_LSA 
	(
		validMethods,
		hhbrHash,
		whouse[curr_ccmr_ix].br_inv_date,
		TRUE,
		HistoryMonths,
		LRP_PASSED_MONTH,
		DemandInclude
	);
	CalcMethods (curr_ccmr_ix);
}

void
Forecast (
 long   hhbrHash)
{
	char	abcCode[2];

	firstPartNo = TRUE;

	for (curr_ccmr_ix = 0; curr_ccmr_ix < MAX_CCMR && 
		   whouse[curr_ccmr_ix].hhcc_hash != 0L; curr_ccmr_ix++)
	{
		firstWarehouse = TRUE;
		LSA_vld_cc[0] = whouse[curr_ccmr_ix].hhcc_hash;
		LSA_vld_cc[1] = 0L;
		incc_rec.hhcc_hash = whouse[curr_ccmr_ix].hhcc_hash;
		incc_rec.hhbr_hash = hhbrHash;
		cc = find_rec ("incc", &incc_rec, COMPARISON, "u");
		if (cc) 
			continue;

		strcpy (abcCode, inmr_rec.abc_code);

		if (byBranch)
		{
			inei_rec.hhbr_hash = hhbrHash;
			strcpy (inei_rec.est_no, comm_rec .est_no);
			cc = find_rec (inei, &inei_rec, EQUAL, "r");
			if (!cc)
				strcpy (abcCode, inei_rec.abc_code);
		}
		if (byWarehouse)
			strcpy (abcCode, incc_rec.abc_code);

		if (strchr (abcCodes, abcCode[0]) == (char *) 0)
			continue;

		/*---------------------------
		| Item Exists in Warehouse	|
		---------------------------*/
		leadTime = incc_rec.safety_stock + 
			        GetLeadTime 
					 (
						hhbrHash, 
						whouse[curr_ccmr_ix].br_no,
						whouse[curr_ccmr_ix].wh_no 
					);

		 calc_LSA 
		 (
			validMethods,
			hhbrHash,
			comm_rec .inv_date,
			TRUE,
			HistoryMonths,
			LRP_PASSED_MONTH,
			DemandInclude
		);
		CalcMethods (curr_ccmr_ix);
	}
}

/*----------------------------------
| Determine whether the demand for |
| this product is erratic or not.  |
----------------------------------*/
int
CalculateErratic (
 int    forecast)
{
	/*-------------------------------------------------------
	| If standard error (forecast) * LRP_ERRATIC is greater |
	| than average then demand is considered erratic        |
	| User can input 1 - .5 so this will allow 10-5         |
	-------------------------------------------------------*/
	if (store [forecast].f_sqr_err > (errorThreshold * 10)) 
	{
		return (TRUE); 
	}

	return (FALSE);
}

/*----------------------------------------------
| Determine whether the demand for this        |
| product is outside percentage error allowed. |
----------------------------------------------*/
int
CalcPercentError (
 int    forecast)
{
	float	percentError	=	0.00;

	if (LSA_last3 [LSA_ACT] != 0.00)
	{
		percentError	=	LSA_last3 [forecast];
		percentError	-=	LSA_last3 [LSA_ACT];
		percentError	/=	LSA_last3 [LSA_ACT];
		percentError	*=	100;
		percentError	=	fabs (percentError);
	}
	else
		return (TRUE);

	/*-------------------------------------------------------
	| If standard error (forecast) * LRP_ERRATIC is greater  |
	| than average then demand is considered erratic        |
	-------------------------------------------------------*/
	if (percentError > LSA_percentError)
		return (TRUE); 

	return (FALSE);
}

void
CalcMethods (
 int    indx)
{
	char	*method;
	int		numMethods	=	0;
 	int		erratic		=	0;
 	int		overPercent	=	0;
	int		forecast 	=	-1;
	int		i;

	ClearMethods ();

	/*---------------------------------------
	| Calculate Forecast for Every Method	|
	---------------------------------------*/
	for (method = validMethods, numMethods = 1; strlen (method); method++)
	{
		/*---------------------------
		| Set index for method used	|
		---------------------------*/
		switch (incc_rec.ff_option[0])
		{
		case	'M':
			forecast = 0;
			break;

		case	'P':
			if (incc_rec.ff_method[0] == *method)
				forecast = numMethods;
			break;

		case	'A':
		default:
			break;
		}
		sprintf (store[numMethods].f_method, "%-1.1s", method);

		switch (*method)
		{
		/*-------------------------------
		| Standard Least Squares linear	|
		-------------------------------*/
		case	'A':
			if (ValidMethods ())
				Method_A (numMethods++);
			break;

		/*-------------------------------
		| Seasonal trend variant to LSA	|
		-------------------------------*/
		case	'B':
			if (ValidMethods ())
				Method_B (numMethods++);
			break;

		/*-------------------------------
		| Local Linear variant of   LSA	|
		-------------------------------*/
		case	'C':
			if (ValidMethods ())
				Method_C (numMethods++);
			break;

		/*--------------------
		| Focus Forecasting. |
		--------------------*/
		case	'D':
			if (ValidMethods ())
				Method_D (numMethods++);
			break;

		default:
			break;
		}
	}
	if (incc_rec.ff_option[0] == 'A')
	{
		/*---------------------------------
		| If demand is erratic, forecast  |
		| method E with no demand		  |
		---------------------------------*/
		if (!LSA_hist || zero_hist)
			forecast = METH_D;
		else
		{
			/*-------------------------------------------------------------
			| If biasFlag is true then we should use smallest deviation   |
			| rather than smallest percentage first.                      |
			| If biasFlag is false then we should use smallest percentage |
			| rather than smallest deviation first.                       |
			|                                                             |
			| If BestMethod returns -1 then no suitable method found.     |
			|                                                             |
			-------------------------------------------------------------*/
			forecast = BestMethod (numMethods, (biasFlag) ? FALSE : TRUE);
			if (forecast > 0)
				forecast = BestMethod (numMethods, (biasFlag) ? TRUE  : FALSE);

			if (forecast < 0)
				forecast = METH_D;

			erratic		= CalculateErratic (forecast);
			overPercent	= CalcPercentError (forecast);
			if (erratic || overPercent)
				forecast = METH_D;
		}
	}
	PrintMethod 
	(
		numMethods, 
		forecast, 
		indx, 
		erratic, 
		overPercent
	);

	if (updateData)
	{
		int		newLrph;

		memset (&lrph_rec, 0, sizeof (lrph_rec));

		lrph_rec.hhwh_hash	=	incc_rec.hhwh_hash;
		newLrph	=	find_rec (lrph, &lrph_rec, COMPARISON, "u");

		for (i = (incc_rec.ff_option[0] == 'M') ? 0 : 1;i < numMethods;i++)
		{
	    	if (store [i].f_method_no == forecast)
			{
				strcpy (lrph_rec.prev_sel, lrph_rec.curr_sel);
				strcpy (lrph_rec.curr_sel, store [i].f_method);
				UpdateIncc (i);
			}
			if (store[i].f_method_no == METH_A)
			{
				lrph_rec.a_actual		=	store [i].f_actual		/	3;
				lrph_rec.a_forecast		=	store [i].f_forecast	/	3;
				lrph_rec.a_wks_dem		=	store [i].f_wks_demand;
				lrph_rec.a_sqr_err		=	store [i].f_sqr_err;
				lrph_rec.a_pc_err		=	store [i].f_pc_err;
			}
			if (store[i].f_method_no == METH_B)
			{
				lrph_rec.b_actual		=	store [i].f_actual		/	3;
				lrph_rec.b_forecast		=	store [i].f_forecast	/	3;
				lrph_rec.b_wks_dem		=	store [i].f_wks_demand;
				lrph_rec.b_sqr_err		=	store [i].f_sqr_err;
				lrph_rec.b_pc_err		=	store [i].f_pc_err;
			}
			if (store [i].f_method_no == METH_C)
			{
				lrph_rec.c_actual		=	store [i].f_actual		/	3;
				lrph_rec.c_forecast		=	store [i].f_forecast	/	3;
				lrph_rec.c_wks_dem		=	store [i].f_wks_demand;
				lrph_rec.c_sqr_err		=	store [i].f_sqr_err;
				lrph_rec.c_pc_err		=	store [i].f_pc_err;
			}
			if (store [i].f_method_no == METH_D)
			{
				lrph_rec.d_actual		=	store [i].f_actual		/	3;
				lrph_rec.d_forecast		=	store [i].f_forecast	/	3;
				lrph_rec.d_wks_dem		=	store [i].f_wks_demand;
				lrph_rec.d_sqr_err		=	store [i].f_sqr_err;
				lrph_rec.d_pc_err		=	store [i].f_pc_err;
			}
		}
		if (newLrph)
			cc = abc_add (lrph, &lrph_rec);
		else
			cc = abc_update (lrph, &lrph_rec);
		if (cc)
			file_err (cc, "lrph", "DBADD/DBUPDATE");
	}
}

void
ClearMethods (void)
{
	int		i;
	
	for (i = 0;i < 6;i++)
	{
		strcpy (store [i].f_method," ");
		store [i].f_method_no 	= 0;
		store [i].f_actual 		= 0.00;
		store [i].f_forecast 	= 0.00;
		store [i].f_wks_demand 	= 0.00;
		store [i].f_sqr_err 	= 0.00;
		store [i].f_pc_err 		= 0.00;
	}
}

int
ValidMethods (void)
{
	/*-----------------------------------
	| Printing only manual option items	|
	-----------------------------------*/
	if (manual[0] != 'A' && incc_rec.ff_option[0] == 'M')
		return (TRUE);

	if (manual[0] != 'M' && incc_rec.ff_option[0] != 'M')
		return (TRUE);

	/*-----------------------------------
	| Only Printing Manual Option Items	|
	-----------------------------------*/
	if (manual[0] == 'M')
		return (FALSE);

	return (TRUE);
}

/*===============================
| Decide which Method to Use	|
===============================*/
int
BestMethod (
 int    numMethods,
 int	square)
{
	int		i;
	int		forecast 	= -1;
	double	minErr 		= -1;

	for (i = 1;i < numMethods;i++)
	{
		if (strchr (LSA_methods, store[i].f_method[0]) == (char *) 0)
			continue;

		if (store[i].f_sqr_err == 99.0)
			continue;

		if (minErr < 0)
		{
			minErr 		= (square) ? store[i].f_sqr_err : store[i].f_pc_err;
			forecast 	= store[i].f_method_no;
		}

		/*--------------------------------------------------------------------
		| Standard error for this method must be more than 5 percent better  |
		| than earlier method to replace forecast method. Without this 5     |
		| percent condition random variations may be interpreted as seasonal |
		| or local linear may be incorrectly favoured over seasonal          |
		--------------------------------------------------------------------*/
		if (minErr > ((square) ? store[i].f_sqr_err * 1.5 : store[i].f_pc_err))
		{
			forecast 	= store[i].f_method_no;
			minErr 		= (square) ? store[i].f_sqr_err: store[i].f_pc_err;
		}
	}
	return (forecast);
}

void
UpdateIncc (
 int    forecast)
{
	if (store[forecast].f_method[0] == ' ')
		return;

	switch (incc_rec.ff_option[0])
	{
	/*-----------
	| Automatic	|
	-----------*/
	case	'A':
		strcpy (incc_rec.ff_method, store[forecast].f_method);
		incc_rec.pwks_demand	= incc_rec.wks_demand;
		incc_rec.wks_demand	= store[forecast].f_wks_demand;
		cc = abc_update ("incc", &incc_rec);
		if (cc)
			file_err (cc, "incc", "DBUPDATE");
		break;

	/*-----------
	| Manual	|
	-----------*/
	case	'M':
		break;

	/*---------------
	| Predetermined	|
	---------------*/
	case	'P':
		strcpy (incc_rec.ff_method, store[forecast].f_method);
		incc_rec.pwks_demand 	= incc_rec.wks_demand;
		incc_rec.wks_demand 		= store[forecast].f_wks_demand;
		cc = abc_update ("incc", &incc_rec);
		if (cc)
			file_err (cc, "incc", "DBUPDATE");
		break;

	default:
		break;
	}
}

/*=======================
| Straight LSA only.	|
=======================*/
void
Method_A (
 int    indx)
{
	store[indx].f_actual 	= LSA_last3[0];
	store[indx].f_sqr_err 	= 99.00;
	store[indx].f_method_no = METH_A;
	if (strchr (LSA_methods, 'A') != (char *) 0)
	{
		store[indx].f_forecast 	= LSA_last3[METH_A];
		CalculateDemand (indx, METH_A);
		store[indx].f_sqr_err 	= LSA_error[METH_A][1];
		store[indx].f_pc_err 	= fabs (LSA_error[METH_A][0]);
	}
}

/*===========================
| Seaonal demand on LSA.	|
===========================*/
void
Method_B (
 int    indx)
{
	store[indx].f_sqr_err 	= 99.00;
	store[indx].f_actual = LSA_last3[0];
	store[indx].f_method_no = METH_B;
	if (strchr (LSA_methods, 'B') != (char *) 0)
	{
		store[indx].f_forecast 	= LSA_last3[METH_B];
		CalculateDemand (indx, METH_B);
		store[indx].f_sqr_err 	= LSA_error[METH_B][1];
		store[indx].f_pc_err 	= fabs (LSA_error[METH_B][0]);
	}
}

/*=======================
| Seaonal trend on LSA.	|
=======================*/
void
Method_C (
 int    indx)
{
	store[indx].f_sqr_err 	= 99.00;
	store[indx].f_actual = LSA_last3[0];
	store[indx].f_method_no = METH_C;
	if (strchr (LSA_methods, 'C') != (char *) 0)
	{
		store[indx].f_forecast = LSA_last3[METH_C];
		CalculateDemand (indx, METH_C);
		store[indx].f_sqr_err 	= LSA_error[METH_C][1];
		store[indx].f_pc_err 	= fabs (LSA_error[METH_C][0]);
	}
}

/*===================
| User 'FF' values.	|
===================*/
void
Method_D (
 int    indx)
{
	store[indx].f_sqr_err 	= 99.00;
	store[indx].f_actual = LSA_last3[0];
	store[indx].f_method_no = METH_D;
	if (strchr (LSA_methods, 'D') != (char *) 0)
	{
		store[indx].f_forecast = LSA_last3[METH_D];
		CalculateDemand (indx, METH_D);
		store[indx].f_sqr_err 	= LSA_error[METH_D][1];
		store[indx].f_pc_err 	= fabs (LSA_error[METH_D][0]);
	}
}

void
CalculateDemand (
 int    indx,
 int    method)
{
	double	demand	=	0.00;

	demand 	= LSA_result [method][36]; 
	demand 	/= 4.348;

	store[indx].f_wks_demand = demand;

	/*------------------------------------------
	| Set demand to zero if demand is negative |
	| and negative demand is not allowed.      |
	------------------------------------------*/
	if (!negativeDemandOk && store[indx].f_wks_demand < 0.00)
		store[indx].f_wks_demand = 0.00;
}

/*===============================
| Print details for ALL methods	|
| - flag indicates method used	|
===============================*/
void
PrintMethod (
 int    numMethods,
 int    forecast,
 int    indx,
 int    erratic,
 int    overPercent)
{
	int		i;

	for (i = (incc_rec.ff_option[0] == 'M') ? 0 : 1;i < numMethods;i++)
	{
	    /*-----------------------------------
	    | Printing Selected Options Only	|
	    | ie	Manual						|
	    |	Automatic	- best Method		|
	    |	Predetermined	- Method		|
	    -----------------------------------*/
	    if (selected[0] != 'A' && store [i].f_method_no != forecast)
			continue;

	    /*-------------------------------------------
	    | First Method in First Warehouse for Item	|
	    -------------------------------------------*/
	    if (firstPartNo == TRUE || selected[0] != 'A')
	    {
			dsp_process ("Item No : ",inmr_rec.item_no);
			if (printReport)
			{
				fprintf (fout, "| %16.16s ", inmr_rec.item_no);
				fprintf (fout, "| %40.40s ", inmr_rec.description);
				fprintf (fout, "|%4.4s ", 	 inmr_rec.sale_unit);
			}
			firstPartNo = FALSE;
	    }
	    else
	    {
			if (printReport)
			{
				fprintf (fout, "| %16.16s ", " ");
				fprintf (fout, "| %40.40s ", " ");
				fprintf (fout, "|%4.4s ", 	 " ");
			}
	    }

	    if (printReport)
	    {
			/*---------------------------
			| First Warehouse for Item	|
			---------------------------*/
			fprintf (fout, "| %9.9s ", whouse [indx].wh_name);

			firstWarehouse = FALSE;

			fprintf (fout, "|    %s   ", 	incc_rec.ff_option);
			fprintf (fout, "|  %s  %s  ", 	
							store [i].f_method,
							 (selected[0] == 'A' && store [i].f_method_no == forecast) ? "*" : " ");
			fprintf (fout, "|%10.2f ", store [i].f_wks_demand);
			fprintf (fout, "|%10.2f ", store [i].f_wks_demand * 4.348);
			if (overPercent && LSA_hist && !zero_hist)
			{
	    		if (store [i].f_method_no == forecast)
					fprintf (fout, "|Over Percent|%6.2f|%8.2f|\n",
									store [i].f_pc_err, store [i].f_sqr_err);
				else
					fprintf (fout, "|            |      |        |\n");
			}
			else if (erratic && LSA_hist && !zero_hist)
			{
	    		if (store [i].f_method_no == forecast)
					fprintf (fout, "| Erratic    |%6.2f|%8.2f|\n",
									store [i].f_pc_err, store [i].f_sqr_err);
				else
					fprintf (fout, "|            |      |        |\n");
			}
			else
			{
				if (strchr (LSA_methods, store[i].f_method[0]) == (char *)0)
					fprintf (fout, "|Insuf Hist. |      |        |\n");
				else
					if (zero_hist)
						fprintf (fout, "|Zero Demand |      |        |\n");
					else
						fprintf (fout, "|            |      |        |\n");
			}
	    }
	}
}

float	
GetLeadTime (
 long    hhbrHash,
	char	*br_no,
	char	*wh_no)
{
	float	weeks = 0.00;

	inis_rec.hhbr_hash = hhbrHash;
	strcpy (inis_rec.sup_priority, "W1");
	strcpy (inis_rec.co_no, comm_rec .co_no);
	strcpy (inis_rec.br_no, br_no);
	strcpy (inis_rec.wh_no, wh_no);
	cc = find_rec ("inis", &inis_rec, COMPARISON, "r");
	if (cc)
	{
		inis_rec.hhbr_hash = hhbrHash;
		strcpy (inis_rec.sup_priority, "B1");
		strcpy (inis_rec.co_no, comm_rec .co_no);
		strcpy (inis_rec.br_no, br_no);
		strcpy (inis_rec.wh_no, "  ");
		cc = find_rec ("inis", &inis_rec, COMPARISON, "r");
		if (cc)
		{
			inis_rec.hhbr_hash = hhbrHash;
			strcpy (inis_rec.sup_priority, "C1");
			strcpy (inis_rec.co_no, comm_rec .co_no);
			strcpy (inis_rec.br_no, "  ");
			strcpy (inis_rec.wh_no, "  ");
			cc = find_rec ("inis", &inis_rec, COMPARISON, "r");
		}
	}

	if (!cc)
	{
	    if (inis_rec.lead_time == 0.00)
			weeks = GetLeadDate (inis_rec.hhis_hash, comm_rec .inv_date);
		else
			weeks = inis_rec.lead_time / 7.00;
	}

	/*---------------------------------------
	| Find out what the review-period	    |
	| is for this product. Firstly, try for	|
	| a match on branch/item. Then try for	|
	| a match on item. Then try for a match	|
	| on branch/category. If this fails,	|
	| then use LRP_DFLT_REVIEW environment-	|
	| value. If not found, dflt to 4 weeks.	|
	---------------------------------------*/
	ffpr_rec.hhbr_hash = hhbrHash;
	strcpy (ffpr_rec.br_no, br_no);
	cc = find_rec (ffpr, &ffpr_rec, EQUAL, "r");
	if (cc)
	{
	    strcpy (ffpr_rec.br_no, "  ");
	    cc = find_rec (ffpr, &ffpr_rec, EQUAL, "r");
	    if (cc)
	    {
			abc_selfield (ffpr, "ffpr_id_no_1");
			strcpy (ffpr_rec.category, inmr_rec.category);
			strcpy (ffpr_rec.br_no, br_no);
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
	weeks += ffpr_rec.review_prd;
	return (weeks);
}

/*===============================================
| Get the number of weeks between 'date' & the	|
| next available inld_sup_date.			        |
| Return 0 if none found.			            |
===============================================*/
float	
GetLeadDate (
 long    hash, 
 long    date)
{
	float	weeks;

	inld_rec.hhis_hash = hash;
	inld_rec.ord_date = date;

	cc = find_rec ("inld", &inld_rec, GTEQ, "r");
	if (cc)
		return ((float) 0.00);

	weeks = (inld_rec.sup_date - date) / 7;
	return (weeks);
}

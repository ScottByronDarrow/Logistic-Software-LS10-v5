/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_close.c,v 5.3 2001/08/09 09:18:15 scott Exp $
|  Program Name  : (sk_close.c)
|  Program Desc  : (Close Inventory Master & Warehouse Files)
|---------------------------------------------------------------------|
|  Date Written  : (BP/MM/YYYY)    | Author      : Scott B Darrow.    |
|---------------------------------------------------------------------|
| $Log: sk_close.c,v $
| Revision 5.3  2001/08/09 09:18:15  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:44:45  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:18:57  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_close.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_close/sk_close.c,v 5.3 2001/08/09 09:18:15 scott Exp $";

#include	<pslscr.h>
#include	<proc_sobg.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<Costing.h>

#include 	"schema"

#define	CO_INV		 (envVarCoClose [2] == '1')
#define CO_DBT		 (envVarCoClose [0] == '1')

	struct	ccmrRecord	ccmr_rec;
	struct	inmrRecord	inmr_rec;
	struct	inmuRecord	inmu_rec;
	struct	ffdmRecord	ffdm_rec;
	struct	inmbRecord	inmb_rec;
	struct	inwuRecord	inwu_rec;
	struct	inmeRecord	inme_rec;
	struct	inmlRecord	inml_rec;
	struct	inccRecord	incc_rec;
	struct	commRecord	comm_rec;

	float 	*incc_con 		=	&incc_rec.c_1;
	Money 	*incc_val		=	&incc_rec.c_val_1;
	Money 	*incc_prf		=	&incc_rec.c_prf_1;


	char	envVarCoClose [6];
	char	strSysDate [11];
	char	processMonth [3];

	long	longSysDate = 0L;
	int		mendFile		=	0,
			mth				=	0,
			yearEnd			=	0,
			tmth			=	0,
			envVarSaYend 	=	0,
			allBranches 	=	FALSE,
			envSkMendBalLog = 	FALSE,
			printAudit		=	FALSE,
			envVarMendLp	=	0;

	FILE	*fout;

/*=======================
| Function Declarations |
=======================*/
void 	OpenDB 			 	(void);
void 	CloseDB 			(void);
void 	ProcessByCompany 	(void);
void 	ProcessByBranch 	(void);
void 	ProcessInmu		 	(void);
void 	UpdateIncc 		 	(void);
void 	UpdateInmb 		 	(void);
void 	AddFfdm 			(float, long, long);
void 	PrintCcmrError 	 	(int, long, long);
void 	OpenAudit 			(void);
void 	CloseAudit 		 	(void);


/*==========================
| Main processing Routine. |
==========================*/
int
main (
 int argc,
 char * argv [])
{
	int		monthNum;
	char	*sptr;

	sptr = chk_env ("SK_MEND_BAL_LOG");
	envSkMendBalLog = (sptr == (char *)0) ? FALSE : atoi (sptr);

	sptr = chk_env ("SA_YEND");
	if (!sptr)
		envVarSaYend = comm_rec.fiscal;
	else
		envVarSaYend = atoi (sptr);

	if (envVarSaYend < 1 || envVarSaYend > 12)
		envVarSaYend = comm_rec.fiscal;

	if ((sptr = chk_env ("MEND_LP")) && isdigit (*sptr))
	{
		printAudit = TRUE;
		envVarMendLp = atoi (sptr);
	}
	else
		printAudit = FALSE;

	sptr = chk_env ("CO_CLOSE");
	if (sptr == (char *)0)
		sprintf (envVarCoClose,"%-5.5s","11111");
	else
		sprintf (envVarCoClose,"%-5.5s",sptr);

	strcpy (strSysDate, DateToString (TodaysDate ()));
	longSysDate = TodaysDate ();

	init_scr ();
	clear ();

	OpenDB ();

	if (CO_INV)
		dsp_screen ("Inventory Close By Company.",
					comm_rec.co_no,comm_rec.co_name);
	else
	{
		sprintf (err_str, "Running Inventory Close for %s - %s",
				comm_rec.est_no, clip (comm_rec.est_name)); 

		dsp_screen (err_str, comm_rec.co_no, comm_rec.co_name);
	}

	DateToDMY (comm_rec.inv_date, NULL, &monthNum, NULL);
	mth = tmth = monthNum; 

	if (mth == 12)
		mth = 0;

	if (CO_DBT)
		allBranches = TRUE;
	else
		allBranches = FALSE;

	yearEnd = (tmth == envVarSaYend) ? TRUE : FALSE;
	tmth++;
	if (tmth == 13)
		tmth = 1;

	sprintf (processMonth,"%02d",tmth);
	
	if (printAudit)
		OpenAudit ();

	if (CO_INV)
		ProcessByCompany ();
	else
		ProcessByBranch ();

	ProcessInmu ();

	if (printAudit)
		CloseAudit ();

	CloseDB (); 
	FinishProgram ();
    return (EXIT_SUCCESS);
}

void
OpenDB (
 void)
{
	abc_dbopen ("data");

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (incc, incc_list, INCC_NO_FIELDS, 
								(CO_INV) ? "incc_hhbr_hash" : "incc_id_no_2");
	open_rec (ccmr , ccmr_list, CCMR_NO_FIELDS, 
								(CO_INV) ? "ccmr_hhcc_hash" : "ccmr_id_no");
	open_rec (inme, inme_list, INME_NO_FIELDS, "inme_hhwh_hash");
	open_rec (inwu, inwu_list, INWU_NO_FIELDS, "inwu_id_no");
	open_rec (inml, inml_list, INML_NO_FIELDS, "inml_id_no");
	open_rec (inmb, inmb_list, INMB_NO_FIELDS, "inmb_id_no");
	open_rec (ffdm, ffdm_list, FFDM_NO_FIELDS, "ffdm_id_no2");
	open_rec (inmu, inmu_list, INMU_NO_FIELDS, "inmu_id_no2");
}

void
CloseDB (
 void)
{
	abc_fclose (incc);
	abc_fclose (ccmr);
	abc_fclose (inmu);
	abc_fclose (inme);
	abc_fclose (inmr);
	abc_fclose (inwu);
	abc_fclose (inml);
	abc_fclose (inmb);
	abc_fclose (ffdm);
	CloseCosting ();
	abc_dbclose ("data");
}

void
ProcessByCompany (
 void)
{
	/*--------------------------------
	| Process Inventory Master File. |
	--------------------------------*/
	strcpy (inmr_rec.co_no,comm_rec.co_no);
	strcpy (inmr_rec.item_no,"                ");
	cc = find_rec (inmr, &inmr_rec, GTEQ, "r");
	while (!cc && !strcmp (inmr_rec.co_no,comm_rec.co_no))
	{
		dsp_process ("Item : ", inmr_rec.item_no);

		incc_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
		cc = find_rec (incc,&incc_rec,GTEQ,"u");
		while (!cc && incc_rec.hhbr_hash == inmr_rec.hhbr_hash)
		{
			if (CO_INV)
			{
				ccmr_rec.hhcc_hash	=	incc_rec.hhcc_hash;
				cc = find_rec (ccmr, &ccmr_rec, EQUAL, "r");
				if (cc)
				{
					abc_unlock (incc);
					PrintCcmrError 
					 (		
						cc,
						incc_rec.hhcc_hash,
						incc_rec.hhwh_hash
					);
					cc = find_rec (incc,&incc_rec,NEXT,"u");
					continue;
				}
			}
			UpdateIncc ();

			if (envSkMendBalLog)
				UpdateInmb ();
			abc_unlock (incc);
			cc = find_rec (incc,&incc_rec,NEXT,"u");
		}
		abc_unlock (incc);
		cc = find_rec (inmr, &inmr_rec, NEXT, "r");
	}
	recalc_sobg ();
}

void
ProcessByBranch (
 void)
{
	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, comm_rec.est_no);
	strcpy (ccmr_rec.cc_no, "  ");
	cc = find_rec (ccmr, &ccmr_rec, GTEQ, "r");
	while (!cc && !strcmp (ccmr_rec.co_no, comm_rec.co_no) &&
		       !strcmp (ccmr_rec.est_no, comm_rec.est_no))
	{
		incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
		strcpy (incc_rec.sort, "                            ");
		cc = find_rec (incc, &incc_rec, GTEQ, "u");
		while (!cc && incc_rec.hhcc_hash == ccmr_rec.hhcc_hash)
		{
			dsp_process ("Item : ", incc_rec.sort + 12);
			UpdateIncc ();

			if (envSkMendBalLog)
				UpdateInmb ();
			
			cc = find_rec (incc, &incc_rec, NEXT, "u");
		}
		abc_unlock (incc);
		cc = find_rec (ccmr, &ccmr_rec, NEXT, "r");
	}
	recalc_sobg ();
}

/*========================
| Update warehouse file. |
========================*/
void
UpdateIncc (
 void)
{
	float	Weeks = 0.00,
			UnservedQty = 0.00;

	inml_rec.hhwh_hash		=	incc_rec.hhwh_hash;
	inml_rec.mend_date		=	comm_rec.inv_date;
	cc = find_rec (inml, &inml_rec, COMPARISON, "r");
	if (cc)
	{
		inml_rec.opening_stock	=	incc_rec.opening_stock;
		inml_rec.receipts		=	incc_rec.receipts;
		inml_rec.pur			=	incc_rec.pur;
		inml_rec.issues			=	incc_rec.issues;
		inml_rec.adj			=	incc_rec.adj;
		inml_rec.sales			=	incc_rec.sales;
		inml_rec.stake			=	incc_rec.stake;
		inml_rec.closing_stock	=	incc_rec.closing_stock;
		cc	=	abc_add (inml, &inml_rec);
		if (cc)
			file_err (cc, "inml", "DBADD");
	}
	else
	{
		inml_rec.opening_stock	=	incc_rec.opening_stock;
		inml_rec.receipts		=	incc_rec.receipts;
		inml_rec.pur			=	incc_rec.pur;
		inml_rec.issues			=	incc_rec.issues;
		inml_rec.adj			=	incc_rec.adj;
		inml_rec.sales			=	incc_rec.sales;
		inml_rec.stake			=	incc_rec.stake;
		inml_rec.closing_stock	=	incc_rec.closing_stock;
		cc	=	abc_update (inml, &inml_rec);
		if (cc)
			file_err (cc, "inml", "DBADD");
	}
	/*-------------------------------------------------
	| set relevent values in current cost centre to 0 |
	-------------------------------------------------*/
	incc_rec.sales 			= 0.00;
	incc_rec.opening_stock 	= incc_rec.closing_stock;
	incc_rec.receipts 		= 0.00;
	incc_rec.pur 			= 0.00;
	incc_rec.issues 		= 0.00;
	incc_rec.adj 			= 0.00;

	incc_con [mth] = 0.00;
	incc_val [mth] = 0.00;
	incc_prf [mth] = 0.00;
		
	if (mth == comm_rec.fiscal)
	{
		incc_rec.ytd_receipts 	= 0.00;
		incc_rec.ytd_pur 		= 0.00;
		incc_rec.ytd_issues 	= 0.00;
		incc_rec.ytd_adj 		= 0.00;
		incc_rec.ytd_sales 		= 0.00;
	}

	/*----------------------------------------------------------
	| Look for month end file for current product / warehouse. |
	----------------------------------------------------------*/
	inme_rec.hhwh_hash = incc_rec.hhwh_hash;
	mendFile = !find_rec (inme, &inme_rec, EQUAL, "r");

	/*-----------------------------------------
	| Updated incc record with stored values. |
	-----------------------------------------*/
	if (mendFile)
	{
		incc_rec.receipts 		= inme_rec.receipts;
		incc_rec.issues 		= inme_rec.issues;
		incc_rec.adj 			= inme_rec.adj;
		incc_rec.pur 			= inme_rec.pur;
		incc_rec.sales 			= inme_rec.sales;
		incc_con [mth] 			= inme_rec.qty;
		incc_val [mth] 			= inme_rec.value;
		incc_prf [mth] 			= inme_rec.profit;

		incc_rec.ytd_receipts 	+= inme_rec.receipts;
		incc_rec.ytd_pur 		+= inme_rec.pur;
		incc_rec.ytd_issues 	+= inme_rec.issues;
		incc_rec.ytd_adj 		+= inme_rec.adj;
		incc_rec.ytd_sales 		+= inme_rec.sales;

		incc_rec.closing_stock = 	incc_rec.opening_stock 	+
					  				incc_rec.pur 			+
					  				incc_rec.receipts 		-
					  				incc_rec.issues 		-
					  				incc_rec.sales 			+
					  				incc_rec.adj;
	}

	/*----------------------------------------------------------------------
	| If the product now has stock on hand and the out of stock date > 0L  |
	| then add a record to ffdm providing the weeks demand for item is < 0 |
	----------------------------------------------------------------------*/
	if (incc_rec.closing_stock > 0.00 && incc_rec.os_date != 0L)
	{
		Weeks = (float) ((longSysDate - incc_rec.os_date) / 7.00);

		if (incc_rec.wks_demand > 0.00)
		{
			UnservedQty = Weeks * incc_rec.wks_demand;

			AddFfdm 
			 (
				UnservedQty,
				incc_rec.hhbr_hash,
				incc_rec.hhcc_hash
			);
		}
		incc_rec.os_ldate = 0L;
	}

	cc =  abc_update (incc, &incc_rec);
	if (cc)
		file_err (cc, incc, "DBUPDATE");

	/*------------------------
	| Delete month end file. |
	------------------------*/
	if (mendFile)
	{
		inme_rec.hhwh_hash = incc_rec.hhwh_hash;
		cc = find_rec (inme, &inme_rec, EQUAL, "r");
		if (!cc)
		{
			add_hash 
			 (	
				comm_rec.co_no,
				comm_rec.est_no,
				"RC", 
				0, 
				incc_rec.hhbr_hash, 
				incc_rec.hhcc_hash,
				0L,
				 (double) 0.00
			);

			cc = abc_delete (inme);
			if (cc)
				file_err (cc, inme, "DBDELETE");
		}
	}
	/*--------------------------------------
	| Find Warehouse unit of measure file. |
	--------------------------------------*/
	inwu_rec.hhwh_hash	=	incc_rec.hhwh_hash;
	inwu_rec.hhum_hash	=	0L;
	cc = find_rec (inwu, &inwu_rec, GTEQ,"u");
	while (!cc && inwu_rec.hhwh_hash == incc_rec.hhwh_hash)
	{
		inwu_rec.opening_stock 	= inwu_rec.closing_stock;
		inwu_rec.pur			= 0.00;
		inwu_rec.receipts		= 0.00;
		inwu_rec.adj			= 0.00;
		inwu_rec.issues			= 0.00;
		inwu_rec.sales			= 0.00;

		cc = abc_update (inwu,&inwu_rec);
		if (cc)
			file_err (cc, "inwu", "DBUPDATE");

		cc = find_rec (inwu, &inwu_rec, NEXT,"u");
	}
	abc_unlock (inwu);
}

void
UpdateInmb (
 void)
{
	struct	inmbRecord	inmb_rec2;

	double	lastFifoCost =	0.0;

	int		cc2;

	for (cc2 = FindInei (incc_rec.hhbr_hash, ccmr_rec.est_no, "r");
		cc2;
		cc2 = FindInei (incc_rec.hhbr_hash, ccmr_rec.est_no, "r"))
	{
		clear ();
		box (6, 8, 67, 4);
		print_at (10,8,"No Branch %2.2s Record set up for item %s",
										ineiRec.est_no,
										clip (inmr_rec.item_no));
		printf (err_str);

		fflush (stdout);

		putchar (BELL);
		PauseForKey (11, 8, "Please set up on another terminal and press <return> when ready", '\r');

		if (CO_INV)
			dsp_screen ("Inventory Close By Company.", comm_rec.co_no,
														comm_rec.co_name);
		else
		{
			sprintf (err_str, "Running Inventory Close for %s - %s",
					comm_rec.est_no, clip (comm_rec.est_name)); 
	
			dsp_screen (err_str, comm_rec.co_no, comm_rec.co_name);
		}
	}

	lastFifoCost =	0.0;
	if (inmr_rec.costing_flag [0] == 'F')
	{
		cc2 = FindIncf (incc_rec.hhwh_hash, TRUE, "r");
		while (!cc2 && incfRec.hhwh_hash == incc_rec.hhwh_hash)
		{
			lastFifoCost =	incfRec.fifo_cost;
			cc2 = FindIncf (0L, TRUE, "r");
		}
	}

	strcpy (inmb_rec.co_no, comm_rec.co_no);
	inmb_rec.hhbr_hash 		= incc_rec.hhbr_hash;
	inmb_rec.hhcc_hash 		= incc_rec.hhcc_hash;
	inmb_rec.date 			= (MonthEnd (comm_rec.inv_date) + 1);
	inmb_rec.opening_bal 	= incc_rec.opening_stock;
	inmb_rec.avge_cost 		= CENTS (ineiRec.avge_cost);
	inmb_rec.prev_cost 		= CENTS (ineiRec.prev_cost);
	inmb_rec.last_cost 		= CENTS (ineiRec.last_cost);
	inmb_rec.std_cost 		= CENTS (ineiRec.std_cost);
	inmb_rec.latest_fifo 	= CENTS (lastFifoCost);
	strcpy (inmb_rec.insuf_trx, "N");
	inmb_rec2 				= inmb_rec;
	cc2 = find_rec (inmb, &inmb_rec2, EQUAL, "r");
	cc2 = cc2 ? abc_add (inmb, &inmb_rec) : abc_update (inmb, &inmb_rec);
	if (cc2)
		file_err (cc2, inmb, "DBADD");

	return;
}

/*===========================
| Add record to ffdm table  |
===========================*/
void
AddFfdm (
 float	unsrv,
 long	hhbrHash,
 long	hhccHash)
{
	memset (&ffdm_rec, 0, sizeof (ffdm_rec));

	ffdm_rec.hhbr_hash = hhbrHash;
	ffdm_rec.hhcc_hash = hhccHash;
	ffdm_rec.date 	   = TodaysDate ();
	strcpy (ffdm_rec.type, "3");
	cc = find_rec (ffdm, &ffdm_rec, COMPARISON, "u");
	if (!cc)
	{
		ffdm_rec.qty += unsrv;

		cc = abc_update (ffdm, &ffdm_rec);
		if (cc)
            file_err (cc, "ffdm", "DBUPDATE");
    }
    else
    {
		ffdm_rec.qty = unsrv;
        cc = abc_add (ffdm, &ffdm_rec);
        if (cc)
            file_err (cc, "ffdm", "DBADD");
    }
}
void
PrintCcmrError (
 int    cc,
 long   ccmr_hhcc_hash,
 long   incc_hhwh_hash)
{
	char errorText [121];
	sprintf (errorText,
			 "Error %d finding ccmr ccmr_hhcc_hash = %ld, "
			 "for incc incc_hhwh_hash %ld",
			 cc,
			 ccmr_hhcc_hash,
			 incc_hhwh_hash);
	fprintf (fout, "| %-120.120s |\n", errorText);
}

void
OpenAudit (
 void)
{
	if (! (fout = popen ("pformat","w"))) 
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);

	fprintf (fout, ".START%s<%s>\n", DateToString (comm_rec.inv_date), PNAME);
	fprintf (fout, ".LP%d\n", envVarMendLp);
	fprintf (fout, ".9\n");
	fprintf (fout, ".L124\n");
	fprintf (fout, ".EINVENTORY CLOSE ERRORS AUDIT REPORT\n");
	fprintf (fout, ".E For Company %s\n", comm_rec.co_name);
	fprintf (fout, ".B2\n");

	fprintf (fout,
			 ".R----------------------------------------"
			 "------------------------------------------"
			 "------------------------------------------\n");

	fprintf (fout,
			 "----------------------------------------"
			 "------------------------------------------"
			 "------------------------------------------\n");

	fprintf (fout,
			 "| Description of error                  "
			 "                                          "
			 "                                         |\n");

	fprintf (fout,
			 "|---------------------------------------"
			 "------------------------------------------"
			 "-----------------------------------------|\n");

	fprintf (fout, ".PI12\n");
}

/*===============================================
| Routine to close the audit trail output file. |
===============================================*/
void
CloseAudit (
 void)
{
	fprintf (fout, ".EOF\n");
	pclose (fout);
}

void
ProcessInmu (void)
{
	if (yearEnd)
	{
		abc_selfield (inmu, "inmu_id_no2");

		strcpy (inmu_rec.co_no, comm_rec.co_no);
		strcpy (inmu_rec.year, " ");
		strcpy (inmu_rec.period, " ");
		cc = find_rec (inmu, &inmu_rec, GTEQ, "u");
		while (!cc && !strcmp (inmu_rec.co_no, comm_rec.co_no))
		{
			dsp_process ("Tran : ", "Delete");
			abc_delete (inmu);
			strcpy (inmu_rec.co_no, comm_rec.co_no);
			strcpy (inmu_rec.year, " ");
			strcpy (inmu_rec.period, " ");
			cc = find_rec (inmu, &inmu_rec, GTEQ, "u");
		}
		abc_unlock (inmu);
	}
	else
	{
		strcpy (inmu_rec.co_no, comm_rec.co_no);
		strcpy (inmu_rec.year, "L");
		strcpy (inmu_rec.period, processMonth);
		cc = find_rec (inmu, &inmu_rec, GTEQ, "u");
		while (!cc && !strcmp (inmu_rec.co_no, comm_rec.co_no) &&
					  !strcmp (inmu_rec.year, "L") &&
					  !strcmp (inmu_rec.period, processMonth))
		{
				dsp_process ("Tran : ", "Delete");
				abc_delete (inmu);

				strcpy (inmu_rec.co_no, comm_rec.co_no);
				strcpy (inmu_rec.year, "L");
				strcpy (inmu_rec.period, processMonth);
				cc = find_rec (inmu, &inmu_rec, GTEQ, "u");
		}
		abc_unlock (inmu);
		/*------------------------------------------------
		| Update Current year transactions to last year. |
		------------------------------------------------*/
		strcpy (inmu_rec.co_no, comm_rec.co_no);
		strcpy (inmu_rec.year, "C");
		strcpy (inmu_rec.period, processMonth);
		cc = find_rec (inmu, &inmu_rec, GTEQ, "u");
		while (!cc && !strcmp (inmu_rec.co_no, comm_rec.co_no) &&
					  !strcmp (inmu_rec.year, "C") &&
					  !strcmp (inmu_rec.period, processMonth))
		{
			dsp_process ("Tran : ", "Update");
			strcpy (inmu_rec.year, "L");
			cc = abc_update (inmu, &inmu_rec);
			if (cc)
				file_err (cc, inmu, "DBUPDATE");

			strcpy (inmu_rec.co_no, comm_rec.co_no);
			strcpy (inmu_rec.year, "C");
			strcpy (inmu_rec.period, processMonth);
			cc = find_rec (inmu, &inmu_rec, GTEQ, "u");
		}
	}
}

/*=====================================================================
|  Copyright (C) 1999 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : cr_pagebal.c  )                                    |
|  Program Desc  : Print Suppliers Aged Balance Reports.       )      |
|                                                              )      |
|---------------------------------------------------------------------|
|  Access files  :  comm, pocr, suin, sumr,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Author        : Terry Keillor.  | Date Written  : 23/05/89         |
|---------------------------------------------------------------------|
|  Date Modified : (23/05/89)      | Modified  by : Terry Keillor.    |
|  Date Modified : (26/06/89)      | Modified  by : Scott Darrow.     |
|  Date Modified : (08/04/91)      | Modified  by : Scott Darrow.     |
|  Date Modified : (28/09/92)      | Modified  by : Trevor van Bremen |
|  Date Modified : (24/05/93)      | Modified  by : Scott Darrow.     |
|  Date Modified : (27/01/94)      | Modified  by : Campbell Mander.  |
|  Date Modified : (06/04/96)      | Modified  by : Scott B Darrow.   |
|  Date Modified : (20/05/97)      | Modified  by : Scott B Darrow.   |
|  Date Modified : (03/09/97)      | Modified  by : Jiggs A Veloz.    |
|  Date Modified : (19/06/98)      | Modified  by : Marnie I Organo.  |
|  Date Modified : (04/05/99)      | Modified  by : Ana Marie C Tario |
|  Date Modified : (28/09/1999)    | Modified  by : Ramon A. Pacheco  |
|                                                                     |
|  Comments      : (23/05/89) Rewritten for multi-currency suppliers. |
|                : (26/06/89) - Change Column Headings.               |
|                : (08/04/91) - Added dsp_??? + fixed S/C PEA-4712.   |
|  (28/09/92)    : PSL 7859. Major revamp re CR_CO.                   |
|  (24/05/93)    : DFT-8896. Included addition of get_bal () +        |
|                :           get_cheq ()                              |
|  (27/01/94)    : HGP 9846. Remove suhd_loc_disc_taken from dbview.  |
|                :                                                    |
|   (06/04/96)   : PDL - Updated to change cheque length from 6-8.    |
|                :                                                    |
|   (20/05/97)   : PDL - Updated to change cheque length from 8-13    |
|   (19/06/98)   : BFS - Added line space before "Total for Supplier" |
|                        in printout.                                 |
|   (04/05/99)   : ASL - SC1243 Corrected amount paid for local.      |
|   (28/09/1999) : Ported to ANSI standards.                          |
| $Log: cr_pagebal.c,v $
| Revision 5.2  2001/08/09 08:52:07  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:01:37  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:03:29  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:23:58  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:12:59  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 08:41:01  gerry
| Forced Revision No. Start 2.0 Rel-15072000
|
| Revision 1.22  1999/12/14 09:19:42  ana
| (14/12/1999) SC2191 Corrected condition for AGE_PAY.
|
| Revision 1.21  1999/12/10 04:06:56  scott
| Updated to remove the space between @ and (#) as this prevended version from being displayed correctly. Reported by sunlei
|
| Revision 1.20  1999/12/06 01:29:06  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.19  1999/11/11 06:59:30  scott
| Updated for PNAME removal as you can use ^P
|
| Revision 1.18  1999/11/11 02:40:20  scott
| Updated to add app.schema
|
| Revision 1.17  1999/11/04 10:28:30  scott
| Updated due to usage of -Wall flag on compiler.
|
| Revision 1.16  1999/10/20 02:06:40  nz
| Updated for final changes on date routines.
|
| Revision 1.15  1999/10/13 03:35:13  ramon
| Corrected the writable strings error.
|
| Revision 1.14  1999/10/08 02:12:46  scott
| Updated from Ansi Project.
|
| Revision 1.13  1999/07/14 09:40:54  ana
| (14/07/1999) Patched revision in 9.9.
|$Log: cr_pagebal.c,v $
|Revision 5.2  2001/08/09 08:52:07  scott
|Updated to add FinishProgram () function
|
|Revision 5.1  2001/08/06 23:01:37  scott
|RELEASE 5.0
|
|Revision 5.0  2001/06/19 08:03:29  robert
|LS10-5.0 New Release as of 19 JUNE 2001
|
|Revision 4.0  2001/03/09 02:23:58  scott
|LS10-4.0 New Release as at 10th March 2001
|
|Revision 3.0  2000/10/10 12:12:59  gerry
|Revision No. 3 Start
|<after Rel-10102000>
|
|Revision 2.0  2000/07/15 08:41:01  gerry
|Forced Revision No. Start 2.0 Rel-15072000
|
|Revision 1.22  1999/12/14 09:19:42  ana
|(14/12/1999) SC2191 Corrected condition for AGE_PAY.
|                                                             |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cr_pagebal.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CR/cr_pagebal/cr_pagebal.c,v 5.2 2001/08/09 08:52:07 scott Exp $";

#define		NO_SCRGEN
#include	<pslscr.h>
#define		MCURR		 (multiCurrency [0] == 'Y')
#define		AGE_PAY		 (ageType [0] == 'P')
#define		ZERO		 (!strcmp (loc_bal_str,"         0.00") || \
                          !strcmp (loc_bal_str, "        -0.00"))
#define		DETAIL		 (detailReport [0] == 'D')
#define		SUMMARY		 (detailReport [0] == 'S')
#define		LOC_CUR		 (currencyType [0] == 'L')
#define		OSE_CUR		 (currencyType [0] == 'O')
#define		BY_COMPANY	 (byCompany [0] == 'C')
#define		BY_BRANCH	 (byCompany [0] == 'B')
#define		APPROVED	 (suin_rec.approved [0] == 'Y')
#define		INVOICE		 (suin_rec.type [0] == '1')
#define		CREDIT		 (suin_rec.type [0] == '2')
#define		JOURNAL		 (suin_rec.type [0] == '3')

#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<ml_cr_mess.h>

#include 	"schema"

	/*=================================
	| File comm {System Common file}. |
	=================================*/
	struct dbview comm_list [] ={
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_co_short"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_est_short"},
		{"comm_crd_date"},
		{"comm_pay_terms"}
		};

	int comm_no_fields = 9;

	struct {
		int 	term;
		char 	tco_no [3];
		char 	tco_name [41];
		char 	tco_short [16];
		char 	tes_no [3];
		char 	tes_name [41];
		char 	tes_short [16];
		long 	tcrd_date;
		int		pay_terms;
		} comm_rec;

	struct	suinRecord	suin_rec;
	struct	sumrRecord	sumr_rec;
	struct	suhdRecord	suhd_rec;
	struct	sudtRecord	sudt_rec;
	struct	pocrRecord	pocr_rec;

	char	*comm	= "comm",
			*data	= "data";

	/*=================================================
	| Special variables required.                     |
	=================================================*/
	int		envCrCo = 0,
			printerNumber = 1;

	char	branchNumber [3],
	    	multiCurrency [2],
			byCompany [2],
			currencyType [2],
			currencyCode [4],
			detailReport [2],
			ageType [2];

	double	groupTotal	= 0.00,
			notApproved = 0.00;

	double	supplierTotal	 	[5] = {0,0,0,0,0},
			localCurrencyTotal	[5] = {0,0,0,0,0},
			currencyTotal	 	[5] = {0,0,0,0,0},
			grandTotal		 	[5] = {0,0,0,0,0},
			nonAppoveSupplier	[5] = {0,0,0,0,0},
			napp_loc_cur	 	[5] = {0,0,0,0,0},
			napp_cur_total	 	[5] = {0,0,0,0,0},
			napp_grand		 	[5] = {0,0,0,0,0},
			localTotal			[5] = {0,0,0,0,0},
			fgnTotal			[5] = {0,0,0,0,0};

/*=====================================================================
| The structures 'dtls' are initialised in function 'GetCheq'        |
| the number of details is stored in external variable 'dtlsCnt'.    |
=====================================================================*/ 
struct	Detail
{						/*-----------------------------------*/
	long	hhsi_hash;	/*| detail invoice reference.       |*/
	double	loc_amt;	/*| detail invoice amount.          |*/
	double	fgn_amt;	/*| detail invoice amount.          |*/
	int	cheq_hash;		/*| cheq structure pointer.         |*/
} 	*dtls;				/*-----------------------------------*/
	DArray	dtlsD;
	int		dtlsCnt;

	int	cheqCnt;

	/*-------------------------------
	| Current & last 3 month dates. |
	-------------------------------*/
	long	ageDate [5];

	FILE	*pp;

/*===========================
| Local function prototypes |
===========================*/
void	ProcessSupplier		 (void);
void	PrintGrandTotal		 (void);
void	OpenDB				 (void);
void	CloseDB				 (void);
void	shutdown_prog		 (void);
int		ReportHeading		 (void);
void	PrintCurrentHead 	 (void);
int		PrintLine			 (void);
void	GetCommInfo			 (void);
void	GetBalance			 (long);
int		GetCheque			 (long);
void	GetAmountPaid		 (double *);


/*===================================================
| Main processing routine.                          |
===================================================*/
int
main (
 int	argc,
 char *	argv [])
{
	int	firstCurrency = 1;

	if (argc != 7)
	{
		/*-------------------------------------------------------
		| Usage %s LPNO <C(py),B(rch)> <L(oc),O (vr)> <AZZ,CUR> |
		| <D(et),S(um)> <I(nvoice Age) P(ayment Age)			|
		-------------------------------------------------------*/
		print_at (0,0, mlCrMess011, argv [0]);
		return (EXIT_FAILURE);
	}
	printerNumber = atoi (argv [1]);
	sprintf (byCompany,    	"%1.1s",  argv [2]);
	sprintf (currencyType, 	"%1.1s",  argv [3]);
	sprintf (currencyCode,	"%-3.3s", argv [4]);
	sprintf (detailReport,  "%-1.1s", argv [5]);
	sprintf (ageType, 		"%-1.1s", argv [6]);

	if ((!LOC_CUR && !OSE_CUR) || (!DETAIL && !SUMMARY) ||
		 (!BY_COMPANY && !BY_BRANCH))
	{
		print_at (0, 0, mlCrMess011, argv [0]);
		return (EXIT_FAILURE);
	}

	/*---------------------------------------------------
	| Allocate initial lines for cheques and details    |
	---------------------------------------------------*/
	ArrAlloc (&dtlsD, &dtls, sizeof (struct Detail), 1000);

	sprintf (multiCurrency, "%-1.1s", get_env ("CR_MCURR"));

	OpenDB ();
	GetCommInfo ();

	sprintf (err_str, "Processing Suppliers %s ageing schedule.",
				 (DETAIL) ? "Detailed" : "Summary");

	dsp_screen (err_str, comm_rec.tco_no, comm_rec.tco_name);

	envCrCo = atoi (get_env ("CR_CO"));

	strcpy (branchNumber, (envCrCo) ? comm_rec.tes_no : " 0");

	if (ReportHeading ())
	{
		shutdown_prog ();
		return (EXIT_FAILURE);
	}

	/*---------------------------------------
	| Report on all currencies.             |
	---------------------------------------*/
	if (MCURR && !strcmp (currencyCode,"AZZ"))
	{
		strcpy (pocr_rec.co_no,comm_rec.tco_no);
		sprintf (pocr_rec.code ,"   ");
		cc = find_rec (pocr, &pocr_rec, GTEQ, "r");
		while (!cc && !strcmp (pocr_rec.co_no,comm_rec.tco_no))
		{
			if (firstCurrency != 1)
				fprintf (pp, ".PA\n");
			firstCurrency = 0;

			if (MCURR)
				PrintCurrentHead ();

			ProcessSupplier ();
			cc = find_rec (pocr,&pocr_rec,NEXT,"r");
		}
	}
	/*---------------------------------------
	| Report on selected currency only.     |
	---------------------------------------*/
	else if (MCURR)
	{
		strcpy (pocr_rec.co_no,comm_rec.tco_no);
		sprintf (pocr_rec.code ,"%-3.3s", currencyCode);
		cc = find_rec (pocr, &pocr_rec, COMPARISON, "r");
		if (!cc)
		{
			PrintCurrentHead  ();
			ProcessSupplier ();
		}
	}
	/*---------------------------------------
	| Report without currency selection.    |
	---------------------------------------*/
	else if (!MCURR)
			ProcessSupplier ();

	PrintGrandTotal ();

	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*===================================================
| Process suppliers for currency                    |
===================================================*/
void
ProcessSupplier (
 void)
{
	int	i;
	double	sum_fgn = 0.00,
		sum_loc = 0.00;
	int	zero_bal = TRUE;

	strcpy (sumr_rec.co_no,comm_rec.tco_no);
	strcpy (sumr_rec.est_no, (BY_COMPANY) ? "  " : branchNumber);
	strcpy (sumr_rec.acronym, "         ");

	cc = find_rec (sumr, &sumr_rec, GTEQ, "r");
	while (!cc && !strcmp (sumr_rec.co_no,comm_rec.tco_no))
	{
		if (BY_BRANCH && strcmp (sumr_rec.est_no, branchNumber))
		{
			cc = find_rec (sumr, &sumr_rec, NEXT, "r");
			continue;
		}

		if (MCURR && strcmp (pocr_rec.code, sumr_rec.curr_code))
		{
			cc = find_rec (sumr, &sumr_rec, NEXT, "r");
			continue;
		}
		for (i = 0; i < 4; i++)
		{
			localTotal [i] = 0.00;
			fgnTotal   [i] = 0.00;
		}
		dsp_process ("Supplier", sumr_rec.crd_no);

	
		GetBalance (sumr_rec.hhsu_hash);

		/*---------------------------------------
		| Get history sum, skip debtor if zero. |
		---------------------------------------*/
		zero_bal = TRUE;
		sum_fgn = 0.00;
		sum_loc = 0.00;
		for (i = 0; i < 4; i++)
		{
			if (localTotal [i] != 0.00)
				zero_bal = FALSE;

			if (fgnTotal [i] != 0.00)
				zero_bal = FALSE;

			sum_fgn += fgnTotal [i];
			sum_loc += localTotal [i];
		}
		if (!DETAIL && sum_fgn == 0.00 && sum_loc == 0.00)
		{
			cc = find_rec (sumr, &sumr_rec, NEXT, "r");
			continue;
		}
		if (DETAIL && sum_fgn == 0.00 && sum_loc == 0.00 && zero_bal)
		{
			cc = find_rec (sumr, &sumr_rec, NEXT, "r");
			continue;
		}

		suin_rec.date_of_inv = 0L;
		suin_rec.hhsu_hash = sumr_rec.hhsu_hash;
		cc = find_rec (suin, &suin_rec, GTEQ, "r");
		if ((cc || sumr_rec.hhsu_hash != suin_rec.hhsu_hash) &&
			sum_fgn == 0.00 && sum_loc == 0.00)
		{
			cc = find_rec (sumr, &sumr_rec, NEXT, "r");
			continue;
		}

		if (DETAIL)
		{
			fprintf (pp,"| (%s) %s   ",sumr_rec.crd_no,
						sumr_rec.crd_name);
			fprintf (pp,"|             |             |             |             |             |             |\n");
		}
		else
		{
			fprintf (pp,"| %s | %s   |",sumr_rec.crd_no,
						 sumr_rec.crd_name);
		}

		suin_rec.date_of_inv = 0L;
		suin_rec.hhsu_hash = sumr_rec.hhsu_hash;
		cc = find_rec (suin, &suin_rec, GTEQ, "r");
		while (!cc && sumr_rec.hhsu_hash == suin_rec.hhsu_hash)
		{
			PrintLine ();
			cc = find_rec (suin, &suin_rec, NEXT, "r");
		}

		/*--------------------------------------------
		| Print totals for supplier, if totals != 0. |
		--------------------------------------------*/
		groupTotal = 0.00;
		for (i = 0; i < 5; i++)
			groupTotal += supplierTotal [i];

		if (DETAIL)
		{
			fprintf (pp,"|                                                     |");
		fprintf (pp,"             |             |             |             |             |             |\n");
			fprintf (pp,"|                ** Total For Supplier **             |");
		}

		fprintf (pp,"%13.2f|",DOLLARS (groupTotal));
		for (i = 0; i < 5; i++)
		{
			if (supplierTotal [i] != 0.00)
				fprintf (pp,"%13.2f|", DOLLARS (supplierTotal [i]));
			else
				fprintf (pp,"             |");
		}
		fprintf (pp,"\n");

		fprintf (pp,"|-----------------------------------------------------|");
		fprintf (pp,"-------------|-------------|-------------|-------------|-------------|-------------|\n");

		for (i = 0; i < 5; i++)
		{
			if (!MCURR)
			{
				grandTotal [i]  += supplierTotal [i];
				napp_grand [i]  += nonAppoveSupplier [i];
			}
			supplierTotal [i] 	   = 0.00;
			nonAppoveSupplier [i]  = 0.00;
		}
		cc = find_rec (sumr, &sumr_rec, NEXT, "r");
	}

	/*-------------------------------------
	| Print totals for currency.          |
	-------------------------------------*/
	if (MCURR)
	{
		notApproved = 0.00;
		for (i = 0; i < 5; i++)
			notApproved += napp_cur_total [i];

		fprintf (pp,".LRP4\n");
		fprintf (pp,"|  ** TOTAL NOT APPROVED (FOR CURRENCY) **            |");
		fprintf (pp,"%13.2f|",DOLLARS (notApproved));

		for (i = 0; i < 5; i++)
		{
			if (napp_cur_total [i] != 0.00)
				fprintf (pp,"%13.2f|", DOLLARS (napp_cur_total [i]));
			else
				fprintf (pp,"             |");

			napp_cur_total [i] = 0.00;
		}
		fprintf (pp,"\n");

		fprintf (pp,"|-----------------------------------------------------|");
		fprintf (pp,"-------------|-------------|-------------|-------------|-------------|-------------|\n");

		groupTotal = 0;
		for (i = 0; i < 5; i++)
			groupTotal += currencyTotal [i];

		fprintf (pp,"|  ** TOTAL BALANCE DUE  (FOR CURRENCY) **            |");
		fprintf (pp,"%13.2f|",DOLLARS (groupTotal));

		for (i = 0; i < 5; i++)
		{
			if (currencyTotal [i] != 0.00)
				fprintf (pp,"%13.2f|", DOLLARS (currencyTotal [i]));
			else
				fprintf (pp,"             |");

			currencyTotal [i] = 0.00;
		}
		fprintf (pp,"\n");

		fprintf (pp,"|-----------------------------------------------------|");
		fprintf (pp,"-------------|-------------|-------------|-------------|-------------|-------------|\n");

		/*------------------------------------------
		| Print local currency equivalent totals.  |
		------------------------------------------*/
		notApproved = 0.00;
		for (i = 0; i < 5; i++)
		{
			notApproved += napp_loc_cur [i];
			napp_grand [i] += napp_loc_cur [i];
		}

		fprintf (pp,".LRP4\n");
		fprintf (pp,"|  ** TOTAL NOT APPROVED (EQUIV. LOCAL CURRENCY) **   |");
		fprintf (pp,"%13.2f|",DOLLARS (notApproved));

		for (i = 0; i < 5; i++)
		{
			if (napp_loc_cur [i] != 0.00)
				fprintf (pp,"%13.2f|", DOLLARS (napp_loc_cur [i]));
			else
				fprintf (pp,"             |");

			napp_loc_cur [i] = 0.00;
		}
		fprintf (pp,"\n");

		fprintf (pp,"|-----------------------------------------------------|");
		fprintf (pp,"-------------|-------------|-------------|-------------|-------------|-------------|\n");

		groupTotal = 0;
		for (i = 0; i < 5; i++)
		{
			groupTotal += localCurrencyTotal [i];
			grandTotal [i] += localCurrencyTotal [i];
		}

		fprintf (pp,"|  ** TOTAL BALANCE DUE  (EQUIV. LOCAL CURRENCY) **   |");
		fprintf (pp,"%13.2f|",DOLLARS (groupTotal));

		for (i = 0; i < 5; i++)
		{
			if (localCurrencyTotal [i] != 0.00)
				fprintf (pp,"%13.2f|", DOLLARS (localCurrencyTotal [i]));
			else
				fprintf (pp,"             |");

			localCurrencyTotal [i] = 0.00;
		}
	    	fprintf (pp,"\n");
	}
}

/*======================
| Print grand totals.  |
======================*/
void
PrintGrandTotal (
 void)
{
	int	i;

	fprintf (pp,"=====================================================");
	fprintf (pp,"=====================");
	fprintf (pp,"================================================================\n");
	notApproved = 0.00;
	for (i = 0; i < 5; i++)
		notApproved += napp_grand [i];

	fprintf (pp,".LRP4\n");
	fprintf (pp,"|  ** GRAND TOTAL NOT APPROVED **                     |");
	fprintf (pp,"%13.2f|",DOLLARS (notApproved));

	for (i = 0; i < 5; i++)
	{
		if (napp_grand [i] != 0.00)
			fprintf (pp,"%13.2f|", DOLLARS (napp_grand [i]));
		else
			fprintf (pp,"             |");
	}
	fprintf (pp,"\n");

	fprintf (pp,"|-----------------------------------------------------|");
	fprintf (pp,"-------------|-------------|-------------|-------------|-------------|-------------|\n");

	groupTotal = 0;
	for (i = 0; i < 5; i++)
		groupTotal += grandTotal [i];

	fprintf (pp,"|  ** GRAND TOTAL BALANCE DUE ALL **                  |");
	fprintf (pp,"%13.2f|",DOLLARS (groupTotal));

	for (i = 0; i < 5; i++)
	{
		if (grandTotal [i] != 0.00)
			fprintf (pp,"%13.2f|", DOLLARS (grandTotal [i]));
		else
			fprintf (pp,"             |");
	}
	fprintf (pp,"\n");
	fprintf (pp,".EOF\n");
	pclose (pp);
}

/*======================
| Open Database Files. |
======================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	open_rec (suin, suin_list, SUIN_NO_FIELDS, "suin_cron");
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, "sumr_id_no2");
	open_rec (pocr, pocr_list, POCR_NO_FIELDS, "pocr_id_no");
	open_rec (sudt, sudt_list, SUDT_NO_FIELDS, "sudt_hhsp_hash");
	open_rec (suhd, suhd_list, SUHD_NO_FIELDS, "suhd_hhsu_hash");
}

/*=======================
| Close Database Files. |
=======================*/
void
CloseDB (
 void)
{
	abc_fclose (sumr);
	abc_fclose (suin);
	abc_fclose (pocr);
	abc_fclose (sudt);
	abc_fclose (suhd);
	abc_dbclose (data);
}

void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();

	/*---------------------------
	| Free array memory to OS   |
	---------------------------*/
	ArrDelete (&dtlsD);
	crsr_on ();
}

/*======================
| Print Report Heading |
======================*/
int
ReportHeading (
 void)
{
	/*-----------------------
	| Open pipe to pformat. |
	-----------------------*/
	if ((pp = popen ("pformat", "w")) == 0)
	{
		sys_err ("Error in pformat During (POPEN)", 1, PNAME);
		return (EXIT_FAILURE);
	}

	fprintf (pp,".START%s<%s>\n", DateToString (comm_rec.tcrd_date), PNAME);
	fprintf (pp,".LP%d\n", printerNumber);
	fprintf (pp,".14\n");
	fprintf (pp,".L140\n");
	fprintf (pp,".E%s\n", clip (comm_rec.tco_name));

	fprintf (pp,".EReport Aged using %s\n",
				 (AGE_PAY) ? "Payment date" : "Date of invoice");

	if (DETAIL)
		fprintf (pp,".ESUPPLIERS DETAILED AGEING SCHEDULE\n");
	else
		fprintf (pp,".ESUPPLIERS SUMMARY AGEING SCHEDULE\n");

	if (BY_BRANCH)
		fprintf (pp,".E%s As At : %s\n", comm_rec.tes_short, SystemTime ());
	else
		fprintf (pp,".EAs At : %s \n", SystemTime ());

	fprintf (pp,".B1\n");

	fprintf (pp,".R=====================================================");
	fprintf (pp,"=======================");
	fprintf (pp,"===============================================================\n");
	if (!MCURR)
		fprintf (pp,"Note : '*' Denotes Invoices not approved \n");

	else if (LOC_CUR)
		fprintf (pp,"Note : '*' Denotes Invoices not approved                                             Balances are in LOCAL CURRENCY \n");
	else
		fprintf (pp,"Note : '*' Denotes Invoices not approved                                             Balances are in OVERSEAS CURRENCY \n");

	fprintf (pp,"=====================================================");
	fprintf (pp,"=======================");
	fprintf (pp,"===============================================================\n");

	if (DETAIL)
	{
		fprintf (pp,"|       INVOICE         |    DATE    |    NARRATIVE.  |");
		fprintf (pp,"                            B A L A N C E   D U E                                  |\n");
		fprintf (pp,"|                       |     OF     |                |");
		fprintf (pp,"-----------------------------------------------------------------------------------|\n");
		fprintf (pp,"|       NUMBER          |   INVOICE  |                |");
		fprintf (pp,"     TOTAL   |   CURRENT   |   1 TO 30   |  31 TO 60   |  61 TO 90   |   OVER 90   |\n");
	}
	else
	{
		fprintf (pp,"|SUPPLIER|  SUPPLIER NAME                             |");
		fprintf (pp,"                            B A L A N C E   D U E                                  |\n");
		fprintf (pp,"|  CODE  |                                            |");
		fprintf (pp,"-----------------------------------------------------------------------------------|\n");
		fprintf (pp,"|        |                                            |");
		fprintf (pp,"     TOTAL   |   CURRENT   |   1 TO 30   |  31 TO 60   |  61 TO 90   |   OVER 90   |\n");
	}
	fprintf (pp,"|-----------------------------------------------------|");
	fprintf (pp,"-------------|-------------|-------------|-------------|-------------|-------------|\n");

	fprintf (pp, ".PI12\n");
	return (EXIT_SUCCESS);
}

/*========================
| Print Currency Heading |
========================*/
void
PrintCurrentHead  (
 void)
{
	fprintf (pp,"| CURRENCY %s - %-35.35s  |",
				pocr_rec.code, pocr_rec.description);
	fprintf (pp,"             |             |             |             |             |             |\n");
	fprintf (pp,"|                                                     |");
	fprintf (pp,"             |             |             |             |             |             |\n");
}

/*======================
| Print Invoice Line.  |
======================*/
int
PrintLine (
 void)
{
	int	pip,
		i;

	double	balance = 0.0,
	      	localBalance = 0.0,
	      	loc_amt_paid = 0.0,
	      	prt_balance = 0.0;

	char	approved [2],
			loc_bal_str [14],
			pr_type [3];

	/*----------------------------------------------------
	| For each invoice, print details if dbt - crd <> 0. |
	----------------------------------------------------*/
	balance = suin_rec.amt - suin_rec.amt_paid;

    if (MCURR)
        GetAmountPaid (&loc_amt_paid);

	if ((suin_rec.amt/suin_rec.exch_rate - loc_amt_paid) == 0.00)
		return (EXIT_FAILURE);

	strcpy (approved, (APPROVED) ? " " : "*");
	strcpy (pr_type, "??");

	if (INVOICE)
		strcpy (pr_type, "IN");

	if (CREDIT)
		strcpy (pr_type, "CR");

	if (JOURNAL)
		strcpy (pr_type, "JN");


	/*---------------------------------------
	| Adjust for multi-currency.            |
	---------------------------------------*/
	if (MCURR)
	{
		if (suin_rec.exch_rate > 0.00)
			localBalance = suin_rec.amt / suin_rec.exch_rate - loc_amt_paid;
		else
			localBalance = suin_rec.amt - loc_amt_paid;

	}

	if (MCURR && LOC_CUR)
		prt_balance = localBalance;
	else
		prt_balance = balance;
	sprintf (loc_bal_str, "%13.2f", prt_balance);

	if (DETAIL && !ZERO)
		fprintf (pp,"|%s  %s  %s | %s |     %6.6s     |             |",
			approved,
			pr_type,
			suin_rec.inv_no,
			DateToString (suin_rec.date_of_inv),
			suin_rec.cus_po_no);
	/*---------------------------------------
	| Flag reset when correct column found. |
	---------------------------------------*/
	pip = 1;
	for (i = 0; i < 5; i++)
	{
		if (pip && ((AGE_PAY) ? suin_rec.pay_date : suin_rec.date_of_inv) > ageDate [i])
		{
			if (DETAIL && !ZERO) 
				fprintf (pp,"%13.2f|", DOLLARS (prt_balance));

			supplierTotal [i] += prt_balance;
			if (!APPROVED)
				nonAppoveSupplier [i] += prt_balance;

			if (MCURR)
			{
				currencyTotal [i] += balance;
				localCurrencyTotal [i] += localBalance;
				if (!APPROVED)
				{
					napp_cur_total [i] += balance;
					napp_loc_cur [i] += localBalance;
				}
			}
			pip = 0;
		}
		else if (DETAIL && !ZERO)
			fprintf (pp,"             |");
	}
	if (DETAIL && !ZERO)
		fprintf (pp,"\n");

	return (EXIT_SUCCESS);
}

/*======================
| Get comm record.     |
======================*/
void
GetCommInfo (
 void)
{
	int	i; 

	strcpy (sumr_rec.co_no, comm_rec.tco_no);
	strcpy (sumr_rec.est_no, comm_rec.tes_no);

	/*------------------------------------------------------
	| Find dates for ageing i.e. end of last three months. |
	------------------------------------------------------*/
	ageDate [0] = MonthEnd (comm_rec.tcrd_date);
	ageDate [4] = 0L;

	for (i = 1; i < 4; i++)
		ageDate [i] = AddMonths (ageDate [0], -i);
}
/*=======================
| Get invoice balances. |
=======================*/
void
GetBalance (
 long	hhsuHash)
{
	int		i;

	double	fgnBal = 0.00;
	double	locBal = 0.00;

	GetCheque (hhsuHash);

	suin_rec.date_of_inv = 0L;
	suin_rec.hhsu_hash = hhsuHash;

	for (cc = find_rec ("suin", &suin_rec, GTEQ, "r");
		 !cc && hhsuHash == suin_rec.hhsu_hash ;
		 cc = find_rec ("suin", &suin_rec, NEXT, "r"))
	{
		fgnBal = suin_rec.amt;

		if (suin_rec.exch_rate > 0.00)
			locBal = fgnBal / suin_rec.exch_rate;

		for (i = 0; i < dtlsCnt; i++)
		{
			if (dtls [i].hhsi_hash == suin_rec.hhsi_hash)
			{
				locBal -= dtls [i].loc_amt;
				fgnBal -= dtls [i].fgn_amt;
			}
		}
		if (locBal == 0 && fgnBal == 0)
		{
			continue;
		}
		i = age_bals (comm_rec.pay_terms, suin_rec.date_of_inv, comm_rec.tcrd_date);

		localTotal [i] += locBal;
		fgnTotal [i] += fgnBal;
	}
}

/*------------------------------------------
| Read cheque routine for supplier.        |
------------------------------------------*/
int
GetCheque (
 long	hhsuHash)
{
	cheqCnt = 0;
	dtlsCnt = 0;

	for (cc = find_hash (suhd, &suhd_rec, GTEQ, "r", hhsuHash);
		 !cc && suhd_rec.hhsu_hash == hhsuHash;
	     cc = find_hash (suhd, &suhd_rec, NEXT, "r", hhsuHash))
	{
	    /*-----------------------------------------------
	    | Else create detail records for normal cheque. |
	    -----------------------------------------------*/
	    for (cc = find_hash (sudt, &sudt_rec, GTEQ, "r",suhd_rec.hhsp_hash);
	    	 !cc && suhd_rec.hhsp_hash == sudt_rec.hhsp_hash;
			 cc = find_hash (sudt, &sudt_rec, NEXT, "r", suhd_rec.hhsp_hash))
	    {
			/*-----------------------------------
			| Increase dtls array as required   |
			-----------------------------------*/
			if (!ArrChkLimit (&dtlsD, dtls, dtlsCnt))
			   sys_err ("ArrChkLimit (details)", ENOMEM, PNAME);

			dtls [dtlsCnt].loc_amt = sudt_rec.loc_paid_inv;
			dtls [dtlsCnt].fgn_amt = sudt_rec.amt_paid_inv;
			dtls [dtlsCnt].hhsi_hash = sudt_rec.hhsi_hash;
			dtls [dtlsCnt].cheq_hash = cheqCnt;
			++dtlsCnt;
	    }
	    ++cheqCnt;
	}
	return (EXIT_SUCCESS);
}
void
GetAmountPaid (
 double *	loc_amt_paid)
{
    *loc_amt_paid = 0.00;
    abc_selfield (sudt,"sudt_hhsi_hash");
    sudt_rec.hhsi_hash = suin_rec.hhsi_hash;
    cc = find_rec (sudt, &sudt_rec, COMPARISON, "r");
    while (!cc && suin_rec.hhsi_hash == sudt_rec.hhsi_hash)
    {
        *loc_amt_paid += sudt_rec.loc_paid_inv + sudt_rec.exch_variatio;
        cc = find_rec (sudt, &sudt_rec, NEXT, "r");
    }
    abc_selfield (sudt,"sudt_hhsp_hash");
}


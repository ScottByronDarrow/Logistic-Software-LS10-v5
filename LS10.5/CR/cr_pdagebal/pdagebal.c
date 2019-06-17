/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: pdagebal.c,v 5.4 2001/12/07 04:57:58 scott Exp $
|  Program Name  : (cr_pdagebal.c) 
|  Program Desc  : (Print Suppliers Detailed Aged Balance Report.) 
|---------------------------------------------------------------------|
|  Author        : Terry Keillor.  | Date Written  : 23/05/89         |
|---------------------------------------------------------------------|
| $Log: pdagebal.c,v $
| Revision 5.4  2001/12/07 04:57:58  scott
| Updated to convert to app.schema and perform a general code clean.
|
| Revision 5.3  2001/12/07 03:06:07  kaarlo
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: pdagebal.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CR/cr_pdagebal/pdagebal.c,v 5.4 2001/12/07 04:57:58 scott Exp $";

#define	MCURR		 (multiCurrency [0] == 'Y')

#include	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<ml_std_mess.h>
#include 	<ml_cr_mess.h>

#define	BY_COMPANY	 (byCompany [0] == 'C')
#define	BY_BRANCH	 (byCompany [0] == 'B')
#define	LOCAL		 (currencyType [0] == 'L')
#define	OVERSEAS	 (currencyType [0] == 'O')
#define	APPROVED	 (suin_rec.approved [0] == 'Y')

#include    "schema"

struct commRecord   comm_rec;
struct suhdRecord   suhd_rec;
struct suinRecord   suin_rec;
struct sumrRecord   sumr_rec;
struct pocrRecord   pocr_rec;

	/*=================================================
	| Special variables required.                     |
	=================================================*/
	int		envCrCo = 0,
			printerNumber = 1;

	char	branchNumber [3],
			byCompany [2],
	    	multiCurrency [2],
			currencyType [2],
			currencyCode [4];

	double	grandWorkTotal			 = 0, 
			notApproved				 = 0,
			supplierTotal [5]      	 = {0,0,0,0,0},
			localCurrTotal [5] 	  	 = {0,0,0,0,0},
			currencyTotal [5]      	 = {0,0,0,0,0}, 
			grandTotal [5]    	  	 = {0,0,0,0,0}, 
			notApprovedSuppler [5] 	 = {0,0,0,0,0},
			notApprovedLocalCurr [5] = {0,0,0,0,0},
			notApprovedFgnCurr [5] 	 = {0,0,0,0,0},
			notApprovedGrand [5]     = {0,0,0,0,0};

	/*-------------------------------
	| Current & last 3 month dates. |
	-------------------------------*/
	long	age_date [5];

	FILE	*pp;

/*===========================
| Local function prototypes |
===========================*/
void	ProcessSuppliers (void);
void	PrintGrandTotal	 (void);
void	OpenDB			 (void);
void	CloseDB		 	 (void);
void	shutdown_prog	 (void);
int		HeadingOutput	 (void);
void	PrintCurrHead	 (void);
int		PrintLine		 (void);
void	GetComm		 	 (void);

/*===================================================
| Main processing routine.                          |
===================================================*/
int
main (
 int	argc,
 char *	argv [])
{
	int	first_curr = 1;

	if (argc != 5)
	{
		print_at (0, 0, mlCrMess003 , argv [0]);
		return (EXIT_FAILURE);
	}
	printerNumber = atoi (argv [1]);
	sprintf (byCompany, "%1.1s", argv [2]);
	sprintf (currencyType, "%1.1s", argv [3]);
	sprintf (currencyCode, "%-3.3s", argv [4]);

	if (!BY_COMPANY && !BY_BRANCH)
	{
		print_at (0, 0, mlCrMess003, argv [0]);
		return (EXIT_FAILURE);
	}

	if (!LOCAL && !OVERSEAS)
	{
		print_at (0, 0, mlCrMess003, argv [0]);
		return (EXIT_FAILURE);
	}

	sprintf (multiCurrency, "%-1.1s", get_env ("CR_MCURR"));

	OpenDB ();
	GetComm ();

	envCrCo = atoi (get_env ("CR_CO"));
	strcpy (branchNumber, (envCrCo) ? comm_rec.est_no : " 0");

	if (HeadingOutput ())
		return (EXIT_FAILURE);

	/*---------------------------------------
	| Report on all currencies.             |
	---------------------------------------*/
	if (MCURR && strcmp (currencyCode, "AZZ") == 0)
	{
		strcpy (pocr_rec.co_no, comm_rec.co_no);
		sprintf (pocr_rec.code , "   ");
		cc = find_rec (pocr, &pocr_rec, GTEQ, "r");
		while (!cc && !strcmp (pocr_rec.co_no, comm_rec.co_no))
		{                        
			if (first_curr != 1)
				fprintf (pp, ".PA\n");
			first_curr = 0;

			if (MCURR)
				PrintCurrHead ();

			ProcessSuppliers ();
			cc = find_rec (pocr, &pocr_rec, NEXT, "r");
		}
	}
	/*---------------------------------------
	| Report on selected currency only.     |
	---------------------------------------*/
	else if (MCURR)
	{
		strcpy (pocr_rec.co_no, comm_rec.co_no);
		sprintf (pocr_rec.code , "%-3.3s", currencyCode);
		cc = find_rec (pocr, &pocr_rec, COMPARISON, "r");
		if (!cc)                  
		{
			PrintCurrHead ();
			ProcessSuppliers ();
		}
	}
	/*---------------------------------------
	| Report without currency selection.    |
	---------------------------------------*/
	else if (!MCURR)
		ProcessSuppliers ();

	PrintGrandTotal ();

	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*===================================================
| Process suppliers for currency                    |
===================================================*/
void
ProcessSuppliers (void)
{
	int	i; 
	double	sum = 0L;

	strcpy (sumr_rec.co_no, comm_rec.co_no);
	strcpy (sumr_rec.est_no, branchNumber);
	strcpy (sumr_rec.acronym, "         ");

	cc = find_rec (sumr, &sumr_rec, GTEQ, "r");
	while (!cc && !strcmp (sumr_rec.co_no, comm_rec.co_no))
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

		/*---------------------------------------
		| Get history sum, skip debtor if zero. |
		---------------------------------------*/
		sum = 0.00;

		sum = sumr_rec.bo_curr + sumr_rec.bo_per1 +
			  sumr_rec.bo_per2 + sumr_rec.bo_per3;

		if (sum == 0.00)
		{
			cc = find_rec (sumr, &sumr_rec, NEXT, "r");
			continue;
		}

		fprintf (pp, "| (%s) %s ", sumr_rec.crd_no, sumr_rec.crd_name);
		fprintf (pp, "|             |             |             |             |             |             |\n");
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
		grandWorkTotal = 0.00;
		for (i = 0; i < 5; i++)
			grandWorkTotal += supplierTotal [i];
	
		if (grandWorkTotal != 0)
		{
			fprintf (pp, 
					 "|                ** Total For Supplier **           |");
			fprintf (pp, "%13.2f|", DOLLARS (grandWorkTotal));
			for (i = 0; i < 5; i++)
			{
				if (supplierTotal [i] != 0L)
					fprintf (pp, "%13.2f|", DOLLARS (supplierTotal [i]));
				else
					fprintf (pp, "             |");
			}
			fprintf (pp, "\n");
		}

		fprintf (pp, "|---------------------------------------------------|");
		fprintf (pp, "-------------|-------------|-------------|-------------|-------------|-------------|\n");

		for (i = 0; i < 5; i++)
		{
			supplierTotal [i] = 0;
			notApprovedSuppler [i] = 0;
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
			notApproved += notApprovedFgnCurr [i];
	
		fprintf (pp, ".LRP4\n");
		fprintf (pp, "|  ** TOTAL NOT APPROVED (FOR CURRENCY) **          |");
		fprintf (pp, "%13.2f|", DOLLARS (notApproved));

		for (i = 0; i < 5; i++)
		{
			if (notApprovedFgnCurr [i] != 0L)
				fprintf (pp, "%13.2f|", DOLLARS (notApprovedFgnCurr [i]));
			else
				fprintf (pp, "             |");

			notApprovedFgnCurr [i] = 0L;
		}
 		fprintf (pp, "\n");

		fprintf (pp, "|---------------------------------------------------|");
		fprintf (pp, "-------------|-------------|-------------|-------------|-------------|-------------|\n");

		grandWorkTotal = 0;
		for (i = 0; i < 5; i++)
			grandWorkTotal += currencyTotal [i];
	
		fprintf (pp, "|  ** TOTAL BALANCE DUE (FOR CURRENCY) **          |");
		fprintf (pp, "%13.2f|", DOLLARS (grandWorkTotal));

		for (i = 0; i < 5; i++)
		{
			if (currencyTotal [i] != 0L)
				fprintf (pp, "%13.2f|", DOLLARS (currencyTotal [i]));
			else
				fprintf (pp, "             |");

			currencyTotal [i] = 0L;
		}
		fprintf (pp, "\n");

		fprintf (pp, "|---------------------------------------------------|");
		fprintf (pp, "-------------|-------------|-------------|-------------|-------------|-------------|\n");

		/*------------------------------------------
		| Print local currency equivalent totals.  |
		------------------------------------------*/
		notApproved = 0.00;
		for (i = 0; i < 5; i++)
		{
			notApproved += notApprovedLocalCurr [i];
			notApprovedGrand [i] += notApprovedLocalCurr [i];
		}
	
		fprintf (pp, ".LRP4\n");
		fprintf (pp, "|  ** TOTAL NOT APPROVED (EQUIV. LOCAL CURRENCY) ** |");
		fprintf (pp, "%13.2f|", DOLLARS (notApproved));

		for (i = 0; i < 5; i++)
		{
			if (notApprovedLocalCurr [i] != 0L)
				fprintf (pp, "%13.2f|", DOLLARS (notApprovedLocalCurr [i]));
			else
				fprintf (pp, "             |");

			notApprovedLocalCurr [i] = 0L;
		}
 		fprintf (pp, "\n");

		fprintf (pp, "|---------------------------------------------------|");
		fprintf (pp, "-------------|-------------|-------------|-------------|-------------|-------------|\n");

		grandWorkTotal = 0;
		for (i = 0; i < 5; i++)
		{
			grandWorkTotal += localCurrTotal [i];
			grandTotal [i] += localCurrTotal [i];
		}
	
		fprintf (pp, "|  ** TOTAL BALANCE DUE (EQUIV. LOCAL CURRENCY) ** |");
		fprintf (pp, "%13.2f|", DOLLARS (grandWorkTotal));

		for (i = 0; i < 5; i++)
		{
			if (localCurrTotal [i] != 0L)
				fprintf (pp, "%13.2f|", DOLLARS (localCurrTotal [i]));
			else
				fprintf (pp, "             |");

			localCurrTotal [i] = 0L;
		}
	    	fprintf (pp, "\n");
	}
}

/*======================
| Print grand totals.  |
======================*/
void
PrintGrandTotal (void)
{
	int	i; 

	fprintf (pp, "=====================================================");
	fprintf (pp, "=====================");
	fprintf (pp, "===============================================================\n");
	notApproved = 0.00;
	for (i = 0; i < 5; i++)
		notApproved += notApprovedGrand [i];
		
	fprintf (pp, ".LRP4\n");
	fprintf (pp, "|  ** GRAND TOTAL NOT APPROVED **                   |");
	fprintf (pp, "%13.2f|", DOLLARS (notApproved));

	for (i = 0; i < 5; i++)
	{
		if (notApprovedGrand [i] != 0L)
			fprintf (pp, "%13.2f|", DOLLARS (notApprovedGrand [i]));
		else
			fprintf (pp, "             |");
	}
 	fprintf (pp, "\n");

	fprintf (pp, "|---------------------------------------------------|");
	fprintf (pp, "-------------|-------------|-------------|-------------|-------------|-------------|\n");

	grandWorkTotal = 0;
	for (i = 0; i < 5; i++)
		grandWorkTotal += grandTotal [i];

	fprintf (pp, "|  ** GRAND TOTAL BALANCE DUE ALL **                |");
	fprintf (pp, "%13.2f|", DOLLARS (grandWorkTotal));

	for (i = 0; i < 5; i++)
	{
		if (grandTotal [i] != 0L)
			fprintf (pp, "%13.2f|", DOLLARS (grandTotal [i]));
		else
			fprintf (pp, "             |");
	}
 	fprintf (pp, "\n");
	fprintf (pp, ".EOF\n");
	pclose (pp);
}

/*======================
| Open Database Files. |
======================*/
void
OpenDB (void)
{
	abc_dbopen ("data");

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	
	open_rec (suin, suin_list, SUIN_NO_FIELDS, "suin_cron");
	open_rec (suhd, suhd_list, SUHD_NO_FIELDS, "suhd_hhsu_hash");
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, "sumr_id_no2");
	open_rec (pocr, pocr_list, POCR_NO_FIELDS, "pocr_id_no");
}

/*=======================
| Close Database Files. |
=======================*/
void
CloseDB (void)
{
	abc_fclose (sumr);
	abc_fclose (suin);
	abc_fclose (suhd);
	abc_fclose (pocr);
	abc_dbclose ("data");
}

void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*======================
| Print Report Heading |
======================*/
int
HeadingOutput (void)
{
	/*-----------------------
	| Open pipe to pformat. |
	-----------------------*/
	if ((pp = popen ("pformat", "w")) == 0)
	{
		sys_err ("Error in pformat During (POPEN)", 1, PNAME);
		shutdown_prog ();
		return (EXIT_FAILURE);
	}
	
	fprintf (pp, ".START%s<%s>\n", DateToString (comm_rec.crd_date), PNAME);
	fprintf (pp, ".LP%d\n", printerNumber);
	fprintf (pp, ".14\n");
	fprintf (pp, ".L140\n");
	fprintf (pp, ".E%s\n", clip (comm_rec.co_name));
	fprintf (pp, ".B1\n");
	fprintf (pp, ".ECREDITORS DETAILED AGEING SCHEDULE\n");

	if (BY_BRANCH)
		fprintf (pp, ".E%s AS AT : %s \n", comm_rec.est_short, SystemTime ());
	else
		fprintf (pp, ".EAS AT : %s \n", SystemTime ());
	
	fprintf (pp, ".B1\n");

	fprintf (pp, ".R=====================================================");
	fprintf (pp, "=====================");
	fprintf (pp, "===============================================================\n");
	fprintf (pp, "Note : '*' Denotes Invoices not approved.\n");

	fprintf (pp, "=====================================================");
	fprintf (pp, "=====================");
	fprintf (pp, "===============================================================\n");

	fprintf (pp, "|   INVOICE             |   DATE   |     P.O.       |");
	fprintf (pp, "                            B A L A N C E   D U E                                  |\n");

	fprintf (pp, "|                       |    OF    |                |");
	fprintf (pp, "-----------------------------------------------------------------------------------|\n");

	fprintf (pp, "|    NUMBER             |  INVOICE |    NUMBER      |");
	fprintf (pp, "     TOTAL   |   CURRENT   |   1 TO 30   |  31 TO 60   |  61 TO 90   |   OVER 90   |\n");

	fprintf (pp, "|---------------------------------------------------|");
	fprintf (pp, "-------------|-------------|-------------|-------------|-------------|-------------|\n");

	fprintf (pp, ".PI12\n");

	return (EXIT_SUCCESS);
}

/*========================
| Print Currency Heading |
========================*/
void
PrintCurrHead (void)
{
	fprintf (pp, "| CURRENCY %s - %-35.35s|", pocr_rec.code, pocr_rec.description);
	fprintf (pp, "             |             |             |             |             |             |\n");
	fprintf (pp, "|                                                   |");
	fprintf (pp, "             |             |             |             |             |             |\n");
}

/*======================
| Print Invoice Line.  |
======================*/
int
PrintLine (void)
{
	int	pip, 
		i; 

	double	balance = 0.0,
	      	loc_balance = 0.0,
	      	prt_balance = 0.0;

	char	approved [2],
			pr_type [3];

	/*----------------------------------------------------
	| For each invoice, print details if dbt - crd <> 0. |
	----------------------------------------------------*/
	balance = suin_rec.amt - suin_rec.amt_paid;

	if (balance == 0.00)
		return (EXIT_FAILURE);
	
	if (!APPROVED)
		strcpy (approved, "*");
	else
		strcpy (approved, " ");

	if (suin_rec.type [0] == '1')
		strcpy (pr_type, "IN");

	else if (suin_rec.type [0] == '2')
		strcpy (pr_type, "CR");

	else if (suin_rec.type [0] == '3')
		strcpy (pr_type, "JN");

	else 
		strcpy (pr_type, "??");

	fprintf (pp, "|%s  %s  %s |%10.10s|     %6.6s     |             |",
			 approved,
			 pr_type,
			 suin_rec.inv_no, 
			 DateToString (suin_rec.date_of_inv),
			 suin_rec.cus_po_no);

	/*---------------------------------------
	| Adjust for multi-currency.            |
	---------------------------------------*/
	if (MCURR)
	{
		if (suin_rec.exch_rate > 0.00)
			loc_balance = balance / suin_rec.exch_rate;
		else
			loc_balance = balance;
	}

	if (MCURR && LOCAL)
		prt_balance = loc_balance;
	else
		prt_balance = balance;

	/*---------------------------------------
	| Flag reset when correct column found. |
	---------------------------------------*/
	pip = 1;
	for (i = 0; i < 5; i++)
	{
		if (pip && (suin_rec.pay_date > age_date [i]))
		{
			fprintf (pp, "%13.2f|", DOLLARS (prt_balance));
			supplierTotal [i] += prt_balance; 
			if (!APPROVED)
				notApprovedSuppler [i] += prt_balance;

			if (MCURR)
			{
				currencyTotal [i] += balance; 
				localCurrTotal [i] += loc_balance; 
				if (!APPROVED)
				{
					notApprovedLocalCurr [i] += balance;
					notApprovedLocalCurr [i] += loc_balance;
				}
			}
			pip = 0;
		}
		else
			fprintf (pp, "             |");
	}
	fprintf (pp, "\n");

	return (EXIT_SUCCESS);
}

/*======================
| Get comm record.     |
======================*/
void
GetComm (void)
{
	int	i, 
		cm, 
		cd,
		cy;

	static	int	days [12] = {31,28,31,30,31,30,31,31,30,31,30,31};

	strcpy (sumr_rec.co_no, comm_rec.co_no);
	strcpy (sumr_rec.est_no, comm_rec.est_no);

	/*------------------------------------------------------
	| Find dates for ageing i.e. end of last three months. |
	------------------------------------------------------*/
	age_date [0] = comm_rec.crd_date; 
	age_date [4] = 0L;

	/*---------------------------
	| Adjust feb for leap year. |
	---------------------------*/
	DateToDMY (comm_rec.crd_date, &cd, &cm, &cy);
	days [1] = ((cy % 4) == 0) ? 29 : 28;

	/*--------------------------------------
	| If day not end of month, adjust day. |
	--------------------------------------*/
	if (cd < days [cm -1])
		age_date [0] += (days [cm -1] - cd);

	for (i = 1; i < 4; i++)
		age_date [i] = age_date [i-1] - days [cm - i + (( (cm-i)<0)*12)];
}

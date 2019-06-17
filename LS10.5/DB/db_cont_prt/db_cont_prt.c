/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: db_cont_prt.c,v 5.1 2001/12/07 03:36:03 scott Exp $
|  Program Name  : (db_cont_prt.c)
|  Program Desc  : (Customer Control report)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 29/06/88         |
|---------------------------------------------------------------------|
| $Log: db_cont_prt.c,v $
| Revision 5.1  2001/12/07 03:36:03  scott
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_cont_prt.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_cont_prt/db_cont_prt.c,v 5.1 2001/12/07 03:36:03 scott Exp $";

#include	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<ml_db_mess.h>

#include 	"schema"

	struct	commRecord	comm_rec;
	struct	cumrRecord	cumr_rec;
	struct	cuhdRecord	cuhd_rec;
	struct	cuinRecord	cuin_rec;

	long	startDate 	= 0L,
			endDate 	= 0L;

	char	systemDate[11],
			branchNumber[3];
	
	FILE	*fout,
			*fin;
	
	struct tm *ts;

	double	openingBalance = 0.00,
			forwardBalance = 0.00,
			closingBalance = 0.00,
			bfBalance[31][5],
			totalBalance[5];

	int		printerNumber = 1;

	int		reportByCompany = FALSE;

#include 	<pr_format3.h>

int	envDbNettUsed = TRUE;
int	envDbMcurr 	  = TRUE;

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void ProcessFile 	(void);
void OpenDB 		(void);
void CloseDB 		(void);
void shutdown_prog 	(void);
void ReportHeading 	(int);
int  check_page 	(void);

int
main (
 int                argc,
 char*              argv[])
{
	int 	i,
			envDbCo;

	char	*sptr;

	sptr = chk_env ("DB_NETT_USED");
	envDbNettUsed = (sptr == (char *)0) ? TRUE : atoi (sptr);

	sptr = chk_env ("DB_CO");
	envDbCo = (sptr == (char *)0) ? 0 : atoi (sptr);

	/*----------------------------
	| Multi-Currency Customers ? |
	----------------------------*/
	sptr = chk_env ("DB_MCURR");
	envDbMcurr = (sptr == (char *)0) ? FALSE : atoi (sptr);

	if (argc < 2)
	{
		/*------------------------------------------------------
		| Usage %s printer-no [ Optional C(ompany Report) ]\n\r|
		------------------------------------------------------*/
		print_at (0,0, mlDbMess700, argv[0]);
        return (EXIT_FAILURE);
	}
	printerNumber = atoi (argv[1]);

	reportByCompany = (argc > 2) ? TRUE : FALSE;

	OpenDB ();

	dsp_screen ("Printing Customer Control Report.",
				comm_rec.co_no,comm_rec.co_name);

	strcpy (branchNumber, (envDbCo) ? comm_rec.est_no : " 0");

	strcpy (systemDate, DateToString (TodaysDate ()));

	ReportHeading (printerNumber);

	openingBalance = 0.00;
	forwardBalance = 0.00;

	strcpy (cumr_rec.co_no, comm_rec.co_no);
	strcpy (cumr_rec.est_no , (reportByCompany) ? "  " : branchNumber);

	startDate 	= MonthStart (comm_rec.dbt_date);
	endDate 	= MonthEnd	 (comm_rec.dbt_date);

	cc = find_rec ("cumr", &cumr_rec, GTEQ, "r");
	while (!cc && !strcmp (cumr_rec.co_no,comm_rec.co_no) &&
		     (!strcmp(cumr_rec.est_no, branchNumber) || reportByCompany))
	{
		dsp_process ("Customer : ",cumr_rec.dbt_no);
		ProcessFile ();
		cc = find_rec ("cumr", &cumr_rec, NEXT, "r");
	}

	pr_format (fin, fout, "HEAD3",  1, openingBalance);
	pr_format (fin, fout, "HEAD4",  0, 0);
	closingBalance += openingBalance;
	for (i = 0; i < DaysInMonth (comm_rec.dbt_date); i++)
	{
		if (i == 0)
			fprintf (fout, "|%-29.29s","CURRENT MONTH TRANSACTIONS");
		else 
			fprintf (fout, "|%-29.29s"," ");

		fprintf (fout, "|%10.10s",DateToString (startDate + (long) i));

		if (bfBalance[i][0] != 0.00)
			fprintf (fout, "| %16.2f ",DOLLARS (bfBalance[i][0]));
		else
			fprintf (fout, "|                  ");

		if (bfBalance[i][1] != 0.00)
			fprintf (fout, "| %16.2f ",DOLLARS (bfBalance[i][1]));
		else
			fprintf (fout, "|                  ");

		if (bfBalance[i][2] != 0.00)
			fprintf (fout, "| %16.2f ",DOLLARS (bfBalance[i][2]));
		else
			fprintf (fout, "|                  ");

		if (bfBalance[i][3] != 0.00)
			fprintf (fout, "| %16.2f ",DOLLARS (bfBalance[i][3]));
		else
			fprintf (fout, "|                  ");

		closingBalance += bfBalance[i][4];
		totalBalance[0] += bfBalance[i][0];
		totalBalance[1] += bfBalance[i][1];
		totalBalance[2] += bfBalance[i][2];
		totalBalance[3] += bfBalance[i][3];

		fprintf (fout, "| %16.2f |\n", DOLLARS (closingBalance));
	}
	closingBalance += forwardBalance;
	pr_format (fin, fout, "HEAD6", 1, forwardBalance);
	pr_format (fin, fout, "HEAD4",  0, 0);
	pr_format (fin, fout, "HEAD5", 1, totalBalance[0]);
	pr_format (fin, fout, "HEAD5", 2, totalBalance[1]);
	pr_format (fin, fout, "HEAD5", 3, totalBalance[2]);
	pr_format (fin, fout, "HEAD5", 4, totalBalance[3]);
	pr_format (fin, fout, "HEAD5", 5, closingBalance);
	fprintf (fout,".EOF\n");
	pclose (fout);
	fclose (fin);

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

void
ProcessFile (void)
{
	double	amt = 0.00;
	int		day = 0;

	cuin_rec.date_of_inv = 0L;
	cuin_rec.hhcu_hash = cumr_rec.hhcu_hash;
	strcpy (cuin_rec.est, "  ");

	/*--------------------------------------
	| Process Invoices/ Credits/ Journals. |
	--------------------------------------*/
	cc = find_rec ("cuin", &cuin_rec, GTEQ, "r");
	while (!cc && cumr_rec.hhcu_hash == cuin_rec.hhcu_hash) 
	{
		amt = (envDbNettUsed) ? cuin_rec.amt - cuin_rec.disc 
						  	  : cuin_rec.amt;

		if (cuin_rec.exch_rate != 0.00)
			amt = amt / cuin_rec.exch_rate;

		if (cuin_rec.date_of_inv < startDate)
		{
			openingBalance += amt;
			cc = find_rec ("cuin", &cuin_rec, NEXT, "r");
			continue;
		}
		if (cuin_rec.date_of_inv > endDate)
		{
			forwardBalance += amt;
			cc = find_rec ("cuin", &cuin_rec, NEXT, "r");
			continue;
		}

		DateToDMY (cuin_rec.date_of_inv, &day, NULL, NULL);

		if (cuin_rec.type[0] == '1')
			bfBalance[day - 1][0] += amt;

		if (cuin_rec.type[0] == '2')
			bfBalance[day - 1][1] += amt * -1;

		if (cuin_rec.type[0] == '3')
			bfBalance[day - 1][3] += amt;

		bfBalance[day - 1][4] += amt;
		cc = find_rec ("cuin", &cuin_rec, NEXT, "r");
	}

	/*----------------------------
	| Process Cheques/ Journals. |
	----------------------------*/
	cuhd_rec.hhcu_hash = cumr_rec.hhcu_hash;
	strcpy (cuhd_rec.receipt_no, "        ");
	cuhd_rec.index_date	=	0L;
		
	cc = find_rec ("cuhd", &cuhd_rec, GTEQ, "r");
	while (!cc && cumr_rec.hhcu_hash == cuhd_rec.hhcu_hash) 
	{
		if (envDbMcurr)
		{
			amt = cuhd_rec.loc_amt_paid - cuhd_rec.exch_variance;
		}
		else
		{
			amt = cuhd_rec.loc_amt_paid;
		}

		if (cuhd_rec.date_payment < startDate)
		{
			openingBalance -= amt;
			cc = find_rec ("cuhd", &cuhd_rec, NEXT, "r");
			continue;
		}
		if (cuhd_rec.date_payment > endDate)
		{
			forwardBalance -= amt;
			cc = find_rec ("cuhd", &cuhd_rec, NEXT, "r");
			continue;
		}
		DateToDMY (cuhd_rec.date_payment, &day, NULL, NULL);
		
		if (cuhd_rec.type[0] == '1')
			bfBalance[day - 1][2] += amt;

		if (cuhd_rec.type[0] == '2')
			bfBalance[day - 1][3] -= amt;

		bfBalance[day - 1][4] -= amt;
		cc = find_rec ("cuhd", &cuhd_rec, NEXT, "r");
	}
}

/*======================
| Open database Files. |
======================*/
void
OpenDB (void)
{
	abc_dbopen ("data");

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec ("cuin", cuin_list, CUIN_NO_FIELDS, "cuin_cron");
	open_rec ("cuhd", cuhd_list, CUHD_NO_FIELDS, "cuhd_id_no");
	open_rec ("cumr", cumr_list, CUMR_NO_FIELDS, "cumr_id_no");
}

/*=======================
| Close database Files. |
=======================*/
void
CloseDB (void)
{
	abc_fclose ("cumr");
	abc_fclose ("cuin");
	abc_fclose ("cuhd");
	abc_dbclose ("data");
}

void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

void
ReportHeading (
 int                prnt_no)
{
	/*----------------------
	| Open pipe to pformat | 
 	----------------------*/
	if ((fout = popen ("pformat","w")) == 0) 
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);

	if (!(fin = pr_open ("db_cont_prt.p")))
		sys_err ("Error in db_cont_prt.p During (FOPEN)", errno, PNAME);

	/*---------------------------------
	| Start output to standard print. |
	---------------------------------*/
	fprintf (fout,".START%s<%s>\n", DateToString (comm_rec.dbt_date), PNAME);
	fprintf (fout,".SO\n");
	fprintf (fout,".LP%d\n",prnt_no);
	fprintf (fout,".PI12\n");
	fprintf (fout,".10\n");
	fprintf (fout,".L150\n");
	fprintf (fout,".E%s\n",clip(comm_rec.co_name));
	fprintf (fout,".ECUSTOMER CONTROL REPORT\n");
	fprintf (fout,".B1\n");
	if (strcmp (branchNumber, " 0") && !reportByCompany)
		fprintf (fout,".EBRANCH : %s\n",comm_rec.est_name);
	else
		fprintf (fout,".B1\n");
		
	fprintf (fout,".EAS AT : %s\n", SystemTime());

	pr_format (fin, fout, "RULER", 0,0);
	pr_format (fin, fout, "RULER2",0,0);
	pr_format (fin, fout, "HEAD1", 0,0);
	pr_format (fin, fout, "HEAD2", 0,0);
}

int
check_page (void)
{
	return (EXIT_SUCCESS);
}

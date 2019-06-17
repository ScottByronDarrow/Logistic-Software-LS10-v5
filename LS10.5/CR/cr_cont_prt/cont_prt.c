/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: cont_prt.c,v 5.3 2001/08/23 11:28:25 scott Exp $
|  Program Name  : (cr_cont_prt.c)
|  Program Desc  : (Customer Control report)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 29/06/88         |
|---------------------------------------------------------------------|
| $Log: cont_prt.c,v $
| Revision 5.3  2001/08/23 11:28:25  scott
| Updated from scotts machine
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cont_prt.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CR/cr_cont_prt/cont_prt.c,v 5.3 2001/08/23 11:28:25 scott Exp $";

#include	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<ml_std_mess.h>

#define		MCURR		(multiCurrency [0] == 'Y')
#define		INV_APP		0
#define		INV_NAPP	1
#define		CREDIT		2
#define		CHEQUE		3
#define		EX_VAR		4
#define		JOURNAL		5
#define		TOTAL		6

#include	"schema"

struct commRecord	comm_rec;
struct suinRecord	suin_rec;
struct suhdRecord	suhd_rec;
struct sumrRecord	sumr_rec;

	int	dtls_cnt;
	
	long	startDate = 0L;

	char	multiCurrency [2],
			branchNo [3];
	
	long	ageingDate [2];

	FILE	*fout;
	
	double	forwardBalance = 0.00,
			closingBalance = 0.00,
			broughtForward [32] [7],
			totalBalance [7];

	int	printerNumber = 1;

static	int	days [14] = {0,31,28,31,30,31,30,31,31,30,31,30,31,31};

/*===========================
| Local function prototypes |
===========================*/
void	proc_file		(void);
void	OpenDB			(void);
void	CloseDB		(void);
void	shutdown_prog	(void);
void	head			(void);
void	ReadComm		(void);


int
main (
 int	argc,
 char *	argv [])
{
	int		i,
			mon;
	long	wk_date2 = 0L;

	int		envDbCo;

 	envDbCo = atoi (get_env ("CR_CO"));

	if (argc != 2)
	{
		/*---------------
		| Usage %s LPNO |
		---------------*/
		print_at (0,0, ML (mlStdMess036), argv [0]);
		return (EXIT_FAILURE);
	}
	printerNumber = atoi (argv [1]);

	sprintf (multiCurrency, "%-1.1s", get_env ("CR_MCURR"));

	OpenDB ();
	ReadComm ();

	dsp_screen ("Printing Suppliers Control Report.",
					comm_rec.co_no,comm_rec.co_name);

	strcpy (branchNo, (envDbCo) ? comm_rec.est_no : " 0");

	head ();

	forwardBalance = 0.00;

	strcpy (sumr_rec.co_no, comm_rec.co_no);
	strcpy (sumr_rec.est_no , branchNo);

	startDate = MonthStart (comm_rec.crd_date);

	DateToDMY (comm_rec.crd_date, NULL, &mon, NULL);

	cc = find_rec (sumr, &sumr_rec, GTEQ, "r");
	while (!cc && !strcmp (sumr_rec.co_no,comm_rec.co_no) &&
		     !strcmp (sumr_rec.est_no, branchNo))
	{
		dsp_process ("Suppliers : ",sumr_rec.crd_no);
		proc_file ();
		cc = find_rec (sumr, &sumr_rec, NEXT, "r");
	}

	wk_date2 = MonthStart (comm_rec.crd_date);

	fprintf (fout,"| OPENING BALANCE    ");
	fprintf (fout,"|          ");

	if (broughtForward [0] [INV_NAPP] != 0.00)
		fprintf (fout, "| %14.2f ",DOLLARS (broughtForward [0] [INV_NAPP]));
	else
		fprintf (fout, "|                ");

	if (broughtForward [0] [INV_APP] != 0.00)
		fprintf (fout, "| %14.2f ",DOLLARS (broughtForward [0] [INV_APP]));
	else
		fprintf (fout, "|                ");

	if (broughtForward [0] [CREDIT] != 0.00)
		fprintf (fout, "| %14.2f ",DOLLARS (broughtForward [0] [CREDIT]));
	else
		fprintf (fout, "|                ");

	if (broughtForward [0] [CHEQUE] != 0.00)
		fprintf (fout, "| %14.2f ",DOLLARS (broughtForward [0] [CHEQUE]));
	else
		fprintf (fout, "|                ");

	if (broughtForward [0] [EX_VAR] != 0.00)
		fprintf (fout, "| %14.2f ",DOLLARS (broughtForward [0] [EX_VAR]));
	else
		fprintf (fout, "|                ");

	if (broughtForward [0] [JOURNAL] != 0.00)
		fprintf (fout, "| %14.2f ",DOLLARS (broughtForward [0] [JOURNAL]));
	else
		fprintf (fout, "|                ");

	if (broughtForward [0] [TOTAL] != 0.00)
		fprintf (fout, "| %14.2f |\n",DOLLARS (broughtForward [0] [TOTAL]));
	else
		fprintf (fout, "|                |\n");

	fprintf (fout,"|--------------------");
	fprintf (fout,"|----------");
	fprintf (fout,"|----------------");
	fprintf (fout,"|----------------");
	fprintf (fout,"|----------------");
	fprintf (fout,"|----------------");
	fprintf (fout,"|----------------");
	fprintf (fout,"|----------------");
	fprintf (fout,"|----------------|\n");

	closingBalance = broughtForward [0] [TOTAL];
	totalBalance [INV_NAPP] = broughtForward [0] [INV_NAPP];
	totalBalance [INV_APP]  = broughtForward [0] [INV_APP];
	totalBalance [CREDIT]   = broughtForward [0] [CREDIT];
	totalBalance [CHEQUE]   = broughtForward [0] [CHEQUE];
	totalBalance [EX_VAR]   = broughtForward [0] [EX_VAR];
	totalBalance [JOURNAL]  = broughtForward [0] [JOURNAL];

	for (i = 1; i <= days [mon]; i++)
	{
		if (i == 1)
			fprintf (fout, "|%-20.20s","CURRENT MONTH TRANS.");
		else 
			fprintf (fout, "|%-20.20s"," ");

	 	fprintf (fout, "|%-10.10s",DateToString ((wk_date2 - 1L) + (long) i));

		if (broughtForward [i] [INV_NAPP] != 0.00)
			fprintf (fout, "| %14.2f ",DOLLARS (broughtForward [i] [INV_NAPP]));
		else
			fprintf (fout, "|                ");

		if (broughtForward [i] [INV_APP] != 0.00)
			fprintf (fout, "| %14.2f ",DOLLARS (broughtForward [i] [INV_APP]));
		else
			fprintf (fout, "|                ");

		if (broughtForward [i] [CREDIT] != 0.00)
			fprintf (fout, "| %14.2f ",DOLLARS (broughtForward [i] [CREDIT]));
		else
			fprintf (fout, "|                ");

		if (broughtForward [i] [CHEQUE] != 0.00)
			fprintf (fout, "| %14.2f ",DOLLARS (broughtForward [i] [CHEQUE]));
		else
			fprintf (fout, "|                ");

		if (broughtForward [i] [EX_VAR] != 0.00)
			fprintf (fout, "| %14.2f ",DOLLARS (broughtForward [i] [EX_VAR]));
		else
			fprintf (fout, "|                ");

		if (broughtForward [i] [JOURNAL] != 0.00)
			fprintf (fout, "| %14.2f ",DOLLARS (broughtForward [i] [JOURNAL]));
		else
			fprintf (fout, "|                ");


		closingBalance += broughtForward [i] [TOTAL];
		totalBalance [INV_NAPP] += broughtForward [i] [INV_NAPP];
		totalBalance [INV_APP]  += broughtForward [i] [INV_APP];
		totalBalance [CREDIT]   += broughtForward [i] [CREDIT];
		totalBalance [CHEQUE]   += broughtForward [i] [CHEQUE];
		totalBalance [EX_VAR]   += broughtForward [i] [EX_VAR];
		totalBalance [JOURNAL]  += broughtForward [i] [JOURNAL];

		fprintf (fout, "| %14.2f |\n", DOLLARS (closingBalance));
	}
	closingBalance += forwardBalance;

	fprintf (fout,"| FWD DUE INVOICES   ");
	fprintf (fout,"|          ");
	fprintf (fout,"|                ");
	fprintf (fout,"|                ");
	fprintf (fout,"|                ");
	fprintf (fout,"|                ");
	fprintf (fout,"|                ");
	fprintf (fout,"|                ");
	fprintf (fout,"| %14.2f |\n", DOLLARS (forwardBalance));

	fprintf (fout,"|--------------------");
	fprintf (fout,"|----------");
	fprintf (fout,"|----------------");
	fprintf (fout,"|----------------");
	fprintf (fout,"|----------------");
	fprintf (fout,"|----------------");
	fprintf (fout,"|----------------");
	fprintf (fout,"|----------------");
	fprintf (fout,"|----------------|\n");

	fprintf (fout,"| SUB TOTALS.        ");
	fprintf (fout,"|          ");
	fprintf (fout,"| %14.2f ",
			     DOLLARS (totalBalance [INV_NAPP] - broughtForward [0] [INV_NAPP]));
	fprintf (fout,"| %14.2f ",
			     DOLLARS (totalBalance [INV_APP] - broughtForward [0] [INV_APP]));
	fprintf (fout,"| %14.2f ",
			     DOLLARS (totalBalance [CREDIT ] - broughtForward [0] [CREDIT ]));
	fprintf (fout,"| %14.2f ",
			     DOLLARS (totalBalance [CHEQUE ] - broughtForward [0] [CHEQUE ]));
	fprintf (fout,"| %14.2f ",
			     DOLLARS (totalBalance [EX_VAR ] - broughtForward [0] [EX_VAR ]));
	fprintf (fout,"| %14.2f ",
			     DOLLARS (totalBalance [JOURNAL] - broughtForward [0] [JOURNAL]));
	fprintf (fout,"|                |\n");

	fprintf (fout,"|--------------------");
	fprintf (fout,"|----------");
	fprintf (fout,"|----------------");
	fprintf (fout,"|----------------");
	fprintf (fout,"|----------------");
	fprintf (fout,"|----------------");
	fprintf (fout,"|----------------");
	fprintf (fout,"|----------------");
	fprintf (fout,"|----------------|\n");
	
	fprintf (fout,"| CLOSING BALANCE    ");
	fprintf (fout,"|          ");
	fprintf (fout,"| %14.2f ", DOLLARS (totalBalance [INV_NAPP]));
	fprintf (fout,"| %14.2f ", DOLLARS (totalBalance [INV_APP]));
	fprintf (fout,"| %14.2f ", DOLLARS (totalBalance [CREDIT]));
	fprintf (fout,"| %14.2f ", DOLLARS (totalBalance [CHEQUE]));
	fprintf (fout,"| %14.2f ", DOLLARS (totalBalance [EX_VAR]));
	fprintf (fout,"| %14.2f ", DOLLARS (totalBalance [JOURNAL]));
	fprintf (fout,"| %14.2f |\n",DOLLARS (closingBalance));


	fprintf (fout,".EOF\n");
	pclose (fout);
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

void
proc_file (
 void)
{
	double	amt = 0.00;
	int		day = 0;

	suin_rec.hhsu_hash = sumr_rec.hhsu_hash;
	suin_rec.date_of_inv = 0L;
	strcpy (suin_rec.est, "  ");

	/*--------------------------------------
	| Process Invoices/ Credits/ Journals. |
	--------------------------------------*/
	cc = find_rec (suin, &suin_rec, GTEQ, "r");
	while (!cc && sumr_rec.hhsu_hash == suin_rec.hhsu_hash) 
	{
		if (MCURR)
		{
			if (suin_rec.exch_rate > 0.00)
				amt = suin_rec.amt / suin_rec.exch_rate;
			else
				amt = suin_rec.amt;
		}
		else
			amt = suin_rec.amt;

		if (suin_rec.date_of_inv < startDate)
		{
			if (suin_rec.type [0] == '1')
			{
				if (suin_rec.approved [0] == 'Y')
					broughtForward [0] [INV_APP] 	+= amt;
				else
					broughtForward [0] [INV_NAPP] += amt;
			}
			if (suin_rec.type [0] == '2')
				broughtForward [0] [CREDIT] 	+= (amt * -1);

			if (suin_rec.type [0] == '3')
				broughtForward [0] [JOURNAL] += amt;
			
			broughtForward [0] [TOTAL] += amt;

			cc = find_rec (suin, &suin_rec, NEXT, "r");
			continue;
		}
		if (suin_rec.date_of_inv > ageingDate [0])
		{
			forwardBalance += amt;
			cc = find_rec (suin, &suin_rec, NEXT, "r");
			continue;
		}
		DateToDMY (suin_rec.date_of_inv, &day, NULL, NULL);

		if (suin_rec.type [0] == '1')
		{
			if (suin_rec.approved [0] == 'Y')
				broughtForward [day] [INV_APP] 	+= amt;
			else
				broughtForward [day] [INV_NAPP] 	+= amt;
		}

		if (suin_rec.type [0] == '2')
			broughtForward [day] [CREDIT] += (amt * -1);

		if (suin_rec.type [0] == '3')
			broughtForward [day] [JOURNAL] += amt;

		broughtForward [day] [TOTAL] += amt;
		cc = find_rec (suin, &suin_rec, NEXT, "r");
	}

	/*----------------------------
	| Process Cheques/ Journals. |
	----------------------------*/
	suhd_rec.hhsu_hash = sumr_rec.hhsu_hash;
	strcpy (suhd_rec.cheq_no, "             ");
		
	cc = find_rec (suhd, &suhd_rec, GTEQ, "r");
	while (!cc && sumr_rec.hhsu_hash == suhd_rec.hhsu_hash) 
	{
		amt = suhd_rec.loc_amt_paid;
			  
		if (suhd_rec.date_payment < startDate)
		{
			broughtForward [0] [CHEQUE] 	+= amt;
			broughtForward [0] [TOTAL]  	-= amt;
			broughtForward [0] [EX_VAR] 	+= suhd_rec.exch_variance;
			broughtForward [0] [TOTAL] 	-= suhd_rec.exch_variance;
			cc = find_rec (suhd, &suhd_rec, NEXT, "r");
			continue;
		}
		if (suhd_rec.date_payment > ageingDate [0])
		{
			forwardBalance -= amt;
			forwardBalance -= suhd_rec.exch_variance;
			cc = find_rec (suhd, &suhd_rec, NEXT, "r");
			continue;
		}

		DateToDMY (suhd_rec.date_payment, &day, NULL, NULL);
		
		broughtForward [day] [EX_VAR] += suhd_rec.exch_variance;
		broughtForward [day] [TOTAL] 	-= suhd_rec.exch_variance;
		broughtForward [day] [CHEQUE] += amt;
		broughtForward [day] [TOTAL] 	-= amt;

		cc = find_rec (suhd, &suhd_rec, NEXT, "r");
	}
}
/*======================
| Open database Files. |
======================*/
void
OpenDB (
 void)
{
	abc_dbopen ("data");

	open_rec (suin, suin_list, SUIN_NO_FIELDS, "suin_cron");
	open_rec (suhd, suhd_list, SUHD_NO_FIELDS, "suhd_id_no");
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, "sumr_id_no2");
}

/*=======================
| Close database Files. |
=======================*/
void
CloseDB (
 void)
{
	abc_fclose (sumr);
	abc_fclose (suin);
	abc_fclose (suhd);
	abc_dbclose ("data");
}

void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

void
head (
 void)
{
	/*----------------------
	| Open pipe to pformat | 
 	----------------------*/
	if ((fout = popen ("pformat","w")) == 0) 
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);

	/*---------------------------------
	| Start output to standard print. |
	---------------------------------*/

	fprintf (fout,".START%s<%s>\n", DateToString (comm_rec.crd_date), PNAME);
	fprintf (fout,".SO\n");
	fprintf (fout,".LP%d\n",printerNumber);
	fprintf (fout,".PI12\n");
	fprintf (fout,".10\n");
	fprintf (fout,".L158\n");
	fprintf (fout,".E%s\n",clip (comm_rec.co_name));
	fprintf (fout,".ESUPPLIER CONTROL REPORT\n");
	fprintf (fout,".B1\n");
	if (strcmp (branchNo, " 0"))
		fprintf (fout,".EBRANCH : %s\n",comm_rec.est_name);
	else
		fprintf (fout,".B1\n");
		
	fprintf (fout,".EAS AT : %s\n", SystemTime ());

	fprintf (fout,"=====================");
	fprintf (fout,"===========");
	fprintf (fout,"=================");
	fprintf (fout,"=================");
	fprintf (fout,"=================");
	fprintf (fout,"=================");
	fprintf (fout,"=================");
	fprintf (fout,"=================");
	fprintf (fout,"==================\n");

	fprintf (fout,".R=====================");
	fprintf (fout,"===========");
	fprintf (fout,"=================");
	fprintf (fout,"=================");
	fprintf (fout,"=================");
	fprintf (fout,"=================");
	fprintf (fout,"=================");
	fprintf (fout,"=================");
	fprintf (fout,"==================\n");

	fprintf (fout,"|                    ");
	fprintf (fout,"| TRAN DATE");
	fprintf (fout,"|UNAPPROVED IN/CR");
	fprintf (fout,"|    INVOICES    ");
	fprintf (fout,"|    CREDITS     ");
	fprintf (fout,"|    CHEQUES     ");
	fprintf (fout,"| EXCH VARIATION ");
	fprintf (fout,"|    JOURNALS    ");
	fprintf (fout,"|     TOTALS     |\n");

	fprintf (fout,"|====================");
	fprintf (fout,"|==========");
	fprintf (fout,"|================");
	fprintf (fout,"|================");
	fprintf (fout,"|================");
	fprintf (fout,"|================");
	fprintf (fout,"|================");
	fprintf (fout,"|================");
	fprintf (fout,"|================|\n");

	fprintf (fout,".PI12\n");
}

/*============================================ 
| Get common info from commom database file. |
============================================*/
void
ReadComm (
 void)
{
	int	cm,		/* Current debtors month	*/
		cd,		/* Current debtors day		*/
		cy;		/* Current debtors year		*/

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	/*------------------------------------------------------
	| Find dates for ageing i.e. end of last three months. |
	------------------------------------------------------*/
	ageingDate [0] = comm_rec.crd_date; 
	DateToDMY (comm_rec.crd_date, &cd, &cm, &cy);

	/*---------------------------
	| Adjust feb for leap year. |
	---------------------------*/
	days [2] = (cy % 4) == 0 ? 29 : 28;

	/*--------------------------------------
	| If day not end of month, adjust day. |
	--------------------------------------*/
	if (cd < days [cm])
		ageingDate [0] += (days [cm] - cd);
}

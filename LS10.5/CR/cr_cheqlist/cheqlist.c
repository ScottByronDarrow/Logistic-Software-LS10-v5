/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: cheqlist.c,v 5.2 2001/08/09 08:51:34 scott Exp $
|  Program Name  : (cr_cheqlist.c) 
|  Program Desc  : (Suppliers Cheque Listings)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 27/01/87         |
|---------------------------------------------------------------------|
| $Log: cheqlist.c,v $
| Revision 5.2  2001/08/09 08:51:34  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:01:12  scott
| RELEASE 5.0
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cheqlist.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CR/cr_cheqlist/cheqlist.c,v 5.2 2001/08/09 08:51:34 scott Exp $";

#include	<pslscr.h>
#include	<dsp_screen.h>
#include	<dsp_process2.h>
#include	<ml_std_mess.h>
#include	<ml_cr_mess.h>

#define	MCURR				 (multiCurrency [0] == 'Y')
#define	ChequeList			 (programRunType [0] == 'C')
#define	UnpresentedCheque	 (programRunType [0] == 'U')
#define	PresentedCheque		 (programRunType [0] == 'P')

	/*=================================================
	| The Following are needed for branding Routines. |
	=================================================*/
	char	multiCurrency [2];
	char	programRunType [2];

#include	"schema"

struct commRecord	comm_rec;
struct suhpRecord	suhp_rec;
struct suhtRecord	suht_rec;

	char *fifteenSpaces	=	"               ";
	double	grandTotal = 0.00;
	double	grandLocal = 0.00;
	
	FILE	*pp;
	char	branchNo [3];

/*===========================
| Local function prototypes |
===========================*/
void	OpenDB			 (void);
void	CloseDB			 (void);
void	shutdown_prog	 (void);
void	HeadingSection	 (void);
void	PrintTotal		 (void);
void	StartReport		 (int);
void	ProcessFile		 (void);
void	EndReport		 (void);

/*==================
| Main Processing. |
==================*/
int
main (
 int	argc,
 char *	argv [])
{
	if (argc < 3)
	{
		print_at (0,0,mlCrMess703,argv [0]);
		return (EXIT_FAILURE);
	}

	OpenDB ();

	sprintf (programRunType,"%-1.1s", argv [2]);
	
	if (!ChequeList && !UnpresentedCheque && !PresentedCheque)
	{
		print_at (0,0,mlCrMess703,argv [0]);
		return (EXIT_FAILURE);
	}
	if (ChequeList)	
		dsp_screen (ML ("Printing Supplier Cheque History."),comm_rec.co_no,comm_rec.co_name);
	if (UnpresentedCheque)
		dsp_screen (ML ("Printing Supplier Unpresented Cheques."),comm_rec.co_no,comm_rec.co_name);
	if (PresentedCheque)
		dsp_screen (ML ("Printing Supplier Presented Cheques."),comm_rec.co_no,comm_rec.co_name);

	sprintf (multiCurrency, "%-1.1s", get_env ("CR_MCURR"));

	StartReport (atoi (argv [1]));

	strcpy (suhp_rec.co_no, comm_rec.co_no);
	strcpy (suhp_rec.cheq_no, fifteenSpaces);

	cc = find_rec (suhp, &suhp_rec, GTEQ, "r");
	while (!cc && !strcmp (suhp_rec.co_no, comm_rec.co_no))
	{
		if (ChequeList && suhp_rec.stat_flag [0] == 'D')
		{
			cc = find_rec (suhp, &suhp_rec, NEXT, "r");
			continue;
		}
		if (UnpresentedCheque && suhp_rec.presented [0] == 'Y')
		{
			cc = find_rec (suhp, &suhp_rec, NEXT, "r");
			continue;
		}
		if (PresentedCheque && suhp_rec.presented [0] != 'Y')
		{
			cc = find_rec (suhp, &suhp_rec, NEXT, "r");
				continue;
		}
		HeadingSection ();

		if (ChequeList)
		{
			ProcessFile ();
			PrintTotal ();
		}

		grandTotal += suhp_rec.tot_amt_paid;
		grandLocal += suhp_rec.loc_amt_paid;

		dsp_process ("Cheque : ",suhp_rec.cheq_no);
		cc = find_rec (suhp, &suhp_rec, NEXT, "r");
	}
	EndReport ();
	pclose (pp);
	shutdown_prog ();
	return (EXIT_SUCCESS);
}
/*======================
| Open database Files. |
======================*/
void
OpenDB (
 void)
{
	abc_dbopen ("data");

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (suhp, suhp_list, SUHP_NO_FIELDS, "suhp_id_no");
	open_rec (suht, suht_list, SUHT_NO_FIELDS, "suht_id_no");
}

/*=======================
| Close database Files. |
=======================*/
void
CloseDB (
 void)
{
	abc_fclose (suhp);
	abc_fclose (suht);
	abc_dbclose ("data");
}

void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*========================
| Print Pheque headings. |
========================*/
void
HeadingSection (
 void)
{
	fprintf (pp, "|%-15.15s", suhp_rec.cheq_no);
	fprintf (pp, "|%-10.10s", DateToString (suhp_rec.date_payment));
	fprintf (pp, "|%-40.40s",suhp_rec.payee_name);
	fprintf (pp, "|%-20.20s",suhp_rec.narrative);
	fprintf (pp, "|%-5.5s ",suhp_rec.bank_id);
	if (suhp_rec.pay_type [0] == '1')
		fprintf (pp, "| MANUAL CHEQ ");

	else if (suhp_rec.pay_type [0] == '2')
		fprintf (pp, "| DRAFT CHEQUE");

	else if (suhp_rec.pay_type [0] == '3')
		fprintf (pp, "| DEPOSIT CHQ ");

	else if (suhp_rec.pay_type [0] == '4')
		fprintf (pp, "|DEPOSIT DRAFT");

	else if (suhp_rec.pay_type [0] == '5')
		fprintf (pp, "|SUNDRY CHEQUE");

	else if (suhp_rec.pay_type [0] == '6')
		fprintf (pp, "|COMPUT CHEQUE");

	else 
		fprintf (pp, "|?????????????");
	
	if (ChequeList)
	{
		fprintf (pp, "|    ");
		if (MCURR)
	 		fprintf (pp,"|              |              |\n");
		else
	 		fprintf (pp,"|             |\n");
	}
	else
	{
		if (MCURR)
		{
			fprintf (pp,"|%13.2f |%13.2f |\n", 
					DOLLARS (suhp_rec.tot_amt_paid),
					DOLLARS (suhp_rec.loc_amt_paid));
		}
		else
		{
			fprintf (pp,"|%13.2f |\n",
					DOLLARS (suhp_rec.tot_amt_paid));
		}
	}
}

/*=========================
| Print total for cheque. |
=========================*/
void
PrintTotal (
 void)
{
	fprintf (pp, "|               ");
	fprintf (pp, "|          ");
	fprintf (pp, "| T O T A L   OF   C H E Q U E           ");
	fprintf (pp, "|                    ");
	fprintf (pp, "|      ");
	fprintf (pp, "|             ");
	fprintf (pp, "|    ");
	if (MCURR)
	{
		fprintf (pp,"|%13.2f |%13.2f |\n", 
				DOLLARS (suhp_rec.tot_amt_paid),
				DOLLARS (suhp_rec.loc_amt_paid));
	}
	else
	{
		fprintf (pp,"|%13.2f |\n",DOLLARS (suhp_rec.tot_amt_paid));
	}

	fprintf (pp, "|---------------");
	fprintf (pp, "|----------");
	fprintf (pp, "|----------------------------------------");
	fprintf (pp, "|--------------------");
	fprintf (pp, "|------");
	fprintf (pp, "|-------------");
	fprintf (pp, "|----");
	if (MCURR)
	 	fprintf (pp,"|--------------|--------------|\n");
	else
	 	fprintf (pp,"|--------------|\n");
}

/*============================
| Print report headings etc. | 
============================*/
void
StartReport (
 int prnt_no)
{
	/*----------------------
	| Open pipe to pformat | 
 	----------------------*/
	if ( (pp = popen ("pformat","w")) == NULL)
		sys_err ("Error in pformat During (POPEN)", cc, PNAME);

	/*---------------------------------
	| Start output to standard print. |
	---------------------------------*/
	fprintf (pp,".START%s<%s>\n", DateToString (comm_rec.crd_date), PNAME);
	fprintf (pp,".LP%d\n",prnt_no);
	fprintf (pp,".11\n");
	fprintf (pp,".L152\n");
	if (ChequeList)
		fprintf (pp,".ECHEQUE LISTING\n");

	if (UnpresentedCheque)
		fprintf (pp,".EUNPRESENTED CHEQUE LISTING\n");

	if (PresentedCheque)
		fprintf (pp,".EPRESENTED CHEQUE LISTING\n");

	fprintf (pp,".E%s\n",comm_rec.co_name);
	fprintf (pp,".B1\n");
	fprintf (pp,".EAS AT : %s\n", SystemTime ());

	fprintf (pp, ".R================");
	fprintf (pp, "===========");
	fprintf (pp, "=========================================");
	fprintf (pp, "=====================");
	fprintf (pp, "=======");
	fprintf (pp, "==============");
	if (ChequeList)
		fprintf (pp, "=====");

	if (MCURR)
		fprintf (pp,"===============================\n");
	else
		fprintf (pp,"===============\n");
	
	fprintf (pp, "================");
	fprintf (pp, "===========");
	fprintf (pp, "=========================================");
	fprintf (pp, "=====================");
	fprintf (pp, "=======");
	fprintf (pp, "==============");
	if (ChequeList)
		fprintf (pp, "=====");

	if (MCURR)
	 	fprintf (pp,"===============================\n");
	else
	 	fprintf (pp,"===============\n");


	fprintf (pp, "|    CHEQUE     ");
	fprintf (pp, "|  CHEQUE  ");
	fprintf (pp, "|                   PAYEE                ");
	fprintf (pp, "|     NARRATIVE      ");
	fprintf (pp, "| BANK ");
	fprintf (pp, "| TRANSACTION ");
	if (ChequeList)
		fprintf (pp, "| BR ");
	if (MCURR)
	 	fprintf (pp,"|BASE  CURRENCY| LOCAL CURR   |\n");
	else
	 	fprintf (pp,"|   CHEQUE    |\n");

	fprintf (pp, "|    NUMBER     ");
	fprintf (pp, "|   DATE   ");
	fprintf (pp, "|                                        ");
	fprintf (pp, "|                    ");
	fprintf (pp, "| CODE ");
	fprintf (pp, "|     TYPE    ");
	if (ChequeList)
		fprintf (pp, "| NO ");
	if (MCURR)
	 	fprintf (pp,"|    AMOUNT    |    AMOUNT    |\n");
	else
	 	fprintf (pp,"|    AMOUNT    |\n");
                 
	fprintf (pp, "|---------------");
	fprintf (pp, "|----------");
	fprintf (pp, "|----------------------------------------");
	fprintf (pp, "|--------------------");
	fprintf (pp, "|------");
	fprintf (pp, "|-------------");
	if (ChequeList)
		fprintf (pp, "|----");
	if (MCURR)
	 	fprintf (pp,"|--------------|--------------|\n");
	else
	 	fprintf (pp,"|--------------|\n");

	fprintf (pp,".PI12\n");
}

/*===========================
| Validate and print lines. |
===========================*/
void
ProcessFile (
 void)
{
	suht_rec.hhsq_hash = suhp_rec.hhsq_hash;
	strcpy (suht_rec.est_no, "  ");

	cc = find_rec (suht, &suht_rec, GTEQ, "r");
	while (!cc && suht_rec.hhsq_hash == suhp_rec.hhsq_hash)
	{
		fprintf (pp, "|               ");
		fprintf (pp, "|          ");
		fprintf (pp, "|                                        ");
		fprintf (pp, "|                    ");
		fprintf (pp, "|      ");
		fprintf (pp, "|             ");
		fprintf (pp, "| %2.2s ", suht_rec.est_no);
		if (MCURR)
		{
	 		fprintf (pp,"|%13.2f |%13.2f |\n", 
								DOLLARS (suht_rec.est_amt_paid),
								DOLLARS (suht_rec.est_loc_amt));
		}
		else
	 		fprintf (pp,"|%13.2f |\n",DOLLARS (suht_rec.est_amt_paid));

		cc = find_rec (suht, &suht_rec, NEXT, "r");
	}
}

/*=========================================
| Print totals and end report to pformat. |
=========================================*/
void
EndReport (
 void)
{
	fprintf (pp, "|===============");
	fprintf (pp, "|==========");
	fprintf (pp, "|========================================");
	fprintf (pp, "|====================");
	fprintf (pp, "|======");
	fprintf (pp, "|=============");
	if (ChequeList)
		fprintf (pp, "|====");
	if (MCURR)
	 	fprintf (pp,"|==============|==============|\n");
	else
	 	fprintf (pp,"|==============|\n");

	fprintf (pp, "|               ");
	fprintf (pp, "|          ");
	fprintf (pp, "| *** G R A N D    T O T A L  ***        ");
	fprintf (pp, "|                    ");
	fprintf (pp, "|      ");
	fprintf (pp, "|             ");
	if (ChequeList)
		fprintf (pp, "|    ");
	if (MCURR)
	{
 		fprintf (pp,"|%13.2f |%13.2f |\n", 
						DOLLARS (grandTotal), DOLLARS (grandLocal));
	}
	else
 		fprintf (pp,"|%13.2f |\n",DOLLARS (grandTotal));
	
	fprintf (pp,".EOF\n");
}

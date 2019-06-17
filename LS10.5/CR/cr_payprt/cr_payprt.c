/*=====================================================================
|  Copyright (C) 1999 - 1999 Logistic Software Limited   .            |
|=====================================================================|
| $Id: cr_payprt.c,v 5.3 2001/12/06 10:04:25 kaarlo Exp $
|  Program Name  : (cr_payprt.c ) 
|  Program Desc  : (Suppliers payment voucher print.)
|---------------------------------------------------------------------|
|  Author        : Scott B Darrow  | Date Written  : 18/08/1996       |
|---------------------------------------------------------------------|
| $Log: cr_payprt.c,v $
| Revision 5.3  2001/12/06 10:04:25  kaarlo
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
#define NO_SCRGEN
char	*PNAME = "$RCSfile: cr_payprt.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CR/cr_payprt/cr_payprt.c,v 5.3 2001/12/06 10:04:25 kaarlo Exp $";

#include <pslscr.h>
#include <get_lpno.h>
#include <dsp_process2.h>
#include <dsp_screen.h>
#include <ml_std_mess.h>
#include <ml_cr_mess.h>

#include    "schema"

struct commRecord   comm_rec;
struct sumrRecord   sumr_rec;
struct suinRecord   suin_rec;

	double	totalVoucher =	0.00;
	int		printerNumber;
	long	supplierHash = 0L,
			voucherNo	 = 0L;

/*==========================
| Special fields and flags |
==========================*/

FILE	*fout;
     
int		pipeOpened = FALSE;

/*===========================
| Local function prototypes |
===========================*/
void	Process		 	 (int, long, long);
void	OpenDB		 	 (void);
void	CloseDB	 		 (void);
void	HeadingOutput	 (void);
void	PrintLine	 	 (void);
void	PrintFooter	 	 (void);

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int	argc,
 char *	argv [])
{
	if (argc != 4)
	{
		print_at (0, 0, ML (mlCrMess139), argv [0]);
		return (EXIT_FAILURE);
	}
	OpenDB ();

	printerNumber	=	atoi (argv [1]);
	supplierHash	=	atol (argv [2]);
	voucherNo		=	atol (argv [3]);
	
	Process (printerNumber, supplierHash, voucherNo);
	
	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

/*=======================================
| Main processing routine for vouchers. |
=======================================*/
void
Process (
 int	printerNumber,
 long	hhsu_hash,
 long	voucherNo)
{
	totalVoucher =	0.00;

	/*-----------------------
	| Supplier not on file. |
	-----------------------*/
	sumr_rec.hhsu_hash = hhsu_hash;
	cc = find_rec (sumr, &sumr_rec, COMPARISON, "r");
	if (cc)
		return;

	/*------------------------------------
	| Supplier not selected for payment. |
	------------------------------------*/
	if (sumr_rec.stat_flag [0] != 'S')
		return;
	
	suin_rec.hhsu_hash = sumr_rec.hhsu_hash;
	strcpy (suin_rec.inv_no,"               ");
	cc = find_rec (suin, &suin_rec, GTEQ, "u");
	while (!cc && suin_rec.hhsu_hash == sumr_rec.hhsu_hash) 
	{
		/*----------------------
		| Invoice not on file. |
		----------------------*/
		if (suin_rec.stat_flag [0] == 'S' && suin_rec.pay_voucher [0] != 'Y')
			PrintLine ();

		abc_unlock (suin);
		cc = find_rec (suin, &suin_rec, NEXT, "u");
	}
	abc_unlock (suin);

	if (pipeOpened == TRUE)
	{
		PrintFooter ();
		fprintf (fout, ".EOF\n");
		pclose (fout);
	}
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (void)
{
	abc_dbopen ("data");
	read_comm (comm_list, COMM_NO_FIELDS, (char *)&comm_rec);
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, "sumr_hhsu_hash");
	open_rec (suin, suin_list, SUIN_NO_FIELDS, "suin_id_no2");
}	

/*========================
| Close data base files. |	
========================*/
void
CloseDB (void)
{
	abc_unlock (sumr);
	abc_unlock (suin);
	abc_fclose (sumr);
	abc_dbclose ("data");
}

/*===================================
| Start Out Put To Standard Print . |
===================================*/
void
HeadingOutput (void)
{
	if ((fout = popen ("pformat","w")) == (FILE *) NULL)
		file_err (errno, "pformat", "POPEN");

	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (fout,".LP%d\n",printerNumber);
	fprintf (fout,".PI16\n");
	fprintf (fout,".10\n");
	fprintf (fout,".L130\n");
	fprintf (fout,".B1\n");
	fprintf (fout,".EPAYMENT VOUCHER \n\n");
	fprintf (fout,".CSUPPLIER : %s - %s (%s)\n", sumr_rec.crd_no, 
										   		 sumr_rec.crd_name,
												 sumr_rec.bank_code);
	fprintf (fout,".EVOUCHER NO : %010ld\n", 		voucherNo);

	fprintf (fout, ".R \n");

	fprintf (fout, "================");
	fprintf (fout, "=====================");
	fprintf (fout, "======");
	fprintf (fout, "================");
	fprintf (fout, "==================");
	fprintf (fout, "===================\n");

	fprintf (fout, "|   INVOICE     ");
	fprintf (fout, "|       NARRATIVE    ");
	fprintf (fout, "|CURR.");
	fprintf (fout, "| EXCHANGE RATE ");
	fprintf (fout, "|      FOREIGN    ");
	fprintf (fout, "|   BASE CURRENCY |\n");

	fprintf (fout, "|   NUMBER      ");
	fprintf (fout, "|                    ");
	fprintf (fout, "|CODE ");
	fprintf (fout, "|               ");
	fprintf (fout, "|     AMOUNT      ");
	fprintf (fout, "|       AMOUNT    |\n");

	fprintf (fout, "|---------------");
	fprintf (fout, "|--------------------");
	fprintf (fout, "|-----");
	fprintf (fout, "|---------------");
	fprintf (fout, "|-----------------");
	fprintf (fout, "|-----------------|\n");

	pipeOpened = TRUE;
}
	    	 
/*============================
| Print payment voucher line |
============================*/
void
PrintLine (void)
{
	char	printAmt [2][18];
	double	localAmt 	= 0.00;
	double	exchRate	= 0.00;

	if (pipeOpened == FALSE)
		HeadingOutput ();

	exchRate = suin_rec.exch_rate;

	if (exchRate == 0.00)
		exchRate = 1.00;

	localAmt = suin_rec.pay_amt / exchRate;
	
	sprintf (printAmt [0], "%17.17s", comma_fmt (DOLLARS (suin_rec.pay_amt), 
												"NN,NNN,NNN,NNN.NN"));
	sprintf (printAmt [1], "%17.17s", comma_fmt (DOLLARS (localAmt), 
												"NN,NNN,NNN,NNN.NN"));

	fprintf (fout, "|%-15.15s", suin_rec.inv_no);
	fprintf (fout, "|%-20.20s",	suin_rec.narrative);
	fprintf (fout, "| %3.3s ",	suin_rec.currency);
	fprintf (fout, "| %13.8f ", exchRate);
	fprintf (fout, "|%17.17s",  printAmt [0]);
	fprintf (fout, "|%17.17s|\n", printAmt [1]);

	totalVoucher += DOLLARS (localAmt);

	strcpy (suin_rec.pay_voucher, "Y");
	cc = abc_update (suin, &suin_rec);	
	if (cc)
		file_err (cc, suin, "DBUPDATE");
}

void
PrintFooter (void)
{
	char	totalAmt [18];

	sprintf (totalAmt, "%17.17s",comma_fmt (totalVoucher,"NN,NNN,NNN,NNN.NN"));

	fprintf (fout, "|---------------");
	fprintf (fout, "|--------------------");
	fprintf (fout, "|-----");
	fprintf (fout, "|---------------");
	fprintf (fout, "|-----------------");
	fprintf (fout, "|-----------------|\n");

	fprintf (fout, "|               ");
	fprintf (fout, "|                    ");
	fprintf (fout, "|     ");
	fprintf (fout, "|               ");
	fprintf (fout, "|     TOTAL       ");
	fprintf (fout, "|%17.17s|\n", totalAmt);

	fprintf (fout, "================");
	fprintf (fout, "=====================");
	fprintf (fout, "======");
	fprintf (fout, "================");
	fprintf (fout, "==================");
	fprintf (fout, "===================\n");

	fprintf (fout, ".LRP10\n");

	fprintf (fout, ".B3\n");

	fprintf (fout, "Cheque #  : ________________________\n");
	fprintf (fout, ".B1\n");
	fprintf (fout, "Bank Code : ________________________\n");
	fprintf (fout, ".B6\n");
	fprintf (fout, "______________________________   ______________________________   ______________________________\n");
	fprintf (fout, "         Prepared by                      Approved By                  Received By \n");
}

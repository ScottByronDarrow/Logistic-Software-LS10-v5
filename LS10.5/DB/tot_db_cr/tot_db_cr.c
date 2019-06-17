/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: tot_db_cr.c,v 5.3 2001/11/27 08:54:24 scott Exp $
|  Program Name  : (tot_db_cr.c)
|  Program Desc  : (Summary Customer & Supplier (Totals Only)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 20/01/87         |
|---------------------------------------------------------------------|
| $Log: tot_db_cr.c,v $
| Revision 5.3  2001/11/27 08:54:24  scott
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: tot_db_cr.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/tot_db_cr/tot_db_cr.c,v 5.3 2001/11/27 08:54:24 scott Exp $";

#include	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include	<ml_std_mess.h>
#include 	<ml_db_mess.h>

#define		CF(x)	comma_fmt (DOLLARS (x), "NNN,NNN,NNN.NN")

#include	"schema"

struct commRecord	comm_rec;
struct cumrRecord	cumr_rec;
struct sumrRecord	sumr_rec;


	Money	*cumr_balance	=	&cumr_rec.bo_current;
	Money	*sumr_balance	=	&sumr_rec.bo_curr;

	double	debitTotal [6];
	double	creditTotal [4];

	int		printerNo 	= 	1,
			per1_val	=	0,
			per2_val	=	0,
			per3_val	=	0;

	double	percent [6];

	FILE	*pp;

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	shutdown_prog 	(void);
void 	ReportHeading 	(int);
void 	EndReport 		(void);

int
main (
 int                argc,
 char*              argv [])
{
	int	i;

	if (argc != 2)
	{
		print_at (0,0,mlStdMess036,argv [0]);
        return (EXIT_FAILURE);
	}
	printerNo = atoi (argv [1]);

	OpenDB ();

	per1_val = comm_rec.pay_terms;
	per2_val = comm_rec.pay_terms * 2;
	per3_val = comm_rec.pay_terms * 3;

	dsp_screen ("Printing Customer /Supplier Summary Total Report.",
			comm_rec.co_no,comm_rec.co_name);

	ReportHeading (printerNo);

	strcpy (cumr_rec.co_no, comm_rec.co_no);
	strcpy (cumr_rec.est_no, "  ");
	strcpy (cumr_rec.dbt_no, "      ");

	cc = find_rec (cumr, &cumr_rec, GTEQ, "r");
	while (!cc && !strcmp (cumr_rec.co_no, comm_rec.co_no))
	{
		for (i = 0; i < 6; i++)
			debitTotal [i] += cumr_balance [i];
	
		cc = find_rec (cumr, &cumr_rec, NEXT, "r");
		dsp_process ("Customer : ",cumr_rec.dbt_no);
	}

	strcpy (sumr_rec.co_no, comm_rec.co_no);
	strcpy (sumr_rec.est_no, "  ");
	strcpy (sumr_rec.crd_no, "      ");

	cc = find_rec (sumr, &sumr_rec, GTEQ, "r");
	while (!cc && !strcmp (sumr_rec.co_no, comm_rec.co_no))
	{
		for (i = 0; i < 4; i++)
			creditTotal [i] += sumr_balance [i];
	
		cc = find_rec (sumr, &sumr_rec, NEXT, "r");
		dsp_process ("Supplier : ",sumr_rec.crd_no);
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
OpenDB (void)
{
	abc_dbopen ("data");

	/*-------------------------------
	| Read common terminal record . |
	-------------------------------*/
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_id_no");
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, "sumr_id_no");
}

/*=======================
| Close database Files. |
=======================*/
void
CloseDB (void)
{
	abc_fclose (cumr);
	abc_fclose (sumr);
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
	if ((pp = popen ("pformat","w")) == NULL)
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);

	/*---------------------------------
	| Start output to standard print. |
	---------------------------------*/
	fprintf (pp, ".START%s<%s>\n", DateToString (comm_rec.dbt_date), PNAME);
	fprintf (pp,".LP%d\n",prnt_no);
	fprintf (pp,".10\n");
	fprintf (pp,".L120\n");
	fprintf (pp,".E%s\n",comm_rec.co_name);
	fprintf (pp,".B1\n");
	fprintf (pp,".ECUSTOMER/SUPPLIER TOTAL REPORT\n");
	fprintf (pp,".B1\n");
	fprintf (pp,".EAS AT : %s\n", SystemTime ());

	fprintf (pp, ".R================================================");
	fprintf (pp, "================================================");
	fprintf (pp, "=================\n");

	fprintf (pp, "================================================");
	fprintf (pp, "================================================");
	fprintf (pp, "=================\n");

	fprintf (pp, "|    TOTAL      |    CURRENT    | OVERDUE PER 1 ");
	fprintf (pp, "| OVERDUE PER 2 | OVERDUE PER 3 | OVERDUE PER 4 ");
	fprintf (pp, "|   FORWARD.    |\n");

	fprintf (pp, "|---------------|---------------|---------------");
	fprintf (pp, "|---------------|---------------|---------------");
	fprintf (pp, "|---------------|\n");

}

/*=========================================
| Print totals and end report to pformat. |
=========================================*/
void
EndReport (void)
{
	int	i;

	double	debitGrandTotal 	= 0.00,
			creditGrandTotal 	= 0.00;

	debitGrandTotal = 	debitTotal [0] + debitTotal [1] + 
		    			debitTotal [2] + debitTotal [3] + 
		    			debitTotal [4] + debitTotal [5];

	for (i = 0; i < 6; i++)
	{
		if (debitGrandTotal != 0)
			percent [i] = debitTotal [i] / debitGrandTotal * 100;
		else
			percent [i] = 0.00;
	}

	fprintf (pp, "|---------------|---------------|---------------");
	fprintf (pp, "|---------------|---------------|---------------");
	fprintf (pp, "|---------------|\n");

	fprintf (pp, "|           ********* T O T A L S   F O R   C U S T O M E R S *********                         ");
	fprintf (pp, "                |\n");

	fprintf (pp, "|               |               |               ");
	fprintf (pp, "|               |               |               ");
	fprintf (pp, "|               |\n");

	fprintf (pp, "|%14.14s ",   CF (debitGrandTotal));
	fprintf (pp, "|%14.14s ",   CF (debitTotal [0]));
	fprintf (pp, "|%14.14s ",   CF (debitTotal [1]));
	fprintf (pp, "|%14.14s ",   CF (debitTotal [2]));
	fprintf (pp, "|%14.14s ",   CF (debitTotal [3]));
	fprintf (pp, "|%14.14s ",   CF (debitTotal [4]));
	fprintf (pp, "|%14.14s |\n",CF (debitTotal [5]));

	fprintf (pp, "|        100.00%%");
	fprintf (pp, "|        %6.2f%%",percent [0]);
	fprintf (pp, "|        %6.2f%%",percent [1]);
	fprintf (pp, "|        %6.2f%%",percent [2]);
	fprintf (pp, "|        %6.2f%%",percent [3]);
	fprintf (pp, "|        %6.2f%%",percent [4]);
	fprintf (pp, "|        %6.2f%%|\n",percent [5]);

	for (i = 0; i < 6; i++)
		percent [i] = 0.00;

	creditGrandTotal = 	creditTotal [0] + 
						creditTotal [1] + 	
						creditTotal [2] + 
						creditTotal [3];

	for (i = 0; i < 4; i++)
	{
		if (creditGrandTotal != 0)
			percent [i] = creditTotal [i] / creditGrandTotal * 100;
		else
			percent [i] = 0.00;
	}

	fprintf (pp, "|===============|===============|===============");
	fprintf (pp, "|===============|===============|===============");
	fprintf (pp, "|===============|\n");

	fprintf (pp, "|           ********* T O T A L S   F O R  S U P P L I E R S  *********                         ");
	fprintf (pp, "                |\n");

	fprintf (pp, "|               |               |               ");
	fprintf (pp, "|               |               |               ");
	fprintf (pp, "|               |\n");

	fprintf (pp, "|%14.14s ",   CF (creditGrandTotal));
	fprintf (pp, "|%14.14s ",   CF (creditTotal [0]));
	fprintf (pp, "|%14.14s ",   CF (creditTotal [1]));
	fprintf (pp, "|%14.14s ",   CF (creditTotal [2]));
	fprintf (pp, "|%14.14s ",   CF (creditTotal [3]));
	fprintf (pp, "|               ");
	fprintf (pp, "|               |\n");

	fprintf (pp, "|        100.00%%");
	fprintf (pp, "|        %6.2f%%",percent [0]);
	fprintf (pp, "|        %6.2f%%",percent [1]);
	fprintf (pp, "|        %6.2f%%",percent [2]);
	fprintf (pp, "|        %6.2f%%",percent [3]);
	fprintf (pp, "|               ");
	fprintf (pp, "|               |\n");
	fprintf (pp,".EOF\n");
}

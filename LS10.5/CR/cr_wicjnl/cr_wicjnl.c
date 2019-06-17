/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: cr_wicjnl.c,v 5.2 2001/08/09 08:52:23 scott Exp $
|  Program Name  : (cr_wicjnl.c)
|  Program Desc  : (Print Suppliers Invoice & Credits Journal)
|---------------------------------------------------------------------|
|  Author        : Terry Keillor.  | Date Written  : 29/03/89         |
|---------------------------------------------------------------------|
| $Log: cr_wicjnl.c,v $
| Revision 5.2  2001/08/09 08:52:23  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:01:48  scott
| RELEASE 5.0
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cr_wicjnl.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CR/cr_wicjnl/cr_wicjnl.c,v 5.2 2001/08/09 08:52:23 scott Exp $";

#define		MCURR	 (multiCurrency [0] == 'Y')
#define		GST		 (gstApplies [0] == 'Y')
#include	<pslscr.h>
#include	<GlUtils.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<twodec.h>
#include 	<ml_cr_mess.h>

	/*===========================
	| Special fields and flags. |
	===========================*/
   	int		pidNumber			= 0,	
			glwkWorkFileNumber	= 0,
			printerNumber		= 1,
    		headingOpenFlag 	= FALSE;

	long	pageNumber			= 0L,
    		oldPageNumber		= 0L;

	char 	invoiceCreditType 	[2],
   			pageStart 			[sizeof glwkRec.sys_ref],
	    	multiCurrency 		[2],
			gstApplies 			[2],
			gstCode 			[4];

   	double 	totalAmount 		[6],
   			creditTotal 	= 0.00,
   			debitTotal 		= 0.00,
   			exchangeRate 	= 0.00,
			addGst 			= 0.00,
			addTotal 		= 0.00;

   	FILE 	*ftmp;

#include	"schema"

struct commRecord	comm_rec;
struct extrRecord	extr_rec;


/*===========================
| Local function prototypes |
===========================*/
void	OpenDB		 	 (void);
void	CloseDB		 	 (void);
int		ReadGljc	 	 (void);
int		PrintDetails 	 (void);
void	PrintTotals	 	 (void);
int		ReportHeading	 (void);
void	shutdown_prog	 (void);
void	AddExtr		 	 (char *, char *, char *, char *, char *, char *, long, double, double);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char *	argv [])
{
	int		dataFound = FALSE;

	if (argc != 4)
	{
		/*---------------------------------------
		| Usage %s <lpno> <pid> <ic_type = I/C> |
		---------------------------------------*/
		print_at (0, 0, mlCrMess014, argv [0]);
		return (EXIT_FAILURE);
	}
	printerNumber 	= atoi (argv [1]);
	pidNumber   	= atoi (argv [2]);
	sprintf (invoiceCreditType, "%-1.1s",argv [3]);

	if (invoiceCreditType [0] != 'I' && invoiceCreditType [0] != 'C')
	{
		print_at (0,0, mlCrMess014, argv [0]);
		return (EXIT_FAILURE);
	}

	sprintf (multiCurrency, "%-1.1s", get_env ("CR_MCURR"));
	sprintf (gstApplies, "%-1.1s", get_env ("GST"));

	if (GST)
		sprintf (gstCode, "%-3.3s", get_env ("GST_TAX_NAME"));
	else
		sprintf (gstCode, "%-3.3s", "TAX");

	/*=============================================
	| Get common info from commom database file . |
	=============================================*/
	OpenDB ();

	if (invoiceCreditType [0] == 'I')
		dsp_screen ("Printing Suppliers Payables Journals.",
					comm_rec.co_no,comm_rec.co_name);
	else
		dsp_screen ("Printing Suppliers Credit Note Journals.",
					comm_rec.co_no,comm_rec.co_name);

	/*---------------------------------
	| Read journal control record.    |
	---------------------------------*/
	strcpy (gljcRec.journ_type, (invoiceCreditType [0] == 'I') ? " 7" : " 8");
		
	if (ReadGljc ())
		return (EXIT_FAILURE);
	
	/*---------------------------------
	| Print  journal transactions.    |
	---------------------------------*/
	cc = RF_READ (glwkWorkFileNumber, (char *) &glwkRec);
	while (!cc)
	{
		dataFound = TRUE;
		if (PrintDetails ())
			return (EXIT_FAILURE);

		cc = RF_READ (glwkWorkFileNumber, (char *) &glwkRec);
	}
	/*---------------------------------
	| Print  journal control records. |
	---------------------------------*/
	if (dataFound == TRUE)
		PrintTotals ();

	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*======================
| Open Database Files. |
======================*/
void
OpenDB (
 void)
{
	char	filename [100];
	char *	sptr = getenv ("PROG_PATH");

	abc_dbopen ("data");

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	sprintf (filename,"%s/WORK/gl_work%05d",
				 (sptr == (char *) 0) ? "/usr/LS10.5" : sptr, pidNumber);

	cc = RF_OPEN (filename, sizeof (glwkRec), "r", &glwkWorkFileNumber);
	if (cc) 
		sys_err ("Error in glwk During (WKOPEN)", cc, PNAME);

	OpenGljc ();
	open_rec (extr, extr_list, EXTR_NO_FIELDS, "extr_id_no");
}

/*=======================
| Close Database Files. |
=======================*/
void
CloseDB (
 void)
{
	cc = RF_CLOSE (glwkWorkFileNumber);
	if (cc) 
		sys_err ("Error in glwk During (WKCLOSE)", cc, PNAME);

	abc_fclose (extr);
	GL_Close ();
	abc_dbclose ("data");
}

/*============================================
| Get journal control records for jnl type . |
============================================*/
int
ReadGljc (
 void)
{
	strcpy (gljcRec.co_no, comm_rec.co_no);
	cc = find_rec (gljc, &gljcRec, COMPARISON, "r");
	if (cc)
	{
		/*---------------------------------------
		| UNABLE TO FIND JOURNAL CONTROL RECORD |
		---------------------------------------*/
		errmess (ML (mlCrMess013));
		sleep (sleepTime);
		shutdown_prog ();
		return (EXIT_FAILURE);
	}

	return (EXIT_SUCCESS);
}

/*============================================
| Print journal transaction record.          |
============================================*/
int
PrintDetails (
 void)
{
	int	type;
	double	gst_bse = 0.00;

	if (strcmp (glwkRec.co_no,comm_rec.co_no))
		return (EXIT_SUCCESS);

	if (headingOpenFlag != 1)
	{
		if (ReportHeading () == -1)
			return (EXIT_FAILURE);
	}

	strcpy (pageStart, glwkRec.sys_ref);
	pageNumber = atol (pageStart);
	if (pageNumber != oldPageNumber)
	{
		fprintf (ftmp,".PG%ld\n",pageNumber);
		oldPageNumber = pageNumber;
	}

	/*--------------------------------------------------
 	| Print Invoice / Credit Heading Details.          |
	--------------------------------------------------*/
	if (strcmp (glwkRec.chq_inv_no,"               "))
	{
		if (invoiceCreditType [0] == 'I')
			dsp_process ("Invoice No. : ", glwkRec.chq_inv_no);
		else
			dsp_process ("Credit Note No. : ",
					glwkRec.chq_inv_no);

		fprintf (ftmp,"|%-9.9s",glwkRec.acronym);
		fprintf (ftmp,"|%-15.15s",glwkRec.chq_inv_no);

		if (MCURR)
		{
			exchangeRate = glwkRec.exch_rate;
			fprintf (ftmp,"|%15.2f",DOLLARS (glwkRec.ci_amt));
			if (GST)
			{
				gst_bse = glwkRec.o4_amt;
				fprintf (ftmp,"|%12.2f", DOLLARS (gst_bse));
			}
			else
				fprintf (ftmp,"|            ");

			fprintf (ftmp,"|%9.4f", exchangeRate);
			fprintf (ftmp,"|%3.3s ", glwkRec.currency);

			if (exchangeRate != 0.00)
			{
				fprintf (ftmp,"|%12.2f", DOLLARS (glwkRec.ci_amt / exchangeRate));
			}
			else
				fprintf (ftmp,"|%12.2f", DOLLARS (glwkRec.ci_amt / 1.00));
		}
		else
		{
			fprintf (ftmp,"|%15.2f",DOLLARS (glwkRec.ci_amt));
			if (GST)
			{
				gst_bse = glwkRec.o4_amt;
				fprintf (ftmp,"|%12.2f", DOLLARS (gst_bse));
			}
			else
				fprintf (ftmp,"|            ");
			fprintf (ftmp,"|              |            ");
		}

		AddExtr (comm_rec.co_no,
				  glwkRec.run_no,
				  glwkRec.acronym,
				  glwkRec.chq_inv_no,
				  glwkRec.tran_type,
				  glwkRec.period_no,
				  glwkRec.tran_date,
				  glwkRec.ci_amt,
				  glwkRec.o4_amt);

		addTotal += glwkRec.ci_amt;
		addGst += gst_bse;
	}
	else
	{
		if (MCURR)
		{
			fprintf (ftmp,"|         |               |               ");
			fprintf (ftmp,"|            |         |    |            ");
		}
		else
		{
			fprintf (ftmp,"|         |               |               ");
			fprintf (ftmp,"|            |              |            ");
		}
	}

	/*-------------------------------
 	| Print GL Allocation Details.  |
	-------------------------------*/
	if (strncmp (glwkRec.acc_no,"                ", MAXLEVEL))
	{
		type = atoi (glwkRec.jnl_type);
		if ( (type % 2) == 0)
		{
			fprintf (ftmp,"|                |              ");
			fprintf (ftmp,"|%-16.16s",glwkRec.acc_no);
			fprintf (ftmp,"|%13.2f ",DOLLARS (glwkRec.amount));
			creditTotal += glwkRec.amount;
		}
		else
		{
			fprintf (ftmp,"|%-16.16s",glwkRec.acc_no);
			fprintf (ftmp,"|%13.2f ",DOLLARS (glwkRec.amount));
			fprintf (ftmp,"|                |              ");
			debitTotal += glwkRec.amount;
		}
		fprintf (ftmp,"| %2.2d",mth2fisc (atoi (glwkRec.period_no),
						 comm_rec.fiscal));
		fprintf (ftmp,"|\n");
	}
	else
	{
		fprintf (ftmp,"|                |              ");
		fprintf (ftmp,"|                |              |   |\n");
	}

	return (EXIT_SUCCESS);
}

/*============================================
| Print journal totals.                      |
============================================*/
void
PrintTotals (
 void)
{
	fprintf (ftmp,"|---------|---------------");
	fprintf (ftmp,"|---------------|------------|--------------|------------|");
	fprintf (ftmp,"----------------|--------------|");
	fprintf (ftmp,"----------------|--------------|---|\n");

	fprintf (ftmp,"|         |               ");
	if (MCURR)
	{
		fprintf (ftmp,"|               |");
		fprintf (ftmp,"%12.2f", DOLLARS (addGst));
		fprintf (ftmp,"|              |%12.2f|",DOLLARS (addTotal));
	}
	else
	{
		fprintf (ftmp,"|%15.2f|", DOLLARS (addTotal));
		fprintf (ftmp,"%12.2f", DOLLARS (addGst));
		fprintf (ftmp,"|              |            |");
	}
	fprintf (ftmp,"                |              |");
	fprintf (ftmp,"                |              |   |\n");

	fprintf (ftmp,"|=========|===============");
	fprintf (ftmp,"|===============|============|==============|============|");
	fprintf (ftmp,"================|==============|");
	fprintf (ftmp,"================|==============|===|\n");

	fprintf (ftmp,"|          T O D A Y S   T O T A L S      ");
	fprintf (ftmp,"                                         |");
	fprintf (ftmp,"                |%13.2f |",DOLLARS (debitTotal));
	fprintf (ftmp,"                |%13.2f |   |\n",DOLLARS (creditTotal));

	totalAmount [0]	=	gljcRec.tot_1 + 
				 		gljcRec.tot_3 + 
				 		gljcRec.tot_5 - 
						debitTotal;

	totalAmount [1] =	gljcRec.tot_4 + 
				 		gljcRec.tot_2 + 
				 		gljcRec.tot_6 - 
						creditTotal;

	fprintf (ftmp,"|          P R I O R   B A L A N C E S    ");
	fprintf (ftmp,"                                         |");
	fprintf (ftmp,"                |%13.2f |",DOLLARS (totalAmount [0]));
	fprintf (ftmp,"                |%13.2f |   |\n",DOLLARS (totalAmount [1]));

	totalAmount [0]	=	gljcRec.tot_1 + 
				 		gljcRec.tot_3 + 
				 		gljcRec.tot_5;

	totalAmount [1] =	gljcRec.tot_4 + 
				 		gljcRec.tot_2 + 
				 		gljcRec.tot_6;

	fprintf (ftmp,"|          M O N T H   T O   D A T E      ");
	fprintf (ftmp,"                                         |");
	fprintf (ftmp,"                |%13.2f |",DOLLARS (totalAmount [0]));
	fprintf (ftmp,"                |%13.2f |   |\n",DOLLARS (totalAmount [1]));

	fprintf (ftmp,".EOF\n");

	pclose (ftmp);
}

/*========================= 
| Print journal headings. |
=========================*/
int
ReportHeading (
 void)
{
	/*---------------------------------
	| Open pipe work file to pformat. |
	---------------------------------*/
	strcpy (pageStart, glwkRec.sys_ref);
	pageNumber = atol (pageStart);
	oldPageNumber = pageNumber ;
	headingOpenFlag = 1;

	if ( (ftmp = popen ("pformat","w")) == NULL)
	{
		sys_err ("Error in pformat During (POPEN)", 1, PNAME);
		shutdown_prog ();
		return (-1);
	}

	/*-----------------------
	| Start output to file. |
	-----------------------*/

	fprintf (ftmp,".START%s<%s>\n", DateToString (comm_rec.crd_date), PNAME);
	fprintf (ftmp,".LP%d\n", printerNumber);
	fprintf (ftmp,".SO\n");
	fprintf (ftmp,".12\n");
	fprintf (ftmp,".L158\n");
	fprintf (ftmp,".PG%ld\n",pageNumber);
	fprintf (ftmp,".B1\n");
	fprintf (ftmp,".ESUPPLIERS %s JOURNAL (BATCH NO. : %s)\n",
				clip (gljcRec.jnl_desc), glwkRec.run_no);
	fprintf (ftmp,".B1\n");
	fprintf (ftmp,".E%s AS AT: %s",clip (comm_rec.co_short), SystemTime ());
	fprintf (ftmp,".B1\n");

	fprintf (ftmp,".R=====================");
	fprintf (ftmp,"==========================================================");
	fprintf (ftmp,"=========================");
	fprintf (ftmp,"================================================\n");

	fprintf (ftmp,"=======================");
	fprintf (ftmp,"========================================================");
	fprintf (ftmp,"=========================");
	fprintf (ftmp,"================================================\n");


	fprintf (ftmp,"|SUPPLIER |");

	if (invoiceCreditType [0] == 'I')
		fprintf (ftmp,"          I N V O I C E                     ");
	else
		fprintf (ftmp,"          C R E D I T   N O T E             ");

	if (MCURR)
		fprintf (ftmp,"| EXCHANGE|CURR|   AMOUNT   |");
	else
		fprintf (ftmp,"|              |            |");

	fprintf (ftmp,"     D E B I T S               |");
	fprintf (ftmp,"     C R E D I T S             |G/L|\n");

	fprintf (ftmp,"|NO./ACRON|   NUMBER      |    AMOUNT     ");
	if (GST)
		fprintf (ftmp,"|     %-3.3s    ", gstCode);
	else
		fprintf (ftmp,"|            ");

	if (MCURR)
		fprintf (ftmp,"|   RATE  |CODE| LOCAL CURR |");
	else
		fprintf (ftmp,"|              |            |");

	fprintf (ftmp,"    ACCOUNT     |    AMOUNT    |");
	fprintf (ftmp,"    ACCOUNT     |    AMOUNT    |PER|\n");

	fprintf (ftmp,"|---------|---------------|---------------");
	if (MCURR)
		fprintf (ftmp,"|------------|---------|----|------------|");
	else
		fprintf (ftmp,"|------------|--------------|------------|");
	fprintf (ftmp,"----------------|--------------|");
	fprintf (ftmp,"----------------|--------------|---|\n");

	fprintf (ftmp,".PI12\n");

	return (EXIT_SUCCESS);
}

void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*=====================
| Add tax file audit. |
=====================*/
void
AddExtr (
	 char	*co_no,
	 char 	*run_no,
	 char 	*int_no,
	 char 	*ref_no,
	 char 	*jnl_type,
	 char 	*period,
	 long	date,
	 double	gross,
	 double	gst)
{
	gross = no_dec (gross);
	gst   = no_dec (gst);

	if (gross == 0.00 && gst == 0.00)
		return;

	strcpy (extr_rec.co_no, co_no);
	extr_rec.run_no = atol (run_no);
	sprintf (extr_rec.int_no, "%-8.8s", int_no);
	sprintf (extr_rec.ref_no, "%-15.15s", ref_no);
	strcpy (extr_rec.jnl_type, jnl_type);
	sprintf (extr_rec.gl_per, "%2.2d", mth2fisc (atoi (period),
						       comm_rec.fiscal));
	strcpy (extr_rec.stat_flag,"0");

	cc = find_rec (extr, &extr_rec, COMPARISON, "r");
	if (cc)
	{
		extr_rec.date = date;
		extr_rec.sal_val = DOLLARS (gross);
		extr_rec.gst_val = DOLLARS (gst);
		cc = abc_add (extr, &extr_rec);
		if (cc)
			file_err (cc, extr, "DBADD");
	}
	else
	{
		extr_rec.date = date;
		extr_rec.sal_val = DOLLARS (gross);
		extr_rec.gst_val = DOLLARS (gst);
	
		cc = abc_update (extr, &extr_rec);
		if (cc)
			file_err (cc, extr, "DBUPDATE");
	}
}

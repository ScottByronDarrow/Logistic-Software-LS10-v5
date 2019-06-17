/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: cr_payjnl.c,v 5.3 2002/07/17 09:57:03 scott Exp $
|  Program Name  : (cr_payjnl.c)
|  Program Desc  : (Print Suppliers Payments Journals)
|---------------------------------------------------------------------|
|  Author        : Terry Keillor.  | Date Written  : 04/05/89         |
|---------------------------------------------------------------------|
| $Log: cr_payjnl.c,v $
| Revision 5.3  2002/07/17 09:57:03  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.2  2001/08/09 08:52:10  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:01:39  scott
| RELEASE 5.0
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cr_payjnl.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CR/cr_payjnl/cr_payjnl.c,v 5.3 2002/07/17 09:57:03 scott Exp $";

#define		MCURR	 (multiCurrency [0] == 'Y')
#define		GST		 (gstApplied [0] == 'Y')
#include <pslscr.h>
#include <GlUtils.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <ml_std_mess.h>
#include <ml_cr_mess.h>

#define	X_OFF	22
#define	Y_OFF	5
#include <get_lpno.h>

#include	"schema"

	struct	commRecord	comm_rec;
	/*
	 * Special fields and flags.
	 */
   	int		processID			= 0,
			glwkNumber			= 0,
			printerNumber 		= 1,
    		headingPrintedFlag	= FALSE;

	long 	pageNumberCounter	= 0,
    		oldPageNumberCounter= 0;

   	char 	branchNumber [3],
   			previousCheque [16],
   			previousAcronym [10],
   			controlTypeDescription [13],
   			pageStart [11],
	    	multiCurrency [2],
			gstApplied [2];

   	double 	totalAmount [6],
   			creditTotal = 0,
   			debitTotal	= 0;

   	FILE 	*ftmp;

/*===========================
| Local function prototypes |
===========================*/
int		OpenDB			 (void);
void	CloseDB		 	 (void);
int		ReadGljc		 (void);
int		PrintDetails	 (void);
void	PrintTotal		 (void);
int		PrintHeader	 	 (void);
int		LoadPaper		 (void);
void	shutdown_prog	 (void);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char *	argv [])
{
	int	no_data = 0;

	if (argc < 3)
	{
		print_at (0, 0, mlCrMess138, argv [0]);
		return (EXIT_FAILURE);
	}
	printerNumber 	= atoi (argv [1]);
	processID   	= atoi (argv [2]);

	sprintf (multiCurrency, "%-1.1s", get_env ("CR_MCURR"));
	sprintf (gstApplied, "%-1.1s", get_env ("GST"));

	set_tty ();
	init_scr ();

	if (OpenDB ())
	{
		shutdown_prog ();
		return (EXIT_FAILURE);
	}
	
	if (!printerNumber)
		LoadPaper ();

	dsp_screen ("Printing Suppliers Disbursements Journals.",
					comm_rec.co_no,comm_rec.co_name);

	/*---------------------------------
	| Read journal control record.    |
	---------------------------------*/
	strcpy (gljcRec.journ_type," 9");
	if (ReadGljc ())
	{
		shutdown_prog ();
		return (EXIT_FAILURE);
	}
	
	/*---------------------------------
	| Print  journal transactions.    |
	---------------------------------*/
	strcpy (previousCheque, "               ");
	strcpy (previousAcronym, "         ");
	cc = RF_READ (glwkNumber, (char *) &glwkRec);
	while (!cc)
	{
		no_data = 1;
		if (!strcmp (gljcRec.journ_type,glwkRec.tran_type))
		{
			if (PrintDetails ())
			{
				shutdown_prog ();
				return (EXIT_FAILURE);
			}
		}
		else
		{
			errmess (ML (mlCrMess169));
			sleep (10);
			shutdown_prog ();
			return (EXIT_FAILURE);
		}
		cc = RF_READ (glwkNumber, (char *) &glwkRec);
	}
	/*---------------------------------
	| Print  journal control records. |
	---------------------------------*/
	if (no_data == 1)
		PrintTotal ();

	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*======================
| Open Database Files. |
======================*/
int
OpenDB (
 void)
{
	char	filename [100];
	char *	sptr = getenv ("PROG_PATH");

	sprintf (filename,"%s/WORK/gl_work%05d",
				 (sptr == (char *) 0) ? "/usr/LS10.5" : sptr, processID);

	abc_dbopen ("data");
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	strcpy (branchNumber, comm_rec.est_no);

	OpenGljc ();

	cc = RF_OPEN (filename,sizeof (glwkRec),"r",&glwkNumber);
	return (cc);
}

/*=======================
| Close Database Files. |
=======================*/
void
CloseDB (void)
{
	GL_Close ();
	abc_dbclose ("data");

	cc = RF_CLOSE (glwkNumber);
	if (cc && cc != 2) 
		sys_err ("Error in glwk During (WKCLOSE)", cc, PNAME);

}

/*============================================
| Get journal control records for jnl type . |
============================================*/
int
ReadGljc (
 void)
{
	strcpy (gljcRec.co_no,comm_rec.co_no);
	cc = find_rec (gljc, &gljcRec, COMPARISON, "r");
	if (cc)
	{
		errmess (ML (mlCrMess013));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}

/*============================================
| Print journal transaction record.          |
============================================*/
int
PrintDetails (void)
{
	int	type;

	if (strcmp (glwkRec.co_no, comm_rec.co_no))
		return (EXIT_SUCCESS);

	if (headingPrintedFlag != 1)
	{
		if (PrintHeader ())
			return (EXIT_FAILURE);
	}

	strcpy (pageStart,glwkRec.sys_ref);
	pageNumberCounter = atol (pageStart);
	if (pageNumberCounter != oldPageNumberCounter)
	{
		fprintf (ftmp,".PG%ld\n",pageNumberCounter);
		oldPageNumberCounter = pageNumberCounter;
	}

	/*
 	 * Print Cheque Heading & Exch Variance Details    
	 */
	if (strncmp (glwkRec.acc_no,"                ",MAXLEVEL) &&
	     strcmp (glwkRec.chq_inv_no,"               ")) 
	{
		/*------------------------------
 		| Print Cheque Header.         |
		------------------------------*/
		if (strcmp (previousAcronym, glwkRec.acronym) ||
	    	     strcmp (previousCheque, glwkRec.chq_inv_no)) 
		{
			dsp_process ("Cheque No. : ", glwkRec.chq_inv_no);
			fprintf (ftmp,"|%-9.9s",glwkRec.acronym);
			fprintf (ftmp,"|%-15.15s|               ", glwkRec.chq_inv_no);

			if (MCURR)
			{
				fprintf (ftmp,"|%15.2f", DOLLARS (glwkRec.o1_amt));
				fprintf (ftmp,"|%-4.4s",  glwkRec.currency);
				fprintf (ftmp,"|%9.4f",  glwkRec.exch_rate);
				fprintf (ftmp,"|%12.2f", DOLLARS (glwkRec.loc_amount));
			}
			else
			{
				fprintf (ftmp,"|%15.2f", DOLLARS (glwkRec.loc_amount));
				fprintf (ftmp,"|    |         |            ");
			}
			strcpy (previousCheque, glwkRec.chq_inv_no);
			strcpy (previousAcronym, glwkRec.acronym);

		}
		/*------------------------------
 		| Print Exchange Variance.     |
		------------------------------*/
		else
		{
			fprintf (ftmp,"|         |               ");
			fprintf (ftmp,"|%-15.15s|                    ", glwkRec.name);
			fprintf (ftmp,"|         |%12.2f", DOLLARS (glwkRec.loc_amount));
		}
	}

	/*--------------------------------------------------
 	| Print Cheque Inv/Cred Note Payment Details.      |
	--------------------------------------------------*/
	else if (!strncmp (glwkRec.acc_no,"                ",MAXLEVEL) &&
	           strcmp (glwkRec.chq_inv_no,"               "))
	{
		fprintf (ftmp,"|         |               ");
		fprintf (ftmp,"|%-15.15s",glwkRec.name);

		if (MCURR)
		{
			fprintf (ftmp,"|%15.2f",DOLLARS (glwkRec.o1_amt));
			fprintf (ftmp,"|%-4.4s",  glwkRec.currency);
			fprintf (ftmp,"|%9.4f", glwkRec.exch_rate);
			fprintf (ftmp,"|%12.2f", DOLLARS (glwkRec.ci_amt));
		}
		else
		{
			fprintf (ftmp,"|%15.2f", DOLLARS (glwkRec.ci_amt));
			fprintf (ftmp,"|         |                ");
		}
	}

	/*--------------------------------------------------
 	| Print Discount / Bank Control Totals.     .      |
	--------------------------------------------------*/
	else if (strncmp (glwkRec.acc_no,"                ",MAXLEVEL) &&
	          !strcmp (glwkRec.chq_inv_no,"               "))
	{
		fprintf (ftmp,"|         |               ");
		fprintf (ftmp,"|               |               |    |         ");

		strcpy (controlTypeDescription ,"            ");

		if (!strcmp (glwkRec.user_ref,"dtctrl"))
			strcpy (controlTypeDescription ," DISCOUNT   ");

		if (!strcmp (glwkRec.user_ref,"bkctrl"))
			strcpy (controlTypeDescription ,"BANK CONTROL");

		if (!strcmp (glwkRec.user_ref,"evctrl"))
			strcpy (controlTypeDescription ," EXCH VAR'N ");

		fprintf (ftmp,"|%s", controlTypeDescription);
	}
	else
	{
		fprintf (ftmp,"|         |               ");
		fprintf (ftmp,"|               |               |    |         |            ");
	}

	/*-------------------------------
 	| Print Debit / Credit Details. |
	-------------------------------*/
	if (strncmp (glwkRec.acc_no,"                ",MAXLEVEL))
	{
		type = atoi (glwkRec.jnl_type);
		if ( (type % 2) == 0)
		{
			fprintf (ftmp,"|                |              ");
			fprintf (ftmp,"|%-16.16s",glwkRec.acc_no);
			fprintf (ftmp,"| %12.2f ",DOLLARS (glwkRec.loc_amount));
			creditTotal += glwkRec.loc_amount;
		}
		else
		{
			fprintf (ftmp,"|%-16.16s",glwkRec.acc_no);
			fprintf (ftmp,"| %12.2f ",DOLLARS (glwkRec.loc_amount));
			fprintf (ftmp,"|                |              ");
			debitTotal += glwkRec.loc_amount;
		}
		fprintf (ftmp,"|\n");
	}
	else
	{
		fprintf (ftmp,"|                |              ");
		fprintf (ftmp,"|                |              |\n");
	}
	return (EXIT_SUCCESS);
}

/*============================================
| Print journal totals.                      |
============================================*/
void
PrintTotal (
 void)
{
	fprintf (ftmp,"|=========|===============");
	fprintf (ftmp,"|===============|===============|====|=========|============|");
	fprintf (ftmp,"================|==============|");
	fprintf (ftmp,"================|==============|\n");

	fprintf (ftmp,"| T O D A Y S   T O T A L S");
	fprintf (ftmp,"                                                           |");
	fprintf (ftmp,"                | %12.2f |",DOLLARS (debitTotal));
	fprintf (ftmp,"                | %12.2f |\n",DOLLARS (creditTotal));

	totalAmount [0] = gljcRec.tot_1 + 
				 	 gljcRec.tot_3 + 
				 	 gljcRec.tot_5 - debitTotal;

	totalAmount [1] = gljcRec.tot_4 + 
				 	 gljcRec.tot_2 + 
				 	 gljcRec.tot_6 - creditTotal;

	fprintf (ftmp,"|P R I O R  B A L A N C E S");
	fprintf (ftmp,"                                                           |");
	fprintf (ftmp,"                | %12.2f |",DOLLARS (totalAmount [0]));
	fprintf (ftmp,"                | %12.2f |\n",DOLLARS (totalAmount [1]));

	totalAmount [0] = gljcRec.tot_1 + 
				 	 gljcRec.tot_3 + 
				 	 gljcRec.tot_5;

	totalAmount [1] = gljcRec.tot_4 + 
				 	 gljcRec.tot_2 + 
				 	 gljcRec.tot_6;

	fprintf (ftmp,"|M O N T H   T O   D A T E ");
	fprintf (ftmp,"                                                           |");
	fprintf (ftmp,"                | %12.2f |",DOLLARS (totalAmount [0]));
	fprintf (ftmp,"                | %12.2f |\n",DOLLARS (totalAmount [1]));

	fprintf (ftmp,".EOF\n");

	pclose (ftmp);
}

/*========================= 
| Print journal headings. |
=========================*/
int
PrintHeader (
 void)
{
	/*---------------------------------
	| Open pipe work file to pformat. |
	---------------------------------*/
	strcpy (pageStart,glwkRec.sys_ref);
	pageNumberCounter 	 = atol (pageStart);
	oldPageNumberCounter = pageNumberCounter ;
	headingPrintedFlag   = TRUE;

	if ( (ftmp = popen ("pformat","w")) == NULL)
	{
		sys_err ("Error in pformat During (POPEN)", 1, PNAME);
		return (EXIT_FAILURE);
	}

	/*-----------------------
	| Start output to file. |
	-----------------------*/

	fprintf (ftmp,".START%s<%s>\n", DateToString (comm_rec.crd_date), PNAME);
	fprintf (ftmp,".LP%d\n", printerNumber);
	fprintf (ftmp,".SO\n");
	fprintf (ftmp,".13\n");
	fprintf (ftmp,".L158\n");
	fprintf (ftmp,".PG%ld\n",pageNumberCounter);
	fprintf (ftmp,".B1\n");
	fprintf (ftmp,".ESUPPLIERS %s JOURNAL (BATCH NO. : %s)\n",
				clip (gljcRec.jnl_desc), glwkRec.run_no);
	fprintf (ftmp,".B1\n");
	fprintf (ftmp,".E%s AS AT: %s\n",clip (comm_rec.co_short), SystemTime ());
	fprintf (ftmp,".B1\n");

	fprintf (ftmp,".R==========================");
	fprintf (ftmp,"========================================================");
	fprintf (ftmp,"=====================================");
	fprintf (ftmp,"================================\n");

	fprintf (ftmp,"==========================");
	fprintf (ftmp,"========================================================");
	fprintf (ftmp,"=====================================");
	fprintf (ftmp,"================================\n");


	fprintf (ftmp,"|SUPPLIER |");
	fprintf (ftmp,"             P A Y M E N T                     ");

	if (MCURR)
		fprintf (ftmp,"|CURR| EXCHANGE|   AMOUNT   |");
	else
		fprintf (ftmp,"|    |         |            |");

	fprintf (ftmp,"          D E B I T S          |");
	fprintf (ftmp,"       C R E D I T S           |\n");

	fprintf (ftmp,"|NO./ACRON|");
	fprintf (ftmp," CHEQUE NUMBER |  DOCUMENT NO  |     AMOUNT    ");

	if (MCURR)
		fprintf (ftmp,"|CODE|   RATE  | LOCAL CURR |");
	else
		fprintf (ftmp,"|    |         |            |");

	fprintf (ftmp,"     ACCOUNT    |    AMOUNT    |");
	fprintf (ftmp,"     ACCOUNT    |    AMOUNT    |\n");

	fprintf (ftmp,"|---------|---------------");
	fprintf (ftmp,"|---------------|---------------|----|---------|------------|");
	fprintf (ftmp,"----------------|--------------|");
	fprintf (ftmp,"----------------|--------------|\n");

	fprintf (ftmp,".PI12\n");
	return (EXIT_SUCCESS);
}

/*======================================
| Routine to check if paper loaded.    |
======================================*/
int
LoadPaper (
 void)
{
	clear ();

	box (12, 0, 56, 3);
	print_at (1, 14,ML (mlCrMess172));
	print_at (3, 14,ML (mlCrMess173));
	fflush (stdout);
	printerNumber = get_lpno (0);
	return (EXIT_SUCCESS);
}

void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

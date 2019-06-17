/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: cr_wevjnl.c,v 5.2 2001/08/09 08:52:23 scott Exp $
|  Program Name  : (cr_wevjnl.c)
|  Program Desc  : (Print Suppliers Exchange Variance Journals.)
|---------------------------------------------------------------------|
|  Author        : Terry Keillor.  | Date Written  : 29/03/89         |
|---------------------------------------------------------------------|
| $Log: cr_wevjnl.c,v $
| Revision 5.2  2001/08/09 08:52:23  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:01:48  scott
| RELEASE 5.0
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cr_wevjnl.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CR/cr_wevjnl/cr_wevjnl.c,v 5.2 2001/08/09 08:52:23 scott Exp $";

#include	<pslscr.h>
#include	<GlUtils.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include	<ml_std_mess.h>
#include	<ml_cr_mess.h>

#define	MCURR		 (multiCurrency [0] == 'Y' || multiCurrency [0] == '1')

	/*================================================================
	| Special fields and flags  ################################## . |
	================================================================*/
   	int		pidNumber			= 0,
			glwkWorkFileNumber 	= 0,
			printerNumber 		= 1,
    		headingFlag 		= 0;

	long 	pageNumber			= 0L,
    		oldPageNumber		= 0L;

	int		accountsReceivable = TRUE;

   	char 	branchNumber 	[3],
			pageStart 		[11],
	    	multiCurrency 	[2],
			baseCurr		[4];

   	double 	totalAmount [6];

	double 	creditTotal 	= 0.00,
			debitTotal 		= 0.00;

   	FILE 	*ftmp;

#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;

/*===========================
| Local function prototypes |
===========================*/
void	OpenDB			 	 (void);
void	CloseDB		 	 	 (void);
int		ReadGljc		 	 (void);
int		PrintJournalDetails	 (void);
void	PrintTotal		 	 (void);
int		Heading			 	 (void);
void	shutdown_prog	 	 (void);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char *	argv [])
{
	char *	sptr;

	int	noDataFound = 0;

	if (argc != 3)
	{
		print_at (0, 0, mlCrMess138, argv [0]);
		return (EXIT_FAILURE);
	}

	printerNumber 	= atoi (argv [1]);
	pidNumber   	= atoi (argv [2]);

	sptr = strrchr (argv [0], '/');
	if (sptr == (char *) 0)
		sptr = argv [0];
	else
		sptr++;

	if (!strcmp (sptr, "cr_wevjnl"))
		accountsReceivable = FALSE;
	else
		accountsReceivable = TRUE;

	if (accountsReceivable)
		sprintf (multiCurrency, "%-1.1s", get_env ("DB_MCURR"));
	else
		sprintf (multiCurrency, "%-1.1s", get_env ("CR_MCURR"));

	OpenDB ();

	strcpy (branchNumber, comm_rec.est_no);

	sprintf (err_str, "Printing %s Exchange Variance Journals.",
				 (accountsReceivable) ? "Customer" : "Suppliers");

	dsp_screen (err_str, comm_rec.co_no, comm_rec.co_name);

	/*---------------------------------
	| Read journal control record.    |
	---------------------------------*/
	strcpy (gljcRec.journ_type,"15");
    if (ReadGljc ())
		return (EXIT_FAILURE);
	
	/*---------------------------------
	| Print  journal transactions.    |
	---------------------------------*/
	cc = RF_READ (glwkWorkFileNumber, (char *) &glwkRec);
	while (!cc)
	{
		noDataFound = 1;
		if (!strcmp (gljcRec.journ_type,glwkRec.tran_type))
		{
			if (PrintJournalDetails () == -1)
				return (EXIT_FAILURE);
		}
		else
		{
			errmess (ML (mlCrMess169));
			sleep (10);
			shutdown_prog ();
			return (EXIT_FAILURE);
		}
		cc = RF_READ (glwkWorkFileNumber, (char *) &glwkRec);
	}
	/*---------------------------------
	| Print  journal control records. |
	---------------------------------*/
	if (noDataFound == 1)
		PrintTotal ();

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

	open_rec (comr,  comr_list, COMR_NO_FIELDS, "comr_co_no");
	strcpy (comr_rec.co_no, comm_rec.co_no);
	cc = find_rec (comr, &comr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, comr, "DBFIND");

	if (!strcmp (comr_rec.base_curr, "   "))
		sprintf (baseCurr, "%-3.3s", get_env ("CURR_CODE"));
	else
		sprintf (baseCurr, "%-3.3s", comr_rec.base_curr);

	abc_fclose (comr);
	OpenGljc ();
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
PrintJournalDetails (
 void)
{
	int	type;

	if (strcmp (glwkRec.co_no,comm_rec.co_no))
		return (EXIT_SUCCESS);

	if (headingFlag != 1)
	{
		if (Heading ())
			return (-1);
	}

	strcpy (pageStart, glwkRec.sys_ref);
	pageNumber = atol (pageStart);
	if (pageNumber != oldPageNumber)
	{
		fprintf (ftmp,".PG%ld\n",pageNumber);
		oldPageNumber = pageNumber;
	}

	/*-------------------------------------------
 	| Print Exchange Variance Details.          |
	-------------------------------------------*/
	if (strncmp (glwkRec.acc_no,"                 ",MAXLEVEL))
	{
		fprintf (ftmp,"|%-10.10s",glwkRec.sys_ref);
		fprintf (ftmp,"|%-20.20s",glwkRec.narrative);
		fprintf (ftmp,"|%-15.15s",glwkRec.user_ref);
		fprintf (ftmp,"|%8.4f ", glwkRec.o1_amt);
		fprintf (ftmp,"|%8.4f ", glwkRec.o2_amt);

		type = atoi (glwkRec.jnl_type);
		if ( (type % 2) == 0)
		{
			fprintf (ftmp,"|                |              ");
			fprintf (ftmp,"|%-16.16s",glwkRec.acc_no);
			fprintf (ftmp,"| %12.2f ",DOLLARS (glwkRec.amount));
			creditTotal += glwkRec.amount;
		}
		else
		{
			fprintf (ftmp,"|%-16.16s",glwkRec.acc_no);
			fprintf (ftmp,"| %12.2f ",DOLLARS (glwkRec.amount));
			fprintf (ftmp,"|                |              ");
			debitTotal += glwkRec.amount;
		}
		fprintf (ftmp,"| %2.2d",mth2fisc (atoi (glwkRec.period_no),
						 comm_rec.fiscal));

		fprintf (ftmp,"|\n");
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
	fprintf (ftmp, "|==========|====================|===============|=========|=========|");
	fprintf (ftmp,"================|==============|");
	fprintf (ftmp,"================|==============|===|\n");

	fprintf (ftmp, "|   T O D A Y S   T O T A L S   |               |         |         |");
	fprintf (ftmp,"                |%13.2f |", DOLLARS (debitTotal));
	fprintf (ftmp,"                |%13.2f |   |\n", DOLLARS (creditTotal));

	totalAmount [0]	=	gljcRec.tot_1 + 
				 	 	gljcRec.tot_3 + 
				 	 	gljcRec.tot_5 - 
						debitTotal;

	totalAmount [1]	=	gljcRec.tot_4 + 
				 	 	gljcRec.tot_2 + 
				 	 	gljcRec.tot_6 - 
						creditTotal;

	fprintf (ftmp, "|   P R I O R  B A L A N C E S  |               |         |         |");
	fprintf (ftmp,"                |%13.2f |", DOLLARS (totalAmount [0]));
	fprintf (ftmp,"                |%13.2f |   |\n", DOLLARS (totalAmount [1]));

	totalAmount [0]	=	gljcRec.tot_1 + 
				 		gljcRec.tot_3 + 
				 		gljcRec.tot_5;

	totalAmount [1] = 	gljcRec.tot_4 + 
						gljcRec.tot_2 + 
						gljcRec.tot_6;

	fprintf (ftmp, "|   M O N T H   T O    D A T E  |               |         |         |");
	fprintf (ftmp,"                |%13.2f |", DOLLARS (totalAmount [0]));
	fprintf (ftmp,"                |%13.2f |   |\n", DOLLARS (totalAmount [1]));

	fprintf (ftmp,".EOF\n");

	pclose (ftmp);
}

/*========================= 
| Print journal headings. |
=========================*/
int
Heading (
 void)
{
	cc = FindPocr (comm_rec.co_no, baseCurr, "r");
	if (cc)
		strcpy (pocrRec.description, " ");
	
	/*---------------------------------
	| Open pipe work file to pformat. |
	---------------------------------*/
	strcpy (pageStart,glwkRec.sys_ref);
	pageNumber = atol (pageStart);
	oldPageNumber = pageNumber ;
	headingFlag = 1;

	if ( (ftmp = popen ("pformat","w")) == NULL)
	{
		sys_err ("Error in pformat During (POPEN)", 1, PNAME);
		shutdown_prog ();
		return (EXIT_FAILURE);
	}

	/*-----------------------
	| Start output to file. |
	-----------------------*/
	fprintf (ftmp,".START%s<%s>\n", DateToString (comm_rec.crd_date), PNAME);
	fprintf (ftmp,".LP%d\n", printerNumber);
	fprintf (ftmp,".SO\n");
	fprintf (ftmp,".14\n");
	fprintf (ftmp,".L158\n");
	fprintf (ftmp,".PG%ld\n",pageNumber);
	fprintf (ftmp,".B1\n");
	fprintf (ftmp,".E%s JOURNAL (BATCH NO. : %s)\n",
				clip (gljcRec.jnl_desc), glwkRec.run_no);
	fprintf (ftmp,".B1\n");
	fprintf (ftmp,".E%s AS AT: %s\n",clip (comm_rec.co_short), SystemTime ());

	fprintf (ftmp,".EFOR LOCAL CURRENCY CODE %s - %s\n", pocrRec.code, pocrRec.description);
	fprintf (ftmp,".B1\n");

	fprintf (ftmp, ".R=====================================================================");
	fprintf (ftmp,"================================");
	fprintf (ftmp,"====================================\n");

	fprintf (ftmp, "=====================================================================");
	fprintf (ftmp,"================================");
	fprintf (ftmp,"====================================\n");

	fprintf (ftmp, "|        E X C H A N G E    V A R I A N C E    J O U R N A L        |");
	fprintf (ftmp,"        D E B I T S            |");
	fprintf (ftmp,"        C R E D I T S          |PER|\n");

	fprintf (ftmp, "|  SYSREF  |   G/L NARRATIVE    |USER REFERENCE | OLD RATE| NEW RATE|");
	fprintf (ftmp, "    ACCOUNT     |    AMOUNT    |");
	fprintf (ftmp, "    ACCOUNT     |    AMOUNT    |NO.|\n");

	fprintf (ftmp, "|----------|--------------------|---------------|---------|---------|");
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

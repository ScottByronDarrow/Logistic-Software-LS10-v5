/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: db_winvjnl.c,v 5.4 2001/08/20 23:10:28 scott Exp $
|  Program Name  : (db_winvjnl.c & db_wcrdjnl.c)                      |
|  Program Desc  : (Print Customer Sales & Sales Returns Journal)     |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
| $Log: db_winvjnl.c,v $
| Revision 5.4  2001/08/20 23:10:28  scott
| Updated for development related to bullet proofing
|
| Revision 5.3  2001/08/09 09:02:14  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:22:31  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:17:17  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_winvjnl.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_winvjnl/db_winvjnl.c,v 5.4 2001/08/20 23:10:28 scott Exp $";

#include 	<pslscr.h>
#include 	<GlUtils.h>
#include 	<twodec.h>
#include 	<ml_std_mess.h>
#include 	<ml_db_mess.h>
#include    <DateToString.h>

#define		INVOICE	(typeFlag [0] == 'I')
#define		GST		(gstApplies [0] == 'Y')

	/*================================================================
	| Special fields and flags  ################################## . |
	================================================================*/
   	int		processID		=	0,
			glwk_no			=	0,
			printerNumber 	=	1,
    		printPipeOpen 	=	FALSE,
			envVarDbMcurr 	=	FALSE,
			envVarDbFind 	=	0,
			firstTime 		=	FALSE;

    long	pageNumber		=	0L,
    		oldPageNumber	=	0L;

   	char 	branchNo [3],
   			typeFlag [2],
   			pageStart [sizeof glwkRec.sys_ref],
			gstApplies [2],
			gstCode [4];

   	double 	totalAmount [6],
   			creditTotal		= 0.00,
   			debitTotal		= 0.00,
			taxTotal		= 0.00,
			freightTotal 	= 0.00,
			gstTotal		= 0.00,
			discountTotal 	= 0.00,
			otherTotal 		= 0.00,
			nettTotal 		= 0.00,
			calculateAmount = 0.00;

   	FILE 	*ftmp;

#include	"schema"

struct commRecord	comm_rec;
struct cumrRecord	cumr_rec;
struct extrRecord	extr_rec;

	Money	*gljc_tots	=	&gljcRec.tot_1;
	char	*data = "data";

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
int 	OpenDB 				(void);
void 	CloseDB 			(void);
int 	ReadGljc 			(void);
int 	PrintReportDetail 	(void);
void 	PrintReportTotal 	(void);
int 	ReportHeading 		(void);
void 	GetCustomer 		(void);
void 	shutdown_prog 		(void);
void 	AddTaxRecord 		(char *,char *,char *,char *,char *,char *,long,double,double);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
	int		argc,
	char	*argv [])
{
	char	*sptr;
	int	no_data = 0;

	if (argc < 4)
	{
		print_at (0,0, mlDbMess163, argv [0]);
        return (EXIT_FAILURE);
	}
	printerNumber 	= atoi (argv [1]);
	processID   	= atoi (argv [2]);
	sprintf (typeFlag, "%-1.1s",argv [3]);

	if (typeFlag [0] != 'I' && typeFlag [0] != 'C')
	{
		print_at (0,0, mlDbMess164, argv [0]);
        return (EXIT_FAILURE);
	}
	init_scr ();
	set_tty ();

	strcpy (gljcRec.journ_type, (INVOICE) ? " 4" : " 5");

	sprintf (gstApplies, "%-1.1s", get_env ("GST"));

	if (GST)
		sprintf (gstCode, "%-3.3s", get_env ("GST_TAX_NAME"));
	else
		sprintf (gstCode, "%-3.3s", "TAX");

	if (OpenDB ())
	{
		rset_tty ();
        return (EXIT_FAILURE);
	}
        
	envVarDbFind = atoi (get_env ("DB_FIND"));

	/*---------------------------------------
	| Check for Multi currency environment. |
	---------------------------------------*/
	sptr = chk_env ("DB_MCURR");
	envVarDbMcurr = (sptr == (char *)0) ? 0 : atoi (sptr);

	strcpy (branchNo, comm_rec.est_no);

	if (INVOICE)
		print_mess (ML ("Printing Customer Sales Journals."));
	else
		print_mess (ML ("Printing Customer Sales Returns Journals."));

	cc = FindGljc (comm_rec.co_no, (INVOICE) ? " 4" : " 5", "r");
    if (cc)
    {
        shutdown_prog (); 
        return (EXIT_FAILURE);
    }
	
	firstTime = TRUE;
	strcpy (cumr_rec.dbt_no, "      ");
	cc = RF_READ (glwk_no, (char *) &glwkRec);
	while (!cc)
	{
		if (!strcmp (gljcRec.journ_type,glwkRec.tran_type))
		{
			no_data = 1;
            if (PrintReportDetail () == 1)
            {
                return (EXIT_FAILURE);
            }
		}

		cc = RF_READ (glwk_no, (char *) &glwkRec);
	}
	/*---------------------------------
	| Print  journal control records. |
	---------------------------------*/
	if (printPipeOpen)
		PrintReportTotal ();

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*======================
| Open Database Files. |
======================*/
int
OpenDB (void)
{
	char	filename [100];
	char	*sptr = getenv ("PROG_PATH");

	sprintf (filename,"%s/WORK/gl_work%05d", 
				(sptr == (char *) 0) ? "/usr/LS10.5" : sptr, processID);

	cc = RF_OPEN (filename,sizeof (glwkRec),"r",&glwk_no);
	if (cc) 
        return (EXIT_FAILURE);

	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	OpenGljc ();
	open_rec (extr, extr_list, EXTR_NO_FIELDS, "extr_id_no");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, (!envVarDbFind) 
							? "cumr_id_no" : "cumr_id_no3");
	return (EXIT_SUCCESS);
}

/*=======================
| Close Database Files. |
=======================*/
void
CloseDB (void)
{
	cc = RF_CLOSE (glwk_no);
	if (cc) 
		file_err (cc, "glwk_no", "RF_CLOSE");

	abc_fclose (cumr);
	abc_fclose (extr);
	GL_Close ();
	abc_dbclose (data);
}
int
PrintReportDetail (void)
{
	double	exchangeRate = 0.00;
	int		journalType;

	if (strcmp (glwkRec.co_no, comm_rec.co_no))
		return (EXIT_SUCCESS);

	if (strcmp (glwkRec.est_no, branchNo))
		return (EXIT_SUCCESS);

	if (!printPipeOpen)
    {
		if (ReportHeading () == 1)
        {
            return (EXIT_FAILURE);
        }
    }

	strcpy (pageStart,glwkRec.sys_ref);
	pageNumber = atol (pageStart);
	if (pageNumber != oldPageNumber)
	{
		fprintf (ftmp, ".PG%ld\n", pageNumber);
		oldPageNumber = pageNumber;
	}
	if (strncmp (cumr_rec.dbt_no, glwkRec.acronym, 6) &&
	    strcmp (glwkRec.acronym, "         "))
	{
		if (!firstTime)
		{
			fprintf (ftmp, "|        ");
			fprintf (ftmp, "|            ");
			fprintf (ftmp, "|            ");
			fprintf (ftmp, "|            ");
			fprintf (ftmp, "|            ");
			if (envVarDbMcurr)
			{
				fprintf (ftmp, "|         ");
				fprintf (ftmp, "|            ");
				fprintf (ftmp, "|            ");
				fprintf (ftmp, "|            ");
				fprintf (ftmp, "|            ");
			}
			fprintf (ftmp, "|                ");
			fprintf (ftmp, "|             |\n");
		}
		firstTime = FALSE;

		GetCustomer ();

		if (envVarDbMcurr)
		{
			fprintf (ftmp, 
				"| %6.6s (%40.40s)                    ",
				cumr_rec.dbt_no,
				cumr_rec.dbt_name);

			fprintf (ftmp, "                         ");
			fprintf (ftmp, "                         ");
			fprintf (ftmp, "                                 |\n");
		}
		else
		{
			fprintf (ftmp, 
				"| %6.6s (%40.40s)                ",
				cumr_rec.dbt_no,
				cumr_rec.dbt_name);
			fprintf (ftmp, "                         |\n");
		}
	}
	fprintf (ftmp,"|%-8.8s",glwkRec.chq_inv_no);

	if (strncmp (glwkRec.chq_inv_no,"        ", 8))
	{
		AddTaxRecord 
		(
			comm_rec.co_no,
			glwkRec.run_no,
			glwkRec.acronym,
			glwkRec.chq_inv_no,
			glwkRec.tran_type,
			glwkRec.period_no,
			glwkRec.tran_date,
			glwkRec.ci_amt,
			glwkRec.o4_amt
		);

		if (envVarDbMcurr)
			exchangeRate = glwkRec.exch_rate;
		else
			exchangeRate = 1.0;

		calculateAmount = DOLLARS (glwkRec.ci_amt);
		fprintf (ftmp,"|%12.2f", calculateAmount);

		calculateAmount = DOLLARS (glwkRec.o1_amt);
		fprintf (ftmp,"|%12.2f", calculateAmount);

		calculateAmount = DOLLARS (glwkRec.o4_amt);
		fprintf (ftmp,"|%12.2f", calculateAmount);

		calculateAmount = DOLLARS (glwkRec.o3_amt);
		fprintf (ftmp,"|%12.2f", calculateAmount);

		if (envVarDbMcurr)
		{
			fprintf (ftmp,"|%9.4f", exchangeRate);

			calculateAmount = DOLLARS (glwkRec.ci_amt / exchangeRate);
			fprintf (ftmp,"|%12.2f", calculateAmount);

			calculateAmount = DOLLARS (glwkRec.o1_amt / exchangeRate);
			fprintf (ftmp, "|%12.2f", calculateAmount);

			calculateAmount = DOLLARS (glwkRec.o4_amt / exchangeRate);
			fprintf (ftmp, "|%12.2f", calculateAmount);

			calculateAmount = DOLLARS (glwkRec.o3_amt / exchangeRate);
			fprintf (ftmp, "|%12.2f", calculateAmount);
		}

		nettTotal += no_dec (glwkRec.ci_amt / exchangeRate);
		if (GST)
			gstTotal += no_dec (glwkRec.o4_amt / exchangeRate);
		else
			taxTotal += no_dec (glwkRec.o4_amt / exchangeRate);

		freightTotal 	+= no_dec (glwkRec.o3_amt / exchangeRate);
		discountTotal 	+= no_dec (glwkRec.o1_amt / exchangeRate);
		otherTotal  	+= no_dec (glwkRec.ci_amt / exchangeRate);
	}
	else
	{
		fprintf (ftmp, "|            ");
		fprintf (ftmp, "|            ");
		fprintf (ftmp, "|            ");
		fprintf (ftmp, "|            ");
		if (envVarDbMcurr)
		{
			fprintf (ftmp, "|         ");
			fprintf (ftmp, "|            ");
			fprintf (ftmp, "|            ");
			fprintf (ftmp, "|            ");
			fprintf (ftmp, "|            ");
		}
	}

	if (strncmp (glwkRec.acc_no, "                ",MAXLEVEL))
	{
		journalType = atoi (glwkRec.jnl_type);
		fprintf (ftmp,"|%-16.16s",glwkRec.acc_no);
		calculateAmount = DOLLARS (glwkRec.loc_amount);
		if ((journalType % 2) == 0)
		{
			creditTotal += glwkRec.loc_amount;
			calculateAmount *= -1.00;
		}
		else
			debitTotal += glwkRec.loc_amount;
		fprintf (ftmp,"|%13.2f", calculateAmount);

		if (glwkRec.narrative [0] == '*')
			fprintf (ftmp,"|***\n");
		else
			fprintf (ftmp,"|\n");
	}
	else
		fprintf (ftmp, "|                  |               |\n");
    return (EXIT_SUCCESS);
}

void
PrintReportTotal (void)
{
	fprintf (ftmp, "|--------");
	fprintf (ftmp, "|------------");
	fprintf (ftmp, "|------------");
	fprintf (ftmp, "|------------");
	fprintf (ftmp, "|------------");
	if (envVarDbMcurr)
	{
		fprintf (ftmp, "|---------");
		fprintf (ftmp, "|------------");
		fprintf (ftmp, "|------------");
		fprintf (ftmp, "|------------");
		fprintf (ftmp, "|------------");
	}
	fprintf (ftmp, "|----------------");
	fprintf (ftmp, "|-------------|\n");

	fprintf (ftmp, "|        ");
	if (envVarDbMcurr)
	{
		fprintf (ftmp, "|            ");
		fprintf (ftmp, "|            ");
		fprintf (ftmp, "|            ");
		fprintf (ftmp, "|            ");
		fprintf (ftmp, "|         ");
	}
	fprintf (ftmp, "|%12.2f", DOLLARS (nettTotal));
	fprintf (ftmp, "|%12.2f", DOLLARS (discountTotal));
	fprintf (ftmp, "|%12.2f", DOLLARS (taxTotal + gstTotal));
	fprintf (ftmp, "|%12.2f", DOLLARS (freightTotal));

	fprintf (ftmp, "|                ");
	fprintf (ftmp, "|             |\n");
		
	fprintf (ftmp, "|========");
	fprintf (ftmp, "|============");
	fprintf (ftmp, "|============");
	fprintf (ftmp, "|============");
	fprintf (ftmp, "|============");
	if (envVarDbMcurr)
	{
		fprintf (ftmp, "|=========");
		fprintf (ftmp, "|============");
		fprintf (ftmp, "|============");
		fprintf (ftmp, "|============");
		fprintf (ftmp, "|============");
	}
	fprintf (ftmp, "|================");
	fprintf (ftmp, "|=============|\n");

	fprintf (ftmp, "|        ");
	fprintf (ftmp, "|T O D A Y S  T O T A L S ");
	fprintf (ftmp, "|            ");
	fprintf (ftmp, "|            ");
	if (envVarDbMcurr)
	{
		fprintf (ftmp, "|         ");
		fprintf (ftmp, "|            ");
		fprintf (ftmp, "|            ");
		fprintf (ftmp, "|            ");
		fprintf (ftmp, "|            ");
	}

	fprintf (ftmp, "|                ");
	fprintf (ftmp, "|%13.2f|\n", DOLLARS (nettTotal));

	if (INVOICE)
	{
		totalAmount [3] = 	gljc_tots [3] + 
							gljc_tots [1] + 
							gljc_tots [5] - 
							creditTotal;

		totalAmount [0] = 	gljc_tots [0] + 
							gljc_tots [2] - 
							debitTotal;
	}
	else
	{
		totalAmount [0] = 	gljc_tots [0] + 
							gljc_tots [2] + 
							gljc_tots [4] - 
							debitTotal;

		totalAmount [1] = 	gljc_tots [1] + 
							gljc_tots [3] - 
							creditTotal;
	}

	fprintf (ftmp, "|        ");
	fprintf (ftmp, "|P R I O R  B A L A N C E ");
	fprintf (ftmp, "|            ");
	fprintf (ftmp, "|            ");
	if (envVarDbMcurr)
	{
		fprintf (ftmp, "|         ");
		fprintf (ftmp, "|            ");
		fprintf (ftmp, "|            ");
		fprintf (ftmp, "|            ");
		fprintf (ftmp, "|            ");
	}

	fprintf (ftmp, "|                ");
	fprintf (ftmp, "|%13.2f|\n",    DOLLARS (totalAmount [0]));

	if (INVOICE)
	{
		totalAmount [3] = gljc_tots [3] + gljc_tots [1] + gljc_tots [5];
		totalAmount [0] = gljc_tots [0] + gljc_tots [2];
	}
	else
	{
		totalAmount [0] = gljc_tots [0] + gljc_tots [2] + gljc_tots [4];
		totalAmount [1] = gljc_tots [1] + gljc_tots [3];
	}

	fprintf (ftmp, "|        ");
	fprintf (ftmp, "|M O N T H   T O   D A T E");
	fprintf (ftmp, "|            ");
	fprintf (ftmp, "|            ");
	if (envVarDbMcurr)
	{
		fprintf (ftmp, "|         ");
		fprintf (ftmp, "|            ");
		fprintf (ftmp, "|            ");
		fprintf (ftmp, "|            ");
		fprintf (ftmp, "|            ");
	}

	fprintf (ftmp, "|                ");
	fprintf (ftmp, "|%13.2f|\n", DOLLARS (totalAmount [0]));
	fprintf (ftmp,".EOF\n");

	pclose (ftmp);
}

/*========================= 
| Print journal headings. |
=========================*/
int
ReportHeading (void)
{
	strcpy (pageStart,glwkRec.sys_ref);

	pageNumber 		= atol (pageStart);
	oldPageNumber 	= pageNumber ;
	printPipeOpen 	= TRUE;

	if ((ftmp = popen ("pformat","w")) == NULL)
	{
		sys_err ("Error in pformat During (POPEN)", 1, PNAME);
		shutdown_prog ();
        return (EXIT_FAILURE);
	}

	/*-----------------------
	| Start output to file. |
	-----------------------*/
	fprintf (ftmp, ".START%s<%s>\n", DateToString (comm_rec.dbt_date), PNAME);
	fprintf (ftmp,".LP%d\n", printerNumber);
	fprintf (ftmp,".SO\n");
	fprintf (ftmp,".PI16\n");
	fprintf (ftmp,".L158\n");
	fprintf (ftmp,".PL65\n");
	fprintf (ftmp,".11\n");
	fprintf (ftmp,".PG%ld\n",pageNumber);
	fprintf (ftmp,".B1\n");
	fprintf (ftmp,".ECUSTOMER %s JOURNAL (BATCH NO : %s)\n",
				clip (gljcRec.jnl_desc), glwkRec.run_no);
	fprintf (ftmp,".B1\n");
	fprintf (ftmp,".E%s AS AT: %s\n",clip (comm_rec.co_short), SystemTime ());
	fprintf (ftmp,".B1\n");

	if (envVarDbMcurr)
	{
		fprintf (ftmp, ".R=========");
		fprintf (ftmp, "=============");
		fprintf (ftmp, "=============");
		fprintf (ftmp, "=============");
		fprintf (ftmp, "=============");
		fprintf (ftmp, "==========");
		fprintf (ftmp, "=============");
		fprintf (ftmp, "=============");
		fprintf (ftmp, "=============");
		fprintf (ftmp, "=============");
		fprintf (ftmp, "=================");
		fprintf (ftmp, "===============\n");

		fprintf (ftmp, "=========");
		fprintf (ftmp, "=============");
		fprintf (ftmp, "=============");
		fprintf (ftmp, "=============");
		fprintf (ftmp, "=============");
		fprintf (ftmp, "==========");
		fprintf (ftmp, "=============");
		fprintf (ftmp, "=============");
		fprintf (ftmp, "=============");
		fprintf (ftmp, "=============");
		fprintf (ftmp, "=================");
		fprintf (ftmp, "===============\n");

		fprintf (ftmp, "|            %41.41s       ", (INVOICE)
			? "        I   N   V   O   I   C   E        " 
			: "         C    R    E    D   I  T         ");
		fprintf (ftmp, "|EXCHANGE ");
		fprintf (ftmp, "|                    V A L U E S      ");
		fprintf (ftmp, "              ");
		fprintf (ftmp, "|    G / L   A C C O U N T S   |\n");

		fprintf (ftmp, "| NUMBER | FOREIGN AMT|DISCOUNT AMT");
		fprintf (ftmp, "| %3.3s AMOUNT.|OTHER CHARGE", gstCode);
		fprintf (ftmp, "|  RATE   ");
		fprintf (ftmp, "|  LOCAL AMT ");
		fprintf (ftmp, "|DISCOUNT AMT");
		fprintf (ftmp, "| TAX AMOUNT ");
		fprintf (ftmp, "|OTHER CHARGE");
		fprintf (ftmp, "|     ACCOUNT    ");
		fprintf (ftmp, "|   AMOUNT    |\n");

		fprintf (ftmp, "|--------");
		fprintf (ftmp, "|------------");
		fprintf (ftmp, "|------------");
		fprintf (ftmp, "|------------");
		fprintf (ftmp, "|------------");
		fprintf (ftmp, "|---------");
		fprintf (ftmp, "|------------");
		fprintf (ftmp, "|------------");
		fprintf (ftmp, "|------------");
		fprintf (ftmp, "|------------");
		fprintf (ftmp, "|----------------");
		fprintf (ftmp, "|-------------|\n");
	}
	else
	{
		fprintf (ftmp, ".R=========");
		fprintf (ftmp, "=============");
		fprintf (ftmp, "=============");
		fprintf (ftmp, "=============");
		fprintf (ftmp, "=============");
		fprintf (ftmp, "=================");
		fprintf (ftmp, "===============\n");

		fprintf (ftmp, "=========");
		fprintf (ftmp, "=============");
		fprintf (ftmp, "=============");
		fprintf (ftmp, "=============");
		fprintf (ftmp, "=============");
		fprintf (ftmp, "=================");
		fprintf (ftmp, "======================\n");

		fprintf (ftmp, "|          %45.45s       ", (INVOICE)
			? "       I   N   V   O   I   C   E        " 
			: "           C    R    E    D    I    T       ");
		fprintf (ftmp, "|    G / L  A C C O U N T S    |\n");

		fprintf (ftmp, "| NUMBER ");
		fprintf (ftmp, "|   AMOUNT   ");
		fprintf (ftmp, "|DISCOUNT AMT");
		fprintf (ftmp, "| %3.3s AMOUNT ", gstCode);
		fprintf (ftmp, "|OTHER CHARGE");
		fprintf (ftmp, "|     ACCOUNT    ");
		fprintf (ftmp, "|   AMOUNT    |\n");

		fprintf (ftmp, "|--------");
		fprintf (ftmp, "|------------");
		fprintf (ftmp, "|------------");
		fprintf (ftmp, "|------------");
		fprintf (ftmp, "|------------");
		fprintf (ftmp, "|----------------");
		fprintf (ftmp, "|-------------|\n");
	}
    return (EXIT_SUCCESS);
}

void
GetCustomer (void)
{
	strcpy (cumr_rec.co_no,glwkRec.co_no);
	strcpy (cumr_rec.est_no,glwkRec.est_no);
	sprintf (cumr_rec.dbt_no, "%-6.6s", glwkRec.acronym);

	cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
	if (cc)
	{
		strcpy (cumr_rec.est_no," 0");
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
	}
	if (cc)
		sprintf (glwkRec.name,"%-30.30s", " ");
	else
		sprintf (glwkRec.name,"%-30.30s", cumr_rec.dbt_name);
}

void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}
/*=====================
| Add tax file audit. |
=====================*/
void
AddTaxRecord	(
	char 	*coNo,
	char	*runNo,
	char	*intNo,
	char	*refNo,
	char	*jnlType,
	char	*period,
	long	date,
	double	gross,
	double	gst)
{
	gross = no_dec (gross);
	gst   = no_dec (gst);
	strcpy (extr_rec.co_no, coNo);
	extr_rec.run_no = atol (runNo);
	sprintf (extr_rec.int_no, "%-8.8s", intNo);
	sprintf (extr_rec.ref_no, "%-15.15s", refNo);
	strcpy (extr_rec.jnl_type, jnlType);
	sprintf (extr_rec.gl_per, "%2.2d", mth2fisc (atoi (period), comm_rec.fiscal));
	extr_rec.date = date;
	extr_rec.sal_val = DOLLARS (gross);
	extr_rec.gst_val = DOLLARS (gst);
	strcpy (extr_rec.stat_flag,"0");

	cc = find_rec (extr, &extr_rec, COMPARISON, "r");
	if (cc)
	{
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

/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: db_crc2jnl.c,v 5.3 2001/08/09 08:23:28 scott Exp $
|  Program Name  : (db_crc2jnl.c) 
|  Program Desc  : (Prints Sundry Receipts Journal)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
| $Log: db_crc2jnl.c,v $
| Revision 5.3  2001/08/09 08:23:28  scott
| Added FinishProgram ();
|
| Revision 5.2  2001/08/06 23:21:50  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:17:02  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_crc2jnl.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_crc2jnl/db_crc2jnl.c,v 5.3 2001/08/09 08:23:28 scott Exp $";

#include	<pslscr.h>
#include	<GlUtils.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include	<ml_std_mess.h>
#include	<ml_db_mess.h>

	/*
	 * Special fields and flags  ##################################
	 */
   	int		pid				= 0,	
			glwkNo			= 0,
     		printerNumber 	= 1,
    		headingFlag 	= 0;

    long	pageNo			= 0,
    		oldPageNo		= 0;

   	double 	totalAmount [5]	=	{0.0,0.0,0.0,0.0,0.0},
   			creditTotal 	= 0,
   			debitTotal		= 0;

   	char 	pageStart [sizeof glwkRec.sys_ref];

   	FILE 	*ftmp;

#include	"schema"

struct commRecord	comm_rec;
struct cumrRecord	cumr_rec;

	char	*data = "data";

	char	branchNo [3];

	int		envDbCo = 0,
			envDbFind = 0;


/*
 * Local Function Prototypes.
 */
int 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	PrintTotals 	(void);
void 	shutdown_prog 	(void);
void	PrintExtraDesc 	(char *);
int 	ReportHeader 	(void);
int 	PrintDetails 	(void);

int
main (
	int		argc,
	char	*argv [])
{
	int		noData = TRUE;

	init_scr ();
	if (argc < 3)
	{
		print_at (0,0,mlDbMess002, argv [0]);
		return (EXIT_FAILURE);
	}
	printerNumber = atoi (argv [1]);
	pid   = atoi (argv [2]);

	if (OpenDB ())
	{
		GL_Close ();
		abc_fclose (cumr);
		abc_dbclose (data);
		shutdown_prog ();
        return (EXIT_FAILURE);
	}

    /*
     * Get common info from commom database file.
     */
    read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	envDbFind 	= atoi (get_env ("DB_FIND"));
	envDbCo 	= atoi (get_env ("DB_CO"));
	strcpy (branchNo, (envDbCo) ? comm_rec.est_no : " 0");
 
	print_mess (ML ("Printing Customer Sundry Receipts Journals."));

	if (FindGljc (comm_rec.co_no, "26", "r"))
    {
		errmess (ML (mlDbMess001));
		sleep (sleepTime);
		shutdown_prog ();
        return (EXIT_FAILURE);
    }
	
	cc = RF_READ (glwkNo, (char *) &glwkRec);
	while (!cc)
	{
		noData = FALSE;
		if (PrintDetails () == 1)
        {
            return (EXIT_FAILURE);
        }
		dsp_process ("Account : ",glwkRec.acc_no);
		cc = RF_READ (glwkNo, (char *) &glwkRec);
	}

	/*
	 * Print  journal control records.
	 */
	if (noData == FALSE)
		PrintTotals (); 


	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*
 * Open Database Files.
 */
int
OpenDB (void)
{
	char	filename [100];
	char	*sptr = getenv ("PROG_PATH");

	abc_dbopen (data);

	sprintf (filename,"%s/WORK/gl_work%05d", 
				 (sptr == (char *) 0) ? "/usr/LS10.5" : sptr, pid);

	OpenGljc ();
	OpenGlmr ();
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, (!envDbFind) ? "cumr_id_no"
													     : "cumr_id_no3");
	return (RF_OPEN (filename,sizeof (glwkRec),"r",&glwkNo));
}

/*
 * Close Database Files.
 */
void
CloseDB (void)
{
	GL_Close ();
	abc_fclose (cumr);
	cc = RF_CLOSE (glwkNo);
	if (cc) 
		file_err (cc, "glwkNo", "RF_CLOSE");

	abc_dbclose (data);
}

int
PrintDetails (void)
{
	int		jnlType;

	if (strcmp (glwkRec.co_no,comm_rec.co_no))
		return (EXIT_SUCCESS);

	if (headingFlag != 1)
    {
        if (ReportHeader () == 1)
        {
            return (EXIT_FAILURE);
        }
    }

	strcpy (pageStart,glwkRec.sys_ref);
	pageNo	 = atol (pageStart);
	jnlType	 = atoi (glwkRec.jnl_type);
	if (pageNo != oldPageNo)
	{
		fprintf (ftmp,".PG%ld\n",pageNo);
		oldPageNo = pageNo;
	}

	if (strncmp (glwkRec.acc_no, "                ",MAXLEVEL))
	{
		strcpy (glmrRec.co_no, comm_rec.co_no);
		sprintf (glmrRec.acc_no, "%-*.*s", 
									MAXLEVEL,MAXLEVEL,glwkRec.acc_no);
		cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
		if (cc)
			print_at (0,0,ML (mlStdMess009));
	
		if (!strcmp (glwkRec.chq_inv_no, "               "))
		{
			fprintf (ftmp,"|        |                              ");
		}
		else
		{
			fprintf (ftmp,"|%-8.8s",glwkRec.chq_inv_no);
			fprintf (ftmp,"|%-30.30s",glwkRec.name);
		}
		fprintf (ftmp,"|%-16.16s",glwkRec.acc_no);
		fprintf (ftmp,"|%-25.25s",glmrRec.desc);
		if (jnlType % 2)
		{
			if (glwkRec.amount < 0.00)
			{
				fprintf (ftmp,"|              ");
				fprintf (ftmp,"|%13.2f ",DOLLARS (glwkRec.loc_amount * -1));
				fprintf (ftmp,"|%s ", glwkRec.currency);
				fprintf (ftmp,"|              ");
				fprintf (ftmp,"|%13.2f ",DOLLARS (glwkRec.amount * -1));
				debitTotal += glwkRec.loc_amount;
			}
			else
			{
				fprintf (ftmp,"|%13.2f ",DOLLARS (glwkRec.loc_amount));
				fprintf (ftmp,"|              ");
				fprintf (ftmp,"|%s ", glwkRec.currency);
				fprintf (ftmp,"|%13.2f ",DOLLARS (glwkRec.amount));
				fprintf (ftmp,"|              ");
				debitTotal += glwkRec.loc_amount;
			}
		}
		else
		{
			if (glwkRec.amount < 0.00)
			{
				fprintf (ftmp,"|%13.2f ",DOLLARS (glwkRec.loc_amount * -1));
				fprintf (ftmp,"|              ");
				fprintf (ftmp,"|%s ", glwkRec.currency);
				fprintf (ftmp,"|%13.2f ",DOLLARS (glwkRec.amount * -1));
				fprintf (ftmp,"|              ");
				creditTotal += glwkRec.loc_amount;
			}
			else
			{
				fprintf (ftmp,"|              ");
				fprintf (ftmp,"|%13.2f ",DOLLARS (glwkRec.loc_amount));
				fprintf (ftmp,"|%s ", glwkRec.currency);
				fprintf (ftmp,"|              ");
				fprintf (ftmp,"|%13.2f ",DOLLARS (glwkRec.amount));
				creditTotal += glwkRec.loc_amount;
			}
		}
		fprintf (ftmp,"| %2.2d|\n", 
					mth2fisc (atoi (glwkRec.period_no), comm_rec.fiscal));

		if (strlen (clip (glwkRec.alt_desc1)))
			PrintExtraDesc (glwkRec.alt_desc1);

		if (strlen (clip (glwkRec.alt_desc2)))
			PrintExtraDesc (glwkRec.alt_desc2);

		if (strlen (clip (glwkRec.alt_desc3)))
			PrintExtraDesc (glwkRec.alt_desc3);
	}
    return (EXIT_SUCCESS);
}

void
PrintTotals (void)
{
	double	total [2];

	total [0] = total [1] = 0.00;

	totalAmount [3] = 	gljcRec.tot_2 + 
				 		gljcRec.tot_4 + 
				 		gljcRec.tot_6 - 
				 		creditTotal;
	totalAmount [0] = 	gljcRec.tot_1 + 
				 		gljcRec.tot_2 + 
				 		gljcRec.tot_5 - 
				 		debitTotal;

	fprintf (ftmp,"|--------|------------------------------");
	fprintf (ftmp,"|----------------");
	fprintf (ftmp,"|-------------------------");
	fprintf (ftmp,"|--------------|--------------|----");
	fprintf (ftmp,"|--------------|--------------|---|\n");

	fprintf (ftmp,"|        |                              ");
	fprintf (ftmp,"|                ");
	fprintf (ftmp,"| BROUGHT FORWARD         ");
	fprintf (ftmp,"|%13.2f |%13.2f |    ",
						DOLLARS (totalAmount [0]), DOLLARS (totalAmount [3]));
	fprintf (ftmp,"|              |              |   |\n");

	fprintf (ftmp,"|        |                              ");
	fprintf (ftmp,"|                ");
	fprintf (ftmp,"| THIS RUN                ");
	fprintf (ftmp,"|%13.2f |%13.2f |    ",
						DOLLARS (debitTotal), DOLLARS (creditTotal));
	fprintf (ftmp,"|              |              |   |\n");


	total [0] = debitTotal + totalAmount [0]; 
	total [1] = creditTotal + totalAmount [3];

	fprintf (ftmp,"|        |                              ");
	fprintf (ftmp,"|                ");
	fprintf (ftmp,"| CARRIED FORWARD         ");
	fprintf (ftmp,"|%13.2f |%13.2f |    ",
						DOLLARS (total [0]), DOLLARS (total [1]));
	fprintf (ftmp,"|              |              |   |\n");

	fprintf (ftmp,".EOF\n");

	/*  program exit sequence	*/

	pclose (ftmp);
}

/*
 * Print journal headings.
 */
int
ReportHeader (void)
{
	strcpy (pageStart,glwkRec.sys_ref);
	pageNo		=	atol (pageStart);
	oldPageNo 	= 	pageNo ;
	headingFlag = 1;

	if ( (ftmp = popen ("pformat","w")) == 0)
	{
		file_err (errno, "pformat", "popen");
		shutdown_prog ();
        return (EXIT_FAILURE);
	}

	fprintf (ftmp, ".START%s<%s>\n", DateToString (comm_rec.dbt_date), PNAME);
	fprintf (ftmp,".LP%d\n", printerNumber);
	fprintf (ftmp,".PI12\n");
	fprintf (ftmp,".SO\n");
	fprintf (ftmp,".13\n");
	fprintf (ftmp,".L158\n");
	fprintf (ftmp,".PG%ld\n",pageNo);
	fprintf (ftmp,".B1\n");
	fprintf (ftmp,".EGENERAL LEDGER AUDIT (BATCH NO : %s)\n", glwkRec.run_no);
	fprintf (ftmp,".B1\n");
	fprintf (ftmp,".E%s AS AT: %s\n",clip (comm_rec.co_short), SystemTime ());
	fprintf (ftmp,".B1\n");

	fprintf (ftmp,".R========================================");
	fprintf (ftmp,"=================");
	fprintf (ftmp,"==========================");
	fprintf (ftmp,"===================================");
	fprintf (ftmp,"===================================\n");

	fprintf (ftmp,"========================================");
	fprintf (ftmp,"=================");
	fprintf (ftmp,"==========================");
	fprintf (ftmp,"===================================");
	fprintf (ftmp,"===================================\n");

	fprintf (ftmp,"|RECEIPT | CUSTOMER NAME / REFERENCE    ");
	fprintf (ftmp,"| GENERAL LEDGER ");
	fprintf (ftmp,"|  ACCOUNT DESCRIPTION    ");
	fprintf (ftmp,"| LOCAL AMOUNT | LOCAL AMOUNT |CURR");
	fprintf (ftmp,"|  FGN AMOUNT  |  FGN AMOUNT  |PER|\n");

	fprintf (ftmp,"| NUMBER |                              ");
	fprintf (ftmp,"| ACCOUNT NUMBER ");
	fprintf (ftmp,"|                         ");
	fprintf (ftmp,"|    DEBIT     |    CREDIT    |CODE");
	fprintf (ftmp,"|    DEBIT     |    CREDIT    |NO.|\n");

	fprintf (ftmp,"|--------|------------------------------");
	fprintf (ftmp,"|----------------");
	fprintf (ftmp,"|-------------------------");
	fprintf (ftmp,"|--------------|--------------|----");
	fprintf (ftmp,"|--------------|--------------|---|\n");
    return (EXIT_SUCCESS);
}

void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

void
PrintExtraDesc (
	char	*extraDesc)
{
	fprintf (ftmp,"|        |%-30.30s", extraDesc);
	fprintf (ftmp,"|                ");
	fprintf (ftmp,"|                         ");
	fprintf (ftmp,"|              |              |    ");
	fprintf (ftmp,"|              |              |   |\n");
}

/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: db_crcjnl.c,v 5.4 2001/10/09 08:36:56 robert Exp $
|  Program Name  : (db_crcjnl.c) 
|  Program Desc  : (Prints Receipts Journal)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
| $Log: db_crcjnl.c,v $
| Revision 5.4  2001/10/09 08:36:56  robert
| Updated continue execution when RF_CLOSE failed
|
| Revision 5.3  2001/08/09 08:23:38  scott
| Added FinishProgram ();
|
| Revision 5.2  2001/08/06 23:21:54  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:17:07  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_crcjnl.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_crcjnl/db_crcjnl.c,v 5.4 2001/10/09 08:36:56 robert Exp $";

#include	<pslscr.h>
#include	<GlUtils.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include	<ml_std_mess.h>
#include	<ml_db_mess.h>

	/*================================================================
	| Special fields and flags  ################################## . |
	================================================================*/
   	int		processID		= 0,
			glwk_no			= 0,	
     		printerNo 		= 1,
    		headingFlag 	= 0;

	long	pageNo			= 0L,
    		oldPageNo		= 0L;

   	double 	totalAmount [5]	= {0.00,0.00,0.00,0.00,0.00},
   			creditTotal 	= 0,
   			debitTotal 		= 0;

   	char 	customerName [31],
   			pageStart 	[sizeof glwkRec.sys_ref];

   	FILE 	*ftmp;

#include	"schema"

struct commRecord	comm_rec;
struct cumrRecord	cumr_rec;

	char	*data = "data";

	char	branchNo [3];

	int		envDbCo 	= 0,
			envDbFind 	= 0;

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
int 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	PrintTotals 	(void);
void 	ReadCustomer 	(void);
void 	shutdown_prog 	(void);
int 	PrintDetails 	(void);
int 	ReportHeading 	(void);

int
main (
	int		argc,
	char	*argv []) 
{
	int	no_data = TRUE;

	if (argc < 3)
	{
		print_at (0,0,mlDbMess002, argv [0]);
        return (EXIT_FAILURE);
	}
	printerNo 	= atoi (argv [1]);
	processID   = atoi (argv [2]);

	if (OpenDB ())
	{
		abc_dbclose ("data");
		shutdown_prog ();
        return (EXIT_FAILURE);
	}


	envDbFind 	= atoi (get_env ("DB_FIND"));
	envDbCo 	= atoi (get_env ("DB_CO"));
	strcpy (branchNo, (envDbCo) ? comm_rec.est_no : " 0");
 

	dsp_screen ("Printing Customer Receipts Journals.",comm_rec.co_no,
							 comm_rec.co_name);

	cc = FindGljc (comm_rec.co_no, " 6", "r");
	if (cc)
    {
		shutdown_prog ();
        return (EXIT_FAILURE);
    }
	
	cc = RF_READ (glwk_no, (char *) &glwkRec);
	while (!cc)
	{
		no_data = FALSE;
		if (PrintDetails () == 1)
        {
            return (EXIT_FAILURE);
        }
		dsp_process ("Account : ",glwkRec.acc_no);
		cc = RF_READ (glwk_no, (char *) &glwkRec);
	}

	/*---------------------------------- 
	| Print  journal control records . |
	----------------------------------*/
	if (no_data == FALSE)
    {
		PrintTotals ();
    }


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

	abc_dbopen (data);

	sprintf (filename,"%s/WORK/gl_work%05d", 
				 (sptr == (char *) 0) ? "/usr/LS10.5" : sptr, processID);
	cc = RF_OPEN (filename,sizeof (glwkRec),"r",&glwk_no);
	if (cc) 
        return (EXIT_FAILURE);

    read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	OpenGljc ();
	OpenGlmr ();
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, (!envDbFind) ? "cumr_id_no"
													      : "cumr_id_no3");
    return (EXIT_SUCCESS);
}

/*=======================
| Close Database Files. |
=======================*/
void
CloseDB (void)
{
	abc_fclose (cumr);
	cc = RF_CLOSE (glwk_no);

	GL_Close ();

	abc_dbclose (data);
}


int
PrintDetails (void)
{
	int	jnlType;

	if (strcmp (glwkRec.co_no,comm_rec.co_no))
    {
		return (EXIT_SUCCESS);
    }

	if (headingFlag != 1)
    {
		if (ReportHeading () == 1)
        {
            return (EXIT_FAILURE);
        }
    }

	strcpy (pageStart,glwkRec.sys_ref);
	pageNo 	= atol (pageStart);
	jnlType = atol (glwkRec.jnl_type);
	if (pageNo != oldPageNo)
	{
		fprintf (ftmp,".PG%ld\n",pageNo);
		oldPageNo = pageNo;
	}

	if (strcmp (glwkRec.chq_inv_no, "               "))
	{
		ReadCustomer ();

		fprintf (ftmp, "|%-6.6s|%-26.26s|%-8.8s|%12.2f |%12.2f ",
						glwkRec.acronym,
						cumr_rec.dbt_name,
						glwkRec.chq_inv_no,
						DOLLARS (glwkRec.ci_amt),
						DOLLARS (glwkRec.o1_amt));
		fprintf (ftmp,"|                |             |             | %2.2d",
				mth2fisc (atoi (glwkRec.period_no), comm_rec.fiscal));
		fprintf (ftmp,"|        |             |             |\n");

	}
	else
	{
		fprintf (ftmp,"|      |                          ");
		fprintf (ftmp,"|        |             |             ");
		if ( (jnlType % 2) == 0)
		{
			fprintf (ftmp,"|                |             |             |   ");
			fprintf (ftmp,"|%-8.8s|%12.2f |%12.2f |\n",
						glwkRec.name,
						DOLLARS (glwkRec.loc_amount),
						DOLLARS (glwkRec.amount));
			debitTotal	+=	glwkRec.loc_amount;
		}
		else
		{
			fprintf (ftmp,"|%16.16s|%12.2f |%12.2f | %2.2d",
						glwkRec.acc_no,
						DOLLARS (glwkRec.loc_amount),
						DOLLARS (glwkRec.amount),
						mth2fisc (atoi (glwkRec.period_no), comm_rec.fiscal));
			fprintf (ftmp,"|        |             |             |\n");
			creditTotal	+=	glwkRec.loc_amount;

		}
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
				 		gljcRec.tot_3 + 
				 		gljcRec.tot_5 - 
				 		debitTotal;

	fprintf (ftmp,"|------|--------------------------");
	fprintf (ftmp,"|--------|-------------|-------------");
	fprintf (ftmp,"|----------------|-------------|-------------|---");
	fprintf (ftmp,"|--------|-------------|-------------|\n");

	fprintf (ftmp,"|      | BROUGHT FORWARD          ");
	fprintf (ftmp,"|        |             |             ");
	fprintf (ftmp,"|                |%12.2f |             |   ",
						DOLLARS (totalAmount [0]));
	fprintf (ftmp,"|        |%12.2f |             |\n",
						DOLLARS (totalAmount [3]));

	fprintf (ftmp,"|      | THIS RUN                 ");
	fprintf (ftmp,"|        |             |             ");
	fprintf (ftmp,"|                |%12.2f |             |   ",
						DOLLARS (debitTotal));
	fprintf (ftmp,"|        |%12.2f |             |\n",
						DOLLARS (creditTotal));

	total [0] = debitTotal + totalAmount [0]; 
	total [1] = creditTotal + totalAmount [3];

	fprintf (ftmp,"|      | CARRIED FORWARD          ");
	fprintf (ftmp,"|        |             |             ");
	fprintf (ftmp,"|                |%12.2f |             |   ",
						DOLLARS (total [0]));
	fprintf (ftmp,"|        |%12.2f |             |\n",
						DOLLARS (total [1]));

	fprintf (ftmp,".EOF\n");

	/*  program exit sequence	*/

	pclose (ftmp);
}

/*========================== 
| Print journal headings . |
==========================*/
int
ReportHeading (void)
{
	strcpy (pageStart,glwkRec.sys_ref);
	pageNo 		= atol (pageStart);
	oldPageNo 	= pageNo ;
	headingFlag = 1;

	if ( (ftmp = popen ("pformat","w")) == 0)
	{
		sys_err ("Error in pformat During (POPEN)", 1, PNAME);
		shutdown_prog ();
        return (EXIT_FAILURE);
	}

	fprintf (ftmp,".START%s<%s>\n", DateToString (comm_rec.dbt_date), PNAME);
	fprintf (ftmp,".LP%d\n", printerNo);
	fprintf (ftmp,".PI12\n");
	fprintf (ftmp,".SO\n");
	fprintf (ftmp,".12\n");
	fprintf (ftmp,".L160\n");
	fprintf (ftmp,".PG%ld\n",pageNo);
	fprintf (ftmp,".B1\n");
	fprintf (ftmp,".EGENERAL LEDGER AUDIT (BATCH NO : %s)\n", glwkRec.run_no);
	fprintf (ftmp,".B1\n");
	fprintf (ftmp,".E%s AS AT: %s\n",clip (comm_rec.co_short), SystemTime ());
	fprintf (ftmp,".B1\n");

	fprintf (ftmp,".R==================================");
	fprintf (ftmp,"======================================");
	fprintf (ftmp,"================================================");
	fprintf (ftmp,"======================================\n");

	fprintf (ftmp,"==================================");
	fprintf (ftmp,"======================================");
	fprintf (ftmp,"================================================");
	fprintf (ftmp,"======================================\n");

	fprintf (ftmp,"|        C U S T O M E R S        ");
	fprintf (ftmp,"|         R E C E I P T              ");
	fprintf (ftmp,"|       G E N E R A L   L E D G E R          |G/L");
	fprintf (ftmp,"|A L L O C A T I O N    D E T A I L S|\n");

	fprintf (ftmp,"|NUMBER|         N A M E          ");
	fprintf (ftmp,"| NUMBER |  LOCAL AMT  | FGN  AMOUNT ");
	fprintf (ftmp,"| ACCOUNT NUMBER |  LOCAL AMT  | FGN  AMOUNT |PER");
	fprintf (ftmp,"| NUMBER |  LOCAL AMT  | FGN  AMOUNT |\n");

	fprintf (ftmp,"|------|--------------------------");
	fprintf (ftmp,"|--------|-------------|-------------");
	fprintf (ftmp,"|----------------|-------------|-------------|---");
	fprintf (ftmp,"|--------|-------------|-------------|\n");
    return (EXIT_SUCCESS);
}

void
ReadCustomer (void)
{
	strcpy (cumr_rec.co_no,glwkRec.co_no);
	strcpy (cumr_rec.est_no, branchNo);
	sprintf (cumr_rec.dbt_no, "%-6.6s", glwkRec.acronym);

	cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
	if (cc)
	{
		strcpy (cumr_rec.est_no, glwkRec.est_no);
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
	}
	if (cc)
		sprintf (customerName,"%30.30s", " ");
	else
		sprintf (customerName,"%-30.30s", cumr_rec.dbt_name);
}

void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

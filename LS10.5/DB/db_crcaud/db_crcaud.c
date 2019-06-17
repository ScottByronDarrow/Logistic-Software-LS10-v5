/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: db_crcaud.c,v 5.4 2001/10/09 08:37:59 robert Exp $
|  Program Name  : (db_crcaud.c)
|  Program Desc  : (Prints Receipts Audit Trail)
|---------------------------------------------------------------------|
|  Author        : Campbell Mander | Date Written  : 20/05/91         |
|---------------------------------------------------------------------|
| $Log: db_crcaud.c,v $
| Revision 5.4  2001/10/09 08:37:59  robert
| Updated to continue execution when RF_CLOSE failed
|
| Revision 5.3  2001/08/09 08:23:35  scott
| Added FinishProgram ();
|
| Revision 5.2  2001/08/06 23:21:52  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:17:05  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_crcaud.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_crcaud/db_crcaud.c,v 5.4 2001/10/09 08:37:59 robert Exp $";

#include	<pslscr.h>
#include	<GlUtils.h>
#include 	<ml_std_mess.h>
#include 	<ml_db_mess.h>

#include	"schema"

struct commRecord	comm_rec;
struct cumrRecord	cumr_rec;

	/*================================================================
	| Special fields and flags  ################################## . |
	================================================================*/
   	int		processID	= 0,
			glwk_no		= 0,
     		printerNo 	= 1,
    		headingFlag = 0,
			envDbCo 	= 0,
			envDbFind 	= 0;

	long 	pageNo		= 0L,
    		oldPageNo	= 0L;

   	char 	customerName [31],
			pageStart [sizeof glwkRec.sys_ref],
			currentBranch [3],
			branchNo [3];

	double	branchTotal 	= 0.00,
			companyTotal 	= 0.00;

   	FILE 	*ftmp;


/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void 	PageBreak 		(void);
int	 	OpenDB 			(void);
void 	CloseDB 		(void);
int 	PrintDetails 	(void);
void 	PrintTotals 	(void);
int 	ReportHeading 	(void);
void 	ReadCustomer 	(void);
void 	shutdown_prog 	(void);

int
main (
 int                argc,
 char*              argv [])
{
	int		noData 		= TRUE,
			firstTime 	= TRUE;

	if (argc < 3)
	{
		print_at (0,0, mlDbMess002, argv [0]);
        return (EXIT_FAILURE);
	}
	printerNo = atoi (argv [1]);
	processID   = atoi (argv [2]);

	envDbFind 	= atoi (get_env ("DB_FIND"));
	envDbCo 	= atoi (get_env ("DB_CO"));

	if (OpenDB ())
	{
		abc_dbclose ("data");
		shutdown_prog ();
        return (EXIT_FAILURE);
	}
	strcpy (branchNo, (envDbCo) ? comm_rec.est_no : " 0");

	init_scr ();
	print_mess (ML ("Printing Customers Receipts Audit Trail."));

	cc = FindGljc (comm_rec.co_no, " 6", "r");
	if (cc)
        return (EXIT_FAILURE);
	
	cc = RF_READ (glwk_no, (char *) &glwkRec);
	while (!cc)
	{
		if (strcmp (glwkRec.co_no,comm_rec.co_no))
			break;

		if (firstTime)
		{
			strcpy (currentBranch, glwkRec.est_no);
			firstTime = FALSE;
		}

		noData = FALSE;

		if (!strcmp (glwkRec.chq_inv_no,"               "))
		{
			cc = RF_READ (glwk_no, (char *) &glwkRec);
			continue;
		}
	
		if (strcmp (glwkRec.est_no, currentBranch))
		{		
			PageBreak ();
			strcpy (currentBranch, glwkRec.est_no);
		}
        if (PrintDetails () == 1)
        {
            return (EXIT_FAILURE);
        }

		cc = RF_READ (glwk_no, (char *) &glwkRec);
	}
	/*---------------------------------- 
	| Print  journal control records . |
	----------------------------------*/
	if (noData == FALSE)
		PrintTotals (); 

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

void
PageBreak (void)
{
	fprintf (ftmp,"|---------|------------------------------|");
	fprintf (ftmp,"--------|-------------|----|---------|--------------|\n");

	fprintf (ftmp,"| BRANCH TOTAL FOR BRANCH : %2.2s        |",
						currentBranch);
	fprintf (ftmp,"        |             |    |         |%14.2f|\n",
				 DOLLARS (branchTotal));

	fprintf (ftmp, ".PA\n");
	fflush (ftmp);

	branchTotal = 0.00;
}


/*======================
| Open Database Files. |
======================*/
int
OpenDB (void)
{
	char	filename [100];
	char	*sptr = getenv ("PROG_PATH");

	abc_dbopen ("data");

    read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	sprintf (filename,"%s/WORK/gl_work%05d", 
				 (sptr == (char *) 0) ? "/usr/LS10.5" : sptr, processID);
	cc = RF_OPEN (filename,sizeof (glwkRec),"r",&glwk_no);
	if (cc)
		return (cc);

	OpenGljc ();
	
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
	abc_fclose (comm);
	cc = RF_CLOSE (glwk_no);

	GL_Close ();
	abc_dbclose ("data");
}
int
PrintDetails (void)
{
	if (headingFlag != 1)
    {
		if (ReportHeading () == 1)
        {
            return (EXIT_FAILURE);
        }
    }

	ReadCustomer ();
	fprintf (ftmp,"| %-6.6s  ",glwkRec.acronym);
	fprintf (ftmp,"|%s",customerName);
	fprintf (ftmp,"|%-8.8s",glwkRec.chq_inv_no);
	fprintf (ftmp,"|%13.2f",DOLLARS (glwkRec.o1_amt));
	fprintf (ftmp,"|%3.3s ",glwkRec.currency);
	fprintf (ftmp,"|%9.4f",glwkRec.exch_rate);
	fprintf (ftmp,"|%14.2f|\n",DOLLARS (glwkRec.ci_amt));
	fflush (ftmp);

	branchTotal += (glwkRec.ci_amt);
	companyTotal += (glwkRec.ci_amt);

	return (EXIT_SUCCESS);
}

void
PrintTotals (void)
{
	fprintf (ftmp,"|---------|------------------------------|");
	fprintf (ftmp,"--------|-------------|----|---------|--------------|\n");

	fprintf (ftmp,"| BRANCH TOTAL FOR BRANCH : %2.2s           |",
						currentBranch);
	fprintf (ftmp,"        |             |    |         |%14.2f|\n", 
						DOLLARS (branchTotal));

	fprintf (ftmp,"|---------|------------------------------|");
	fprintf (ftmp,"--------|-------------|----|---------|--------------|\n");


	fprintf (ftmp,"| COMPANY TOTAL :                        |");
	fprintf (ftmp,"        |             |    |         |%14.2f|\n", 
						DOLLARS (companyTotal));

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

	fprintf (ftmp, ".START%s<%s>\n", DateToString (comm_rec.dbt_date), PNAME);
	fprintf (ftmp,".LP%d\n", printerNo);
	fprintf (ftmp,".SO\n");
	fprintf (ftmp,".13\n");
	fprintf (ftmp,".L150\n");
	fprintf (ftmp,".B1\n");
	fprintf (ftmp,".E%s AUDIT TRAIL (BATCH NO : %s)\n",clip (gljcRec.jnl_desc), glwkRec.run_no);
	fprintf (ftmp,".B1\n");
	fprintf (ftmp,".E%s AS AT: %s\n",clip (comm_rec.co_short), SystemTime ());
	fprintf (ftmp,".B1\n");

	fprintf (ftmp,".R==========================================");
	fprintf (ftmp,"=====================================================\n");

	fprintf (ftmp,"==========================================");
	fprintf (ftmp,"=====================================================\n");

	fprintf (ftmp,"|             D E B T O R S              |");
	fprintf (ftmp,"RECEIPT |    BASE     |CURR|EXCHANGE |    LOCAL     |\n");

	fprintf (ftmp,"| NUMBER  |            N A M E           |");
	fprintf (ftmp,"NUMBER  |   AMOUNT    |CODE|  RATE   |    AMOUNT    |\n");

	fprintf (ftmp,"|---------|------------------------------|");
	fprintf (ftmp,"--------|-------------|----|---------|--------------|\n");
	
	fprintf (ftmp,".PI10\n");
    return (EXIT_SUCCESS);
}

void
ReadCustomer (void)
{
	strcpy (cumr_rec.co_no,glwkRec.co_no);
	strcpy (cumr_rec.est_no, branchNo);
	strcpy (cumr_rec.dbt_no,mid (glwkRec.acronym,1,6));

	cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
	if (cc)
	{
		strcpy (cumr_rec.est_no, glwkRec.est_no);
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
	}
	if (cc)
		sprintf (customerName,"%30.30s", " ");
	else
		sprintf (customerName, "%-30.30s", cumr_rec.dbt_name);
}

void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

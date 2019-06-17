/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: db_bill_aud.c,v 5.1 2001/12/07 03:27:54 scott Exp $
|  Program Name  : (db_bill_aud.c)
|  Program Desc  : (Bill Reversal Audit)
|---------------------------------------------------------------------|
|  Author        : Campbell Mander | Date Written  : 30/11/92         |
|---------------------------------------------------------------------|
| $Log: db_bill_aud.c,v $
| Revision 5.1  2001/12/07 03:27:54  scott
| Updated to convert to app.schema and perform a general code clean.
|
| Revision 5.2  2001/08/09 08:22:27  scott
| Added FinishProgram ();
|
| Revision 5.1  2001/08/06 23:21:40  scott
| RELEASE 5.0
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_bill_aud.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_bill_aud/db_bill_aud.c,v 5.1 2001/12/07 03:27:54 scott Exp $";

#include	<pslscr.h>
#include	<GlUtils.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<ml_db_mess.h>

	/*================================================================
	| Special fields and flags  ################################## . |
	================================================================*/
   	int		processID	=	0,
			glwkNo		=	0,		
     		printerNo 	=	1,
    		headFlag1 	= 	0,
			envDbCo 	= 	0,
			envDbFind 	= 	0,
			first_time 	= 	TRUE,
			BillsDue	= 	FALSE,
			envDbMcurr 	= 	FALSE;

	long 	pageNo		=	0L,
    		oldPageNo	=	0L;

   	char 	startPage 		[sizeof glwkRec.sys_ref],
			branchNo 		[3],
			previousReceipt [16];

   	double 	totalAmount [5]	=	{0.00,0.00,0.00,0.00,0.00},
			creditTotal 	= 	0.00,
			debitTotal 		= 	0.00;

   	FILE 	*ftmp;

#include	"schema"

struct commRecord	comm_rec;
struct cumrRecord	cumr_rec;

	char	*data = "data";

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
int		OpenDB 				(void);
void 	CloseDB 			(void);
void 	PrintTotals 		(void);
void 	shutdown_prog 		(void);
int 	PrintDetails 		(void);
int 	ReportHeading		(void);

int
main (
 int                argc,
 char*              argv [])
{
	int	no_data = TRUE;
	char	*sptr;

	if (argc < 3)
	{
		/*-------------------
		| Usage %s LPNO PID |
		-------------------*/
		print_at (0,0, mlDbMess002, argv [0]);
        return (EXIT_FAILURE);
	}
	printerNo 	= atoi (argv [1]);
	processID   = atoi (argv [2]);

	envDbFind 	= atoi (get_env ("DB_FIND"));
	envDbCo 	= atoi (get_env ("DB_CO"));
	strcpy (branchNo, (envDbCo) ? comm_rec.est_no : " 0");
	sptr = chk_env ("DB_MCURR");
	envDbMcurr = (sptr == (char *)0) ? FALSE : atoi (sptr);

	/*---------------------
	| Check program name. |
	---------------------*/
	sptr = strrchr (argv [0], '/');
	if (sptr)
		sptr++;
	else
		sptr = argv [0];

	BillsDue = FALSE;
	if (!strcmp (sptr, "db_bill_aud"))
		BillsDue = TRUE;

	if (OpenDB ())
        return (EXIT_FAILURE);

	cc = FindGljc (comm_rec.co_no, (BillsDue) ? "20" : "25", "r");
	if (cc)
	{
		CloseDB (); 
		FinishProgram ();
        return (EXIT_FAILURE);
	}
 
	if (BillsDue)
	{
		dsp_screen ("Printing Bills Reversed Audit.", comm_rec.co_no,
													 comm_rec.co_name);
	}
	else
	{
		dsp_screen ("Printing Forward Cheques Audit.", comm_rec.co_no,
													  comm_rec.co_name);
	}
	strcpy (previousReceipt, "");
	cc = RF_READ (glwkNo, (char *) &glwkRec);
	while (!cc)
	{
		no_data = FALSE;
		dsp_process ("Account : ",glwkRec.acc_no);
		if (PrintDetails () == 1)
        {
            return (EXIT_FAILURE);
        }
		cc = RF_READ (glwkNo, (char *) &glwkRec);
	}
	/*---------------------------------- 
	| Print  journal control records . |
	----------------------------------*/
	if (no_data == FALSE && headFlag1)
		PrintTotals (); 

	CloseDB (); 
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*======================
| Open Database Files. |
======================*/
int
OpenDB (void)
{
	char	filename [150];
	char	*sptr = getenv ("PROG_PATH");

    sprintf (filename,"%s/WORK/gl_work%05d", 
				 (sptr == (char *) 0) ? "/usr/LS10.5" : sptr, processID);

	cc = RF_OPEN (filename,sizeof (glwkRec),"r",&glwkNo);
	if (cc) 
		return (EXIT_FAILURE);            

	abc_dbopen (data);

    read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, (!envDbFind) ? "cumr_id_no"
													     	: "cumr_id_no3");
	OpenGljc ();
	OpenGlmr ();

    return (EXIT_SUCCESS);
}

/*=======================
| Close Database Files. |
=======================*/
void
CloseDB (void)
{
	abc_fclose (cumr);

	cc = RF_CLOSE (glwkNo);
	if (cc) 
		file_err (cc, "glwkNo", "WKCLOSE");

	GL_Close ();
	abc_dbclose (data);
}

/*--------------------
| Print detail line. |
--------------------*/
int
PrintDetails (void)
{
	int		type;

	if (strcmp (glwkRec.co_no, comm_rec.co_no))
		return (EXIT_SUCCESS);

	if (headFlag1 != 1)
    {
		if (ReportHeading () == 1)
        {   /* error */
            return (EXIT_FAILURE);            
        }
    }

	strcpy (startPage,glwkRec.sys_ref);
	pageNo 	= atol (startPage);
	type 	= atoi (glwkRec.jnl_type);
	if (pageNo != oldPageNo)
	{
		fprintf (ftmp,".PG%ld\n",pageNo);
		oldPageNo = pageNo;
	}

	strcpy (glmrRec.co_no, comm_rec.co_no);
	sprintf (glmrRec.acc_no, "%-*.*s", 
							MAXLEVEL, MAXLEVEL, glwkRec.acc_no);
	cc = find_rec (glmr, &glmrRec, COMPARISON, "r");
	if (cc)
		file_err (cc, glmr, "DBFIND");

	if (strcmp (previousReceipt, glwkRec.chq_inv_no))
	{
		if (!first_time)
		{
			fprintf (ftmp, "|          ");
			fprintf (ftmp, "|          ");
			if (envDbMcurr)
			{
				fprintf (ftmp, "|        ");
				fprintf (ftmp, "|              ");
				fprintf (ftmp, "|           ");
			}
			fprintf (ftmp, "|                ");
			fprintf (ftmp, "|                           ");
			fprintf (ftmp, "|              ");
			fprintf (ftmp, "|              ");
			fprintf (ftmp, "|   |\n");
		}

		fprintf (ftmp,"|  %-6.6s  ",   glwkRec.chq_inv_no);
		fprintf (ftmp,"|%-10.10s",       DateToString (glwkRec.tran_date));
		if (envDbMcurr)
		{
			fprintf (ftmp,"|  %-3.3s   ",  glwkRec.name);
			fprintf (ftmp,"| %12.2f ",     DOLLARS (glwkRec.o1_amt));
			fprintf (ftmp,"| %9.4f ",      glwkRec.o2_amt);
		}

		strcpy (previousReceipt, glwkRec.chq_inv_no);
	}
	else
	{
		fprintf (ftmp,"|  %-6.6s  ",   " ");
		fprintf (ftmp,"|%-10.10s",       " ");
		if (envDbMcurr)
		{
			fprintf (ftmp,"|  %-3.3s   ",  " ");
			fprintf (ftmp,"| %12.12s ",    " ");
			fprintf (ftmp,"| %9.9s ",      " ");
		}
	}

	fprintf (ftmp,"|%-16.16s", glwkRec.acc_no);
	fprintf (ftmp,"| %-25.25s ",   glmrRec.desc);
	if (type % 2)
	{
		fprintf (ftmp,"| %12.2f ",DOLLARS (glwkRec.amount));
		fprintf (ftmp,"|              ");
		debitTotal += glwkRec.amount;
	}
	else
	{
		fprintf (ftmp,"|              ");
		fprintf (ftmp,"| %12.2f ",DOLLARS (glwkRec.amount));
		creditTotal += glwkRec.amount;
	}
	fprintf (ftmp,
	       "| %02d|\n", 
	       mth2fisc (atoi (glwkRec.period_no),comm_rec.fiscal));

	first_time = FALSE;
    return (EXIT_SUCCESS);
}

/*---------------
| Print totals. |
---------------*/
void
PrintTotals (void)
{
	double	total [2];

	total [0] = total [1] = 0.00;

	totalAmount [3]	=	gljcRec.tot_2 + 
				 		gljcRec.tot_4 + 
				 		gljcRec.tot_6 - 
				 		creditTotal;

	totalAmount [0] = 	gljcRec.tot_1 + 
				 		gljcRec.tot_3 + 
				 		gljcRec.tot_5 - 
				 		debitTotal;

	fprintf (ftmp, "|----------");	
	fprintf (ftmp, "-----------");	
	if (envDbMcurr)
	{
		fprintf (ftmp, "---------");
		fprintf (ftmp, "---------------");
		fprintf (ftmp, "------------");
	}
	fprintf (ftmp, "-----------------");
	fprintf (ftmp, "----------------------------");
	fprintf (ftmp, "---------------");
	fprintf (ftmp, "---------------");
	fprintf (ftmp, "----|\n");

	fprintf (ftmp,"|");
	if (envDbMcurr)
		fprintf (ftmp,"                                    ");
	fprintf (ftmp,"                           ");
	fprintf (ftmp,"BROUGHT FORWARD                        |");
	fprintf (ftmp, "%13.2f |%13.2f |", 
				DOLLARS (totalAmount [0]), DOLLARS (totalAmount [3]));
	fprintf (ftmp,"   |\n");

	fprintf (ftmp,"|");
	if (envDbMcurr)
		fprintf (ftmp,"                                      ");
	fprintf (ftmp,"                         ");
	fprintf (ftmp,"THIS RUN                               |");
	fprintf (ftmp,"%13.2f |%13.2f |",DOLLARS (debitTotal),DOLLARS (creditTotal));
	fprintf (ftmp,"   |\n");

	total [0] = totalAmount [0] + debitTotal;
	total [1] = totalAmount [3] + creditTotal;
	fprintf (ftmp,"|");
	if (envDbMcurr)
		fprintf (ftmp,"                                    ");
	fprintf (ftmp,"                           ");
	fprintf (ftmp,"CARRIED FORWARD                        |");
	fprintf (ftmp,"%13.2f |%13.2f |",DOLLARS (total [0]),DOLLARS (total [1]));
	fprintf (ftmp,"   |\n");

        fprintf (ftmp,".EOF\n");

	pclose (ftmp);
}

/*========================== 
| Print journal headings . |
==========================*/
int
ReportHeading (void)
{
	strcpy (startPage,glwkRec.sys_ref);
	pageNo = atol (startPage);
	oldPageNo = pageNo ;
	headFlag1 = 1;

	if ( (ftmp = popen ("pformat","w")) == 0)
	{
		sys_err ("Error in pformat During (POPEN)", 1, PNAME);
		shutdown_prog ();
        return (EXIT_FAILURE);
	}

	fprintf (ftmp,".START%s<%s>\n", DateToString (comm_rec.dbt_date), PNAME);
	fprintf (ftmp,".LP%d\n", printerNo);
	fprintf (ftmp,".SO\n");
	fprintf (ftmp,".13\n");
	fprintf (ftmp,".L158\n");
	fprintf (ftmp,".PG%ld\n",pageNo);
	fprintf (ftmp,".B1\n");
	if (BillsDue)
		fprintf (ftmp,".EBILL REVERSAL AUDIT (RUN NO : %-6.6s)\n", glwkRec.run_no);
	else
		fprintf (ftmp,".EFORWARD DUE CHEQUES (RUN NO : %-6.6s)\n", glwkRec.run_no);
	fprintf (ftmp,".B1\n");
	fprintf (ftmp,".E%s AS AT: %s\n",clip (comm_rec.co_short), SystemTime ());
	fprintf (ftmp,".B1\n");

	fprintf (ftmp, ".R=============");	
	fprintf (ftmp, "==========");	
	if (envDbMcurr)
	{
		fprintf (ftmp, "==========");
		fprintf (ftmp, "==============");
		fprintf (ftmp, "============");
	}
	fprintf (ftmp, "================");
	fprintf (ftmp, "============================");
	fprintf (ftmp, "===============");
	fprintf (ftmp, "===============");
	fprintf (ftmp, "=====\n");

	fprintf (ftmp, "=============");	
	fprintf (ftmp, "==========");	
	if (envDbMcurr)
	{
		fprintf (ftmp, "==========");
		fprintf (ftmp, "==============");
		fprintf (ftmp, "============");
	}
	fprintf (ftmp, "================");
	fprintf (ftmp, "============================");
	fprintf (ftmp, "===============");
	fprintf (ftmp, "===============");
	fprintf (ftmp, "=====\n");

	fprintf (ftmp, "|RECEIPT NO");	
	fprintf (ftmp, "| DUE DATE ");	
	if (envDbMcurr)
	{
		fprintf (ftmp, "|CURRENCY");
		fprintf (ftmp, "|FOREIGN AMOUNT");
		fprintf (ftmp, "| EXCH RATE ");
	}
	fprintf (ftmp, "|G/L ACCOUNT NO  ");
	fprintf (ftmp, "|    ACCOUNT DESCRIPTION    ");
	fprintf (ftmp, "|    DEBIT     ");
	fprintf (ftmp, "|    CREDIT    ");
	fprintf (ftmp, "|PER|\n");

	fprintf (ftmp, "|----------");	
	fprintf (ftmp, "|----------");	
	if (envDbMcurr)
	{
		fprintf (ftmp, "|--------");
		fprintf (ftmp, "|--------------");
		fprintf (ftmp, "|-----------");
	}
	fprintf (ftmp, "|----------------");
	fprintf (ftmp, "|---------------------------");
	fprintf (ftmp, "|--------------");
	fprintf (ftmp, "|--------------");
	fprintf (ftmp, "|---|\n");

	fprintf (ftmp,".PI12\n");
    return (EXIT_SUCCESS);
}

void
shutdown_prog ()
{
	FinishProgram ();
}

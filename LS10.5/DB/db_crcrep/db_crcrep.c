/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: db_crcrep.c,v 5.3 2001/08/09 08:23:40 scott Exp $
|  Program Name  : (db_crcrep.c) 
|  Program Desc  : (Prints Receipts Journal and Receipt Allocation)
|                 (Report)
|---------------------------------------------------------------------|
|  Author        : Anneliese Allen.| Date Written  : 18/11/92         |
|---------------------------------------------------------------------|
| $Log: db_crcrep.c,v $
| Revision 5.3  2001/08/09 08:23:40  scott
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
char	*PNAME = "$RCSfile: db_crcrep.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_crcrep/db_crcrep.c,v 5.3 2001/08/09 08:23:40 scott Exp $";

#include	<pslscr.h>
#include	<GlUtils.h>
#include 	<twodec.h>

#define		CF(x)	 (comma_fmt (DOLLARS (x), "N,NNN,NNN,NNN.NN"))
#define		CF2(y)	 (comma_fmt (DOLLARS (y), "NNN,NNN,NNN.NN"))

	/*----------------------------------------------------------------
	| Special fields and flags  ################################## . |
	----------------------------------------------------------------*/
   	int		pid,		
			db_crc,		
			cudr_no,
			glwk_no,
			nextLine,
     		lp_no = 1,
    		headingFlag1 = 0,
    		headingFlag2 = 0;

	FILE	*fsort,
			*ftmp;

#include	"schema"

struct commRecord	comm_rec;
struct cumrRecord	cumr_rec;

	/*==============================================
	| Creditors/Customer Receipts Work File Record. |
	==============================================*/
	struct CUDR_REC
	{
		char	co_no [3];		/* Company number.					*/
		char	br_no [3];		/* Branch Number.					*/
		char	rec_no [9];		/* Receipt number.					*/
		char	reversal [2];	/* Reversal flag					*/
		char	period [3];		/* Period No						*/
		long	hhcp_hash;
		char	bank_id [6];
		char	bank_name [41];
		char	bk_ccode [4];
		char	bk_cdesc [41];
		double	bk_exch;		/* Origin to bank exchange */
		double	bk_rec_amt;		/* Bank receipt amount */
		double	bk_charge;		/* Bank charges */
		double	bk_clear_fee;	/* Bank clearance fee */
		double	bk_l_exch;		/* Bank to local exch rate */
		char	dbt_no [7];		/* Customer Number */
		char	dbt_name [41];	/* Customer Name */
		long	hhcu_hash;		/* Customer Hash */
		long	rec_date;		/* Receipt Date */
		char	rec_type [2];	/* Receipt Type */
		long	due_date;		/* Due Date For Bank Drafts */
		char	narrative [21];	/* Bank Draft Reference */
		char	invoice [9];	/* Invoice being paid */
		double	inv_exch;		/* Invoice Exchange Rate */
		double	inv_amt;		/* Invoice Amount */
		char	o_curr [4];		/* Origin Currenty. */
		double	o_exch;			/* Origin to local exchange */
		Money	o_disc;			
		Money	o_total_amt;	
		Money	o_amt_pay;	
		char	gl_disc [MAXLEVEL + 1];	/* GL discount account */
		Money	l_disc;	
		Money	l_total_amt;
		Money	l_amt_pay;		
	} cudr_rec, cudr2_rec;

	int		envDbCo 	= 0,
			envDbFind 	= 0,
			first_time 	= TRUE,
			envDbMcurr 	= FALSE,
			BK_ID	   	=	0,
			BR_NO	   	=	5,
			DBT_NO	   	=	7,
			REC_NO	   	=	13,
			INVOICE	   	=	22,
			DBT_SRT	   	=	31,
			CO_NO	   	=	35,
			BK_NAME	   	=	37,
			BK_CCODE   	=	77,
			BK_EXCH	   	=	80,
			BK_REC_AMT 	=	90,
			BK_CHRG	   	=	103,
			BK_L_EXCH  	=	112,
			INV_EXCH   	=	122,
			INV_AMT	   	=	132,
			O_EXCH	   	=	145,
			O_DISC	   	=	155,
			O_AMT	   	=	168,
			O_AMT_PD   	=	181,
			L_DISC	   	=	194,
			L_TOT_AMT  	=	207,
			L_AMT_PD   	=	220;

	double	totPayAmt		 = 0.00,
			totInvAmt		 = 0.00,
			totExchVar		 = 0.00,
			sub_exch_var 	 = 0.00,
			bank_charges 	 = 0.00,
			gross_rcpt_amt   = 0.00,
			tot_amt_gross 	 = 0.00,
			tot_bk_chrg 	 = 0.00,
			tot_dbt_rcpt_amt = 0.00,
			tot_gr_rcpt_amt  = 0.00,
			tot_lcl_bk_chrg  = 0.00;

	char	*sptr,
			*lcl_ptr,
			runNumber [sizeof glwkRec.run_no],
			previousData [256],	
			batchNo [sizeof	glwkRec.sys_ref],	
			printReport [2],	
			currentBank [6],
			currentCustomer [7],
			currentReceipt [9],
			previousBank [6],
			previousCustomer [7],
			previousReceipt [9],
			prog_path [100];

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void 	CloseDB 		(void);
void 	ReadCudr 		(void);
void 	PrintTotals 	(void);
void 	PrintTotals2 	(void);
void 	shutdown_prog 	(void);
int 	OpenDB 			(void);
int 	PrintHeading 	(void);
int 	PrintDetails 	(char *);
int 	PrintDetails2 	(void);
int 	PrintHeading2 	(void);

int
main (
	int		argc,
	char	*argv [])
{
	int	no_data = TRUE;

	init_scr ();
	set_tty ();
	if (argc < 5)
	{
		print_at (0,0,"Usage:  %s <lpno> <pid> print 2nd Rep [Y/N]", argv [0]);
        return (EXIT_FAILURE);
	}
	lp_no = atoi (argv [1]);
	pid   = atoi (argv [2]);
    printReport [0] = argv [3][0];
	sprintf (batchNo, "%-10.10s", argv [4]);

	envDbFind  = atoi (get_env ("DB_FIND"));
	envDbCo = atoi (get_env ("DB_CO"));

	sptr = getenv ("PROG_PATH");
	strcpy (prog_path, (sptr) ? sptr : "/usr/LS10.5");

	if (OpenDB ())
        return (EXIT_FAILURE);
 
	sptr = chk_env ("DB_MCURR");
	envDbMcurr = (sptr == (char *)0) ? FALSE : atoi (sptr);
	
	print_mess (ML ("Printing Customer Receipts Journals and Allocation Report."));
	fsort = sort_open ("crcjnl");

	ReadCudr ();
	
	sptr = sort_read (fsort);
	while (sptr)
	{
		no_data = FALSE;
		if (PrintDetails (sptr) == 1)
        {
			shutdown_prog ();
            return (EXIT_FAILURE);
        }
		sptr = sort_read (fsort);
	}
	/*---------------------------------- 
	| Print  journal control records . |
	----------------------------------*/
	if (no_data == FALSE)
		PrintTotals (); 

	if (argv [3][0] == 'N')
    {
		shutdown_prog ();
        return (EXIT_SUCCESS);
    }

	/*---------------------- 
	| Print second report. |
	----------------------*/
	sort_rewind (fsort);

	no_data = TRUE;
	first_time = TRUE;

	strcpy (previousData, "");
	sptr = sort_read (fsort);
	while (sptr)
	{
		* (sptr + strlen (sptr) - 1) = '\0';
		no_data = FALSE;
		if (PrintDetails2 () == 1)
        {
			shutdown_prog ();
            return (EXIT_FAILURE); 
        }
		strcpy (previousData, sptr);
		sptr = sort_read (fsort);
	}

	if (no_data == FALSE)
		PrintTotals2 (); 

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

	sprintf (filename,"%s/WORK/db_crc%05d", prog_path, pid);

	cc = RF_OPEN (filename,sizeof (cudr_rec),"r",&cudr_no);
	if (cc) 
        return (EXIT_FAILURE);

	abc_dbopen ("data");

    read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

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
	cc = RF_CLOSE (cudr_no);
	if (cc) 
		file_err (cc, "db_crc", "WKCLOSE");

	abc_dbclose ("data");
}

/*============================================
| Get records from creditors/dbtrs work file.|
============================================*/
void
ReadCudr (void)
{
	int		dbt_seq_no = 0;
	char	dbt_srt [6];
    char    sort_temp [300];
	
	cc = RF_READ (cudr_no, (char *) &cudr_rec);
	while (!cc)
	{
		sprintf (dbt_srt, "%04d", dbt_seq_no++);
        sprintf (sort_temp,
                 "%-5.5s%-2.2s%-6.6s%8.8s %-8.8s %-4.4s%-2.2s%-40.40s%-3.3s%9.4f %12.2f %8.2f %9.4f %9.4f %12.2f %9.4f %12.2f %-12.2f %12.2f %12.2f %12.2f %12.2f\n",
                 cudr_rec.bank_id,
                 cudr_rec.br_no,
                 cudr_rec.dbt_no,
                 cudr_rec.rec_no,
                 cudr_rec.invoice,
                 dbt_srt,
                 cudr_rec.co_no,
                 cudr_rec.bank_name,
                 cudr_rec.bk_ccode,
                 cudr_rec.bk_exch,
                 cudr_rec.bk_rec_amt,
                 cudr_rec.bk_charge,
                 cudr_rec.bk_l_exch,
                 cudr_rec.inv_exch,
                 cudr_rec.inv_amt,
                 cudr_rec.o_exch,
                 cudr_rec.o_disc,
                 cudr_rec.o_total_amt,
                 cudr_rec.o_amt_pay,
                 cudr_rec.l_disc,
                 cudr_rec.l_total_amt,
                 cudr_rec.l_amt_pay);

		sort_save (fsort, sort_temp);

		cc = RF_READ (cudr_no, (char *) &cudr_rec);
	}
	fsort = sort_sort (fsort, "crcjnl");
}

int
PrintDetails (
	char	*printLine)
{
	char	curr_code [4],
			bank_name [41];

	double	fgnAmount	=	0.00,
			locExchRate =	0.00,
			fgnExchRate	=	0.00,
			locAmount	=	0.00,
			tempAmount	=	0.00,
			locInvAmt	=	0.00,
			fgnPayAmt 	=  	0.00,
			locExchVar	=	0.00;

	
	if (strcmp (cudr_rec.co_no,comm_rec.co_no))
		return (EXIT_FAILURE);

	if (headingFlag1 != 1)
    {
		if (PrintHeading () == 1)
        {
            return (EXIT_FAILURE);
        }
    }

	sprintf (currentCustomer,	"%-6.6s",   printLine + DBT_NO);
	sprintf (currentBank,   	"%-5.5s",   printLine + BK_ID);
	sprintf (bank_name, 		"%-40.40s", printLine + BK_NAME);
	sprintf (curr_code, 		"%-3.3s",   printLine + BK_CCODE);
	sprintf (currentReceipt, 	"%-8.8s",   printLine + REC_NO);

	if (first_time)
	{
		sprintf (previousCustomer,  "%-6.6s", " ");
		sprintf (previousBank,   	"%-5.5s", " ");
		sprintf (previousReceipt, 	"%-8.8s", " ");
	}

	sprintf (cumr_rec.co_no,  "%-2.2s", printLine + CO_NO);
	sprintf (cumr_rec.est_no, "%-2.2s", printLine + BR_NO);
	sprintf (cumr_rec.dbt_no, "%-6.6s", printLine + DBT_NO);

	cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
	if (cc)
	{
		strcpy (cumr_rec.curr_code, " ");
		strcpy (cumr_rec.dbt_name, " ");
	}

	if (strcmp (previousReceipt, currentReceipt) && !first_time)
	{	
		if (envDbMcurr)
		{
			fprintf (ftmp, "|        ");
			fprintf (ftmp, "|         ");
			fprintf (ftmp, "|                ");
			fprintf (ftmp, "|     ");
			fprintf (ftmp, "|           ");
			fprintf (ftmp, "|                ");
			fprintf (ftmp, "|                ");
			fprintf (ftmp, "|           ");
			fprintf (ftmp, "|                ");
			fprintf (ftmp, "|                |\n");
		}
		else
		{
			fprintf (ftmp, "|             ");
			fprintf (ftmp, "|          ");
			fprintf (ftmp, "|                ");
			fprintf (ftmp, "|                |\n");
		}

		sub_exch_var = 0.00;
	}

	if (strcmp (previousBank, currentBank))
	{
		fprintf (ftmp,".DS6\n");
		fprintf (ftmp,".EBANK: %s", clip (bank_name));
		if (envDbMcurr)
			fprintf (ftmp,"      CURRENCY: %s\n",clip (curr_code));
		else
			fprintf (ftmp, "\n");

		fprintf (ftmp,".B1\n");

		if (envDbMcurr)
		{
			fprintf (ftmp, "==========");
			fprintf (ftmp, "==========");
			fprintf (ftmp, "=================");
			fprintf (ftmp, "======");
			fprintf (ftmp, "============");
			fprintf (ftmp, "=================");
			fprintf (ftmp, "=================");
			fprintf (ftmp, "============");
			fprintf (ftmp, "=================");
			fprintf (ftmp, "=================\n");

			fprintf (ftmp,"| RECEPT ");
			fprintf (ftmp,"|                   PAYMENT DETAILS   ");
			fprintf (ftmp,"                        ");
			fprintf (ftmp,"|            RECEIPT DETAILS                  ");
			fprintf (ftmp,"|    VARIANCE    |\n");

			fprintf (ftmp,"| NUMBER ");
			fprintf (ftmp,"| DOCUMNT ");
			fprintf (ftmp,"| AMOUNT ORIGIN  ");
			fprintf (ftmp,"| CUR ");
			fprintf (ftmp,"| EXCH RATE ");
			fprintf (ftmp,"|INV AMOUNT LOCAL");
			fprintf (ftmp,"| PAY AMT ORIGIN ");
			fprintf (ftmp,"| EXCH RATE ");
			fprintf (ftmp,"|PAY AMOUNT LOCAL");
			fprintf (ftmp,"| EXCH VAR LOCAL |\n");

			fprintf (ftmp,"|--------");
			fprintf (ftmp,"|---------");
			fprintf (ftmp,"|----------------");
			fprintf (ftmp,"|-----");
			fprintf (ftmp,"|-----------");
			fprintf (ftmp,"|----------------");
			fprintf (ftmp,"|----------------");
			fprintf (ftmp,"|-----------");
			fprintf (ftmp,"|----------------");
			fprintf (ftmp,"|----------------|\n");
		}
		else
		{
			fprintf (ftmp,"==============");
			fprintf (ftmp,"===========");
			fprintf (ftmp,"=================");
			fprintf (ftmp,"==================\n");

			fprintf (ftmp,"| RECEIPT NO. ");
			fprintf (ftmp,"| DOCUMENT ");
			fprintf (ftmp,"| INVOICE AMOUNT ");
			fprintf (ftmp,"| RECEIPT AMOUNT |\n");

			fprintf (ftmp,"|-------------");
			fprintf (ftmp,"|----------");
			fprintf (ftmp,"|----------------");
			fprintf (ftmp,"|----------------|\n");
		}
	
		if (!first_time)
			fprintf (ftmp,".PA\n");
	}
	
	if (strcmp (previousCustomer, currentCustomer))
	{
		if (!first_time && !strcmp (previousBank, currentBank))
		{
			if (envDbMcurr)
			{
				fprintf (ftmp,"|--------");
				fprintf (ftmp,"|---------");
				fprintf (ftmp,"|----------------");
				fprintf (ftmp,"|-----");
				fprintf (ftmp,"|-----------");
				fprintf (ftmp,"|----------------");
				fprintf (ftmp,"|----------------");
				fprintf (ftmp,"|-----------");
				fprintf (ftmp,"|----------------");
				fprintf (ftmp,"|-----------------|\n");
			}	
			else
			{
				fprintf (ftmp,"|-------------");
				fprintf (ftmp,"|----------");
				fprintf (ftmp,"|----------------");
				fprintf (ftmp,"|----------------|\n");
			}	
		}

		fprintf (ftmp,"| %-6.6s", currentCustomer);
		fprintf (ftmp,"  %-40.40s ", cumr_rec.dbt_name);
		if (envDbMcurr)
			fprintf (ftmp,"%-83.83s|\n", " ");
		else
			fprintf (ftmp,"%-8.8s|\n", " ");

		strcpy (previousCustomer, currentCustomer);
		first_time = FALSE;
	}
		
	if (strcmp (currentReceipt, previousReceipt))
	{
		if (envDbMcurr)
			fprintf (ftmp,"|%-8.8s", printLine + REC_NO);
		else
			fprintf (ftmp,"|  %-8.8s   ", printLine + REC_NO);

		strcpy (previousReceipt, currentReceipt);
	}
	else
	{
		if (envDbMcurr)
			fprintf (ftmp,"| %-6.6s ", " ");
		else
			fprintf (ftmp,"|%-13.13s", " ");
	}

	fgnAmount	= atof (printLine + INV_AMT);
	locExchRate	= atof (printLine + INV_EXCH);
	fgnExchRate	= atof (printLine + O_EXCH);
	locAmount	= atof (printLine + L_AMT_PD);
	locInvAmt 	= no_dec (fgnAmount / locExchRate);
	fgnPayAmt 	= atof (printLine + O_AMT_PD);

	/*--------------------------
	| Calculate exch variance. |
	--------------------------*/
	tempAmount = fgnPayAmt;
	if (locExchRate != 0.00)
		tempAmount /= locExchRate;
	tempAmount = no_dec (tempAmount);
	locExchVar = tempAmount - locAmount ;

	/*------------------------
	| Reversal of a deposit. |
	------------------------*/
	if (fgnAmount == 0.00)
		locExchVar = 0.00;

	sub_exch_var += locExchVar;

	if (envDbMcurr)
	{
		fprintf (ftmp,"| %-8.8s", printLine + INVOICE);
		fprintf (ftmp,"|%s",	   CF (fgnAmount));
		fprintf (ftmp,"| %3.3s ",   cumr_rec.curr_code);
		fprintf (ftmp,"| %9.4f ",   locExchRate);
		fprintf (ftmp,"|%s",        CF (locInvAmt));
		fprintf (ftmp,"|%s",        CF (fgnPayAmt));
		fprintf (ftmp,"| %9.4f ",   fgnExchRate);
		fprintf (ftmp,"|%s",        CF (locAmount));
		fprintf (ftmp,"|%s|\n",     CF (locExchVar));

	}
	else
	{
		fprintf (ftmp,"| %-8.8s ", printLine + INVOICE);
		fprintf (ftmp,"|%s",         CF (locInvAmt));
		fprintf (ftmp,"|%s|\n",      CF (locAmount));
	}

	totInvAmt += locInvAmt;
	totPayAmt += locAmount;
	totExchVar += locExchVar;

	strcpy (previousBank, currentBank);
    return (EXIT_SUCCESS);
}

void
PrintTotals (void)
{
	if (envDbMcurr)
	{
		fprintf (ftmp,"|--------");
		fprintf (ftmp,"|---------");
		fprintf (ftmp,"|----------------");
		fprintf (ftmp,"|-----");
		fprintf (ftmp,"|-----------");
		fprintf (ftmp,"|----------------");
		fprintf (ftmp,"|----------------");
		fprintf (ftmp,"|-----------");
		fprintf (ftmp,"|----------------");
		fprintf (ftmp,"|----------------|\n");
	}
	else
	{
		fprintf (ftmp,"|-------------");
		fprintf (ftmp,"|----------");
		fprintf (ftmp,"|----------------");
		fprintf (ftmp,"|----------------|\n");
	}

	if (envDbMcurr)
	{
		fprintf (ftmp,"|                   Totals:                           |");
		fprintf (ftmp,"%s|                            ",CF (totInvAmt));
		fprintf (ftmp,"|%s",    CF (totPayAmt));
		fprintf (ftmp,"|%s|\n", CF (totExchVar));
	}
	else
	{
		fprintf (ftmp,"|   Totals:              |");
		fprintf (ftmp,"%s|",   CF (totInvAmt));
		fprintf (ftmp,"%s|\n", CF (totPayAmt));
	}
    fprintf (ftmp, ".EOF\n");

	totInvAmt = totPayAmt = totExchVar = 0.00;

	pclose (ftmp);
}

/*========================== 
| Print journal headings . |
==========================*/
int
PrintHeading (void)
{
	headingFlag1 = TRUE;

	/*---------------------------
	| Get run number from glwk. |
	---------------------------*/
	if ( (ftmp = popen ("pformat","w")) == 0)
	{
		sys_err ("Error in pformat During (POPEN)", 1, PNAME);
		shutdown_prog ();
        return (EXIT_FAILURE);
	}

	fprintf (ftmp,".START%s<%s>\n", DateToString (comm_rec.dbt_date), PNAME);
	fprintf (ftmp,".LP%d\n", lp_no);
	fprintf (ftmp,".SO\n");
	fprintf (ftmp,".7\n");
	fprintf (ftmp,".L%d\n", (envDbMcurr) ? 135 : 96);
	fprintf (ftmp,".B1\n");
	fprintf (ftmp,".ERECEIPT ALLOCATION REPORT (BATCH NO : %s)\n", batchNo); 
	fprintf (ftmp,".B1\n");
	fprintf (ftmp,".E%s AS AT: %s\n",clip (comm_rec.co_short), SystemTime ());
	fprintf (ftmp,".PI12\n");

	if (envDbMcurr)
	{
		fprintf (ftmp, ".R==========");
		fprintf (ftmp, "==========");
		fprintf (ftmp, "=================");
		fprintf (ftmp, "======");
		fprintf (ftmp, "============");
		fprintf (ftmp, "=================");
		fprintf (ftmp, "=================");
		fprintf (ftmp, "============");
		fprintf (ftmp, "=================");
		fprintf (ftmp, "=================\n");
	}
	else
	{
		fprintf (ftmp,".R==============");
		fprintf (ftmp,"===========");
		fprintf (ftmp,"=================");
		fprintf (ftmp,"==================\n");
	}
    return (EXIT_SUCCESS);
}

int
PrintDetails2 (void)
{
	char curr_code [4],
         bank_name [41];	
	double	lcl_amt;

	lcl_ptr = previousData;

	if (strcmp (cudr_rec.co_no,comm_rec.co_no))
		return (EXIT_FAILURE);

	if (headingFlag2 != 1)
    {
		if (PrintHeading2 () == 1)
        {
            return (EXIT_FAILURE);
        }
    }

	sprintf (currentCustomer,  "%-6.6s",   sptr + DBT_NO);
	sprintf (currentBank,   "%-5.5s",   sptr + BK_ID);
	sprintf (bank_name, "%-40.40s", sptr + BK_NAME);
	sprintf (curr_code, "%-3.3s",   sptr + BK_CCODE);
	sprintf (currentReceipt, "%-8.8s",   sptr + REC_NO);

	if (first_time)
	{
		sprintf (previousCustomer,  "%-6.6s", " ");
		sprintf (previousBank,   "%-5.5s", " ");
		sprintf (previousReceipt, "%-8.8s", sptr + REC_NO);
	}

	sprintf (cumr_rec.co_no,  "%-2.2s", sptr + CO_NO);
	sprintf (cumr_rec.est_no, "%-2.2s", sptr + BR_NO);
	sprintf (cumr_rec.dbt_no, "%-6.6s", sptr + DBT_NO);

	cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
	if (cc)
	{
		strcpy (cumr_rec.curr_code, " ");
		strcpy (cumr_rec.dbt_name, " ");
	}

	if (strcmp (previousReceipt, currentReceipt) && !first_time)
	{
		gross_rcpt_amt = atof (lcl_ptr + L_TOT_AMT);
		lcl_amt = gross_rcpt_amt - atof (lcl_ptr + L_DISC);
		bank_charges =atof (lcl_ptr + BK_CHRG)/atof (lcl_ptr + BK_L_EXCH);

		if (envDbMcurr)
		{
			fprintf (ftmp, "|%-8.8s", lcl_ptr + REC_NO);
			fprintf (ftmp, "| %3.3s ",  cumr_rec.curr_code);
			fprintf (ftmp, "|%s",       CF2 (atof (lcl_ptr + O_AMT)));
			fprintf (ftmp, "|%s",       CF2 (atof (lcl_ptr + O_DISC)));
			fprintf (ftmp, "| %9.4f ",  atof (lcl_ptr + BK_EXCH));
			fprintf (ftmp, "|%s",       CF2 (atof (lcl_ptr + BK_REC_AMT)));
			fprintf (ftmp,"|%s",       CF2 (atof (lcl_ptr + BK_CHRG)));
			fprintf (ftmp, "| %9.4f ",  atof (lcl_ptr + O_EXCH));
			fprintf (ftmp, "|%s",       CF2 (lcl_amt));
			fprintf (ftmp, "|%s",       CF2 (gross_rcpt_amt));
			fprintf (ftmp, "| %9.4f ",  atof (lcl_ptr + BK_L_EXCH));
			fprintf (ftmp, "|%s|\n",    CF2 (bank_charges));
		}
		else
		{
			fprintf (ftmp, "|%-8.8s", lcl_ptr + REC_NO);
			fprintf (ftmp, "|%s",       CF2 (lcl_amt));
			fprintf (ftmp, "|%s",       CF2 (atof (lcl_ptr + L_DISC)));
			fprintf (ftmp, "|%s",       CF2 (gross_rcpt_amt));
			fprintf (ftmp, "|%s|\n",    CF2 (bank_charges));
		}

		tot_amt_gross    += atof (lcl_ptr + BK_REC_AMT);
		tot_bk_chrg      += atof (lcl_ptr + BK_CHRG);
		tot_dbt_rcpt_amt += lcl_amt;
		tot_gr_rcpt_amt  += gross_rcpt_amt;
		tot_lcl_bk_chrg  += bank_charges;

		strcpy (previousReceipt, currentReceipt);
	}

	if (strcmp (previousBank, currentBank))
	{
		fprintf (ftmp,".DS6\n");
		fprintf (ftmp,".EBANK: %s", clip (bank_name));
		if (envDbMcurr)
			fprintf (ftmp,"     CURRENCY: %s\n", clip (curr_code));
		else
			fprintf (ftmp,"\n");
		fprintf (ftmp,".B1\n");

		if (envDbMcurr)
		{
			fprintf (ftmp,"=========");
			fprintf (ftmp,"======");
			fprintf (ftmp,"===============");
			fprintf (ftmp,"===============");
			fprintf (ftmp,"============");
			fprintf (ftmp,"===============");
			fprintf (ftmp,"===============");
			fprintf (ftmp,"============");
			fprintf (ftmp,"===============");
			fprintf (ftmp,"===============");
			fprintf (ftmp,"============");
			fprintf (ftmp,"================\n");

			fprintf (ftmp,"| RECEPT ");
			fprintf (ftmp,"|     ");
			fprintf (ftmp,"|             ORIGIN CURRENCY             ");
			fprintf (ftmp,"|              BANK CURRENCY              ");
			fprintf (ftmp,"|                        ");
			fprintf (ftmp,"LOCAL CURRENCY                  |\n");

			fprintf (ftmp,"| NUMBER ");
			fprintf (ftmp,"| CUR ");
			fprintf (ftmp,"| NET RCPT AMT ");
			fprintf (ftmp,"| DISCOUNT AMT ");
			fprintf (ftmp,"| EXCH RATE ");
			fprintf (ftmp,"| RCPT AMT GRS ");
			fprintf (ftmp,"|  BANK CHRG   ");
			fprintf (ftmp,"| EXCH RATE ");
			fprintf (ftmp,"|CUST RCPT AMT ");
			fprintf (ftmp,"| GRS RCPT AMT ");
			fprintf (ftmp,"| EXCH RATE ");
			fprintf (ftmp,"|  BANK CHRG   |\n");

			fprintf (ftmp,"|--------");
			fprintf (ftmp,"------");
			fprintf (ftmp,"---------------");
			fprintf (ftmp,"---------------");
			fprintf (ftmp,"------------");
			fprintf (ftmp,"---------------");
			fprintf (ftmp,"---------------");
			fprintf (ftmp,"------------");
			fprintf (ftmp,"---------------");
			fprintf (ftmp,"---------------");
			fprintf (ftmp,"------------");
			fprintf (ftmp,"---------------|\n");
		}
		else
		{
			fprintf (ftmp,"=========");
			fprintf (ftmp,"===============");
			fprintf (ftmp,"===============");
			fprintf (ftmp,"===============");
			fprintf (ftmp,"================\n");

			fprintf (ftmp,"| NUMBER ");
			fprintf (ftmp,"|CUST RCPT AMT ");
			fprintf (ftmp,"| DISCOUNT AMT ");
			fprintf (ftmp,"| GRS RCPT AMT ");
			fprintf (ftmp,"|  BANK CHRG   |\n");

			fprintf (ftmp,"|--------");
			fprintf (ftmp,"---------------");
			fprintf (ftmp,"---------------");
			fprintf (ftmp,"---------------");
			fprintf (ftmp,"---------------|\n");
		}
	
		if (!first_time)
			fprintf (ftmp,".PA\n");

	}
	
	if (strcmp (previousCustomer, currentCustomer))
	{
		if (!first_time && !strcmp (previousBank, currentBank))
		{
			if (envDbMcurr)
			{
				fprintf (ftmp,"|--------");
				fprintf (ftmp,"------");
				fprintf (ftmp,"---------------");
				fprintf (ftmp,"---------------");
				fprintf (ftmp,"------------");
				fprintf (ftmp,"---------------");
				fprintf (ftmp,"---------------");
				fprintf (ftmp,"------------");
				fprintf (ftmp,"---------------");
				fprintf (ftmp,"---------------");
				fprintf (ftmp,"------------");
				fprintf (ftmp,"---------------|\n");
			}
			else
			{
				fprintf (ftmp,"|--------");
				fprintf (ftmp,"---------------");
				fprintf (ftmp,"---------------");
				fprintf (ftmp,"---------------");
				fprintf (ftmp,"---------------|\n");
			}
		}

		fprintf (ftmp,"| %-6.6s", currentCustomer);
		fprintf (ftmp,"   %-40.40s", cumr_rec.dbt_name);
		if (envDbMcurr)
			fprintf (ftmp,"%-105.105s|\n", " ");
		else
			fprintf (ftmp,"%-18.18s|\n", " ");

		strcpy (previousCustomer, currentCustomer);
	}
		
	strcpy (previousBank, currentBank);
	first_time = FALSE;
    return (EXIT_SUCCESS);
}

/*========================== 
| Print header for 2nd rep.|
==========================*/
int
PrintHeading2 (void)
{
	headingFlag2 = TRUE;

	if ( (ftmp = popen ("pformat","w")) == 0)
	{
		sys_err ("Error in pformat During (POPEN)", 1, PNAME);
		shutdown_prog ();
        return (EXIT_FAILURE);
	}

	fprintf (ftmp,".START%s<%s>\n", DateToString (comm_rec.dbt_date), PNAME);
	fprintf (ftmp,".LP%d\n", lp_no);
	fprintf (ftmp,".SO\n");
	fprintf (ftmp,".7\n");
	fprintf (ftmp,".L%d\n", (envDbMcurr) ? 158 : 96);
	fprintf (ftmp,".B1\n");
	fprintf (ftmp,".ERECEIPTS JOURNAL (BATCH NO : %s)\n", batchNo);
	fprintf (ftmp,".B1\n");
	fprintf (ftmp,".E%s AS AT: %s\n",clip (comm_rec.co_short), SystemTime ());
	fprintf (ftmp,".PI12\n");

	if (envDbMcurr)
	{
		fprintf (ftmp,".R=========");
		fprintf (ftmp,"======");
		fprintf (ftmp,"===============");
		fprintf (ftmp,"===============");
		fprintf (ftmp,"============");
		fprintf (ftmp,"===============");
		fprintf (ftmp,"===============");
		fprintf (ftmp,"============");
		fprintf (ftmp,"===============");
		fprintf (ftmp,"===============");
		fprintf (ftmp,"============");
		fprintf (ftmp,"================\n");
	}
	else
	{
		fprintf (ftmp,".R=========");
		fprintf (ftmp,"===============");
		fprintf (ftmp,"===============");
		fprintf (ftmp,"===============");
		fprintf (ftmp,"================\n");
	}
    return (EXIT_SUCCESS);
}

void
PrintTotals2 (void)
{
	double	lcl_amt;

	gross_rcpt_amt = atof (lcl_ptr + L_TOT_AMT);
	lcl_amt = gross_rcpt_amt - atof (lcl_ptr + L_DISC);
	bank_charges = atof (lcl_ptr + BK_CHRG) / atof (lcl_ptr + BK_L_EXCH);

	if (envDbMcurr)
	{
		fprintf (ftmp, "|%-8.8s", lcl_ptr + REC_NO);
		fprintf (ftmp, "| %3.3s ",  cumr_rec.curr_code);
		fprintf (ftmp, "|%s",       CF2 (atof (lcl_ptr + O_AMT)));
		fprintf (ftmp, "|%s",       CF2 (atof (lcl_ptr + O_DISC)));
		fprintf (ftmp, "| %9.4f ",  atof (lcl_ptr + BK_EXCH));
		fprintf (ftmp, "|%s",       CF2 (atof (lcl_ptr + BK_REC_AMT)));
		fprintf (ftmp, "|%s",       CF2 (atof (lcl_ptr + BK_CHRG)));
		fprintf (ftmp, "| %9.4f ",  atof (lcl_ptr + O_EXCH));
		fprintf (ftmp, "|%s",       CF2 (lcl_amt));
		fprintf (ftmp, "|%s",       CF2 (gross_rcpt_amt));
		fprintf (ftmp, "| %9.4f ",  atof (lcl_ptr + BK_L_EXCH));
		fprintf (ftmp, "|%s|\n",    CF2 (bank_charges));
	}
	else
	{
		fprintf (ftmp, "|%-8.8s", lcl_ptr + REC_NO);
		fprintf (ftmp, "|%s",       CF2 (lcl_amt));
		fprintf (ftmp, "|%s",       CF2 (atof (lcl_ptr + L_DISC)));
		fprintf (ftmp, "|%s",       CF2 (gross_rcpt_amt));
		fprintf (ftmp, "|%s|\n",    CF2 (bank_charges));
	}

	tot_amt_gross    += atof (lcl_ptr + BK_REC_AMT);
	tot_bk_chrg      += atof (lcl_ptr + BK_CHRG);
	tot_dbt_rcpt_amt += lcl_amt;
	tot_gr_rcpt_amt  += gross_rcpt_amt;
	tot_lcl_bk_chrg  += bank_charges;

	if (envDbMcurr)
	{
		fprintf (ftmp, "|--------");
		fprintf (ftmp, "+-----");
		fprintf (ftmp, "+--------------");
		fprintf (ftmp, "+--------------");
		fprintf (ftmp, "+-----------");
		fprintf (ftmp, "+--------------");
		fprintf (ftmp, "+--------------");
		fprintf (ftmp, "+-----------");
		fprintf (ftmp, "+--------------");
		fprintf (ftmp, "+--------------");
		fprintf (ftmp, "+-----------");
		fprintf (ftmp, "+--------------|\n");

		fprintf (ftmp, "| TOTAL  ");
		fprintf (ftmp, "|     ");
		fprintf (ftmp, "|              ");
		fprintf (ftmp, "|              ");
		fprintf (ftmp, "|           ");
		fprintf (ftmp, "|%s",    CF2 (tot_amt_gross));
		fprintf (ftmp, "|%s",    CF2 (tot_bk_chrg));
		fprintf (ftmp, "|           ");
		fprintf (ftmp, "|%s",    CF2 (tot_dbt_rcpt_amt));
		fprintf (ftmp, "|%s",    CF2 (tot_gr_rcpt_amt));
		fprintf (ftmp, "|           ");
		fprintf (ftmp, "|%s|\n", CF2 (tot_lcl_bk_chrg));
	}
	else
	{
		fprintf (ftmp, "|--------");
		fprintf (ftmp, "+--------------");
		fprintf (ftmp, "+--------------");
		fprintf (ftmp, "+--------------");
		fprintf (ftmp, "+--------------|\n");

		fprintf (ftmp, "| TOTAL  ");
		fprintf (ftmp, "|%s",    CF2 (tot_dbt_rcpt_amt));
		fprintf (ftmp, "|              ");
		fprintf (ftmp, "|%s",    CF2 (tot_gr_rcpt_amt));
		fprintf (ftmp, "|%s|\n", CF2 (tot_lcl_bk_chrg));
	}

        fprintf (ftmp,".EOF\n");
	pclose (ftmp);
}

void
shutdown_prog (void)
{
	sort_delete (fsort,  "crcjnl");
	CloseDB (); 
	FinishProgram ();
}

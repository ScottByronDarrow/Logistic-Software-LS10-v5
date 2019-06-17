/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: db_crc2rep.c,v 5.4 2001/10/18 08:58:23 cha Exp $
|  Program Name  : (db_crc2rep.c) 
|  Program Desc  : (Prints Receipts Journal) 
|---------------------------------------------------------------------|
|  Author        : Campbell Mander.| Date Written  : (11/12/92)       |
|---------------------------------------------------------------------|
| $Log: db_crc2rep.c,v $
| Revision 5.4  2001/10/18 08:58:23  cha
| Updated to fix exchange rate as was only using 4
| decimal places instead of eight.
| Updated by Scott.
|
| Revision 5.3  2001/08/09 08:23:31  scott
| Added FinishProgram ();
|
| Revision 5.2  2001/08/06 23:21:51  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:17:03  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_crc2rep.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_crc2rep/db_crc2rep.c,v 5.4 2001/10/18 08:58:23 cha Exp $";

#include	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<twodec.h>
#include 	<ml_std_mess.h>
#include 	<ml_db_mess.h>

#define	CF(x)	 (comma_fmt (DOLLARS (x), "NNN,NNN,NNN.NN"))

	/*================================================================
	| Special fields and flags  ################################## . |
	================================================================*/
   	int		pid,		/* Process id number for work files.     */
			cusr_no,
			next_line,
     		printerNumber = 1,
    		headingflag = 0;

	FILE	*fsort,
			*ftmp;

#include	"schema"

struct commRecord	comm_rec;

	/*===========================================
	| Customer Sundry Receipts Work File Record. |
	===========================================*/
	struct {
		char	co_no [3];
		char	est [3];
		char	rpt_no [9];
		long    date_of_rpt;
		char    type [2];
		char    pay_type [2];
		char    gl_acc_no [17];
		char    p_no [3];
		char    name [41];
		char    acronym [10];
		char    narrative [21];
		char    bank_code [4];
		char    branch_code [21];
		Money	amt;  
		Money   disc;  
		Money   p_amt;  
		Money   tax;     
		Money   gst;      
		Money   freight;   
		char    stat_flag [2];

		char    bk_id [6];
		char    bk_curr [4];
		double  bk_exch;
		Money   bk_rec_amt; 
		Money   bk_charge;  
		double  bk_lcl_exch;

		Money	o_p_amt;
		char	o_curr [4];
		double  o_exch;
		Money   o_disc;   
		Money   o_amount; 
	} cusr_rec;

	int		envDbCo = 0,
			envDbFind = 0,
			first_time = TRUE;

	int		envDbMcurr = FALSE;
	
	int		BK_ID	   =	0,
			REC_NO	   =	5,
			BK_CCODE   =	13,
			BK_EXCH	   =	16,
			BK_REC_AMT =	28,
			BK_CHRG	   =	40,
			BK_L_EXCH  =	52,
			O_EXCH	   =	64,
			O_DISC	   =	76,
			O_AMT	   =	88,
			O_CURR	   =	100;

	double	bank_charges 	 = 0.00,
			gross_rcpt_amt   = 0.00,
			tot_amt_gross 	 = 0.00,
			tot_bk_chrg 	 = 0.00,
			tot_dbt_rcpt_amt = 0.00,
			tot_gr_rcpt_amt  = 0.00,
			tot_lcl_bk_chrg  = 0.00;

	char	*sptr,
			*lcl_ptr,
			prv_data [256],	
			curr_bk [6],
			curr_dbt [7],
			curr_rcpt [9],
			prev_bk [6],
			prev_dbt [8],
			prev_rcpt [9],
			prog_path [100];

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	ReadCusr 		(void);
int 	PrintDtl 		(void);
int 	ReportHeader 	(void);
void 	PrintTot 		(void);
void 	shutdown_prog 	(void);

int
main (
 int                argc,
 char*              argv [])
{
	int	data_fnd;

	if (argc != 3)
	{
		print_at (0,0,mlDbMess002, argv [0]);
		return (EXIT_FAILURE);
	}
	printerNumber = atoi (argv [1]);
	pid   = atoi (argv [2]);

	OpenDB ();

	envDbFind  = atoi (get_env ("DB_FIND"));
	envDbCo = atoi (get_env ("DB_CO"));

	sptr = get_env ("DB_MCURR");
	envDbMcurr = (sptr == (char *)0) ? FALSE : atoi (sptr);
 

	dsp_screen ("Printing Sundry Receipts Journals.",
		   comm_rec.co_no,
		   comm_rec.co_name);

	fsort = sort_open ("crc2jnl");
	ReadCusr ();

	data_fnd = FALSE;
	first_time = TRUE;
	strcpy (prv_data, "");
	sptr = sort_read (fsort);
	while (sptr)
	{
		data_fnd = TRUE;
        if (PrintDtl () == 1)
        {
            return (EXIT_FAILURE);
        }

		dsp_process ("Account : ",cusr_rec.bk_id);
		strcpy (prv_data, sptr);

		sptr = sort_read (fsort);
	}

	if (data_fnd)
		PrintTot (); 

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*======================
| Open Database Files. |
======================*/
void
OpenDB (void)
{
	char	filename [100];
	char	*sptr = getenv ("PROG_PATH");


	abc_dbopen ("data");

    /*============================================
    | Get common info from commom database file. |
    ============================================*/
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	sprintf (filename,"%s/WORK/db_crc%05d", 
			 (sptr == (char *) 0)? "/usr/LS10.5" : sptr, pid);
	cc = RF_OPEN (filename,sizeof (cusr_rec),"r",&cusr_no);
	if (cc) 
		file_err (cc, "db_crc", "WKOPEN");
}

/*=======================
| Close Database Files. |
=======================*/
void
CloseDB (void)
{
	abc_fclose (comm);
	cc = RF_CLOSE (cusr_no);
	if (cc) 
		file_err (cc, "db_crc", "WKCLOSE");

	abc_dbclose ("data");
}


/*============================================
| Get records from creditors/dbtrs work file.|
============================================*/
void
ReadCusr (void)
{
    char    sort_temp [180];
	
	cc = RF_READ (cusr_no, (char *) &cusr_rec);
	while (!cc)
	{
		sprintf 
		(
			sort_temp, 
			 "%-5.5s%-8.8s%-3.3s%12.8f%12.2f%12.2f%12.8f%12.8f%12.2f%12.2f%-3.3s\n",
			 cusr_rec.bk_id,
			 cusr_rec.rpt_no,
			 cusr_rec.bk_curr,
			 cusr_rec.bk_exch,
			 cusr_rec.bk_rec_amt,
			 cusr_rec.bk_charge,
			 cusr_rec.bk_lcl_exch,
			 cusr_rec.o_exch,
			 cusr_rec.o_disc,
			 cusr_rec.o_amount,
			 cusr_rec.o_curr
		 );
        sort_save (fsort, sort_temp);

		cc = RF_READ (cusr_no, (char *) &cusr_rec);
	}

	fsort = sort_sort (fsort, "crc2jnl");

}

int
PrintDtl (void)
{
	char	curr_code [4],
			bank_name [41];	
	double	lcl_amt;
	double	lcl_disc;
	double	fgn_amt;
	double	fgn_disc;
	double	fgn_exch;

	lcl_ptr = prv_data;

	if (headingflag != 1)
    {
		if (ReportHeader () == 1)
        {
            return (EXIT_FAILURE);
        }
    }

	sprintf (bank_name, "%-5.5s", sptr + BK_ID);
	sprintf (curr_bk,   "%-5.5s", sptr + BK_ID);
	sprintf (curr_code, "%-3.3s", sptr + BK_CCODE);
	sprintf (curr_rcpt, "%-8.8s", sptr + REC_NO);

	if (first_time)
	{
		sprintf (prev_dbt,  "%-6.6s", " ");
		sprintf (prev_bk,   "%-5.5s", " ");
		sprintf (prev_rcpt, "%-8.8s", sptr + REC_NO);
	}

	lcl_disc = atof (lcl_ptr + O_DISC);
	fgn_amt  = atof (lcl_ptr + O_AMT);
	fgn_disc = atof (lcl_ptr + O_DISC);
	fgn_exch = atof (lcl_ptr + O_EXCH);

	if (strcmp (prev_rcpt, curr_rcpt) && !first_time)
	{
		gross_rcpt_amt = fgn_amt / fgn_exch;
		lcl_amt = gross_rcpt_amt - (fgn_disc / fgn_exch);
		bank_charges =atof (lcl_ptr + BK_CHRG)/atof (lcl_ptr + BK_L_EXCH);

		if (envDbMcurr)
		{
			fprintf (ftmp,"|%-8.8s",lcl_ptr + REC_NO);
			fprintf (ftmp,"| %3.3s ", lcl_ptr + O_CURR);
			fprintf (ftmp,"|%s",      CF (fgn_amt));
			fprintf (ftmp,"|%s",      CF (fgn_disc));
			fprintf (ftmp,"|%11.8f", atof (lcl_ptr + BK_EXCH));
			fprintf (ftmp,"|%s",     CF (atof (lcl_ptr + BK_REC_AMT)));
			fprintf (ftmp,"|%s",      CF (atof (lcl_ptr + BK_CHRG)));
			fprintf (ftmp,"|%11.8f", fgn_exch);
			fprintf (ftmp,"|%s",      CF (lcl_amt));
			fprintf (ftmp,"|%s",      CF (gross_rcpt_amt));
			fprintf (ftmp,"|%11.4f", atof (lcl_ptr + BK_L_EXCH));
			fprintf (ftmp,"|%s|\n",   CF (bank_charges));
		}
		else
		{
			fprintf (ftmp,"|  %-8.8s  ", lcl_ptr + REC_NO);
			fprintf (ftmp,"|%s",     CF (lcl_amt));
			fprintf (ftmp,"|%s",     CF (lcl_disc));
			fprintf (ftmp,"|%s",     CF (gross_rcpt_amt));
			fprintf (ftmp,"|%s|\n",  CF (bank_charges));
		}

		tot_amt_gross    += atof (lcl_ptr + BK_REC_AMT);
		tot_bk_chrg      += atof (lcl_ptr + BK_CHRG);
		tot_dbt_rcpt_amt += lcl_amt;
		tot_gr_rcpt_amt  += gross_rcpt_amt;
		tot_lcl_bk_chrg  += bank_charges;

		strcpy (prev_rcpt, curr_rcpt);
	}

	if (strcmp (prev_bk, curr_bk))
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
			fprintf (ftmp, "=========");
			fprintf (ftmp, "======");
			fprintf (ftmp, "===============");
			fprintf (ftmp, "===============");
			fprintf (ftmp, "============");
			fprintf (ftmp, "===============");
			fprintf (ftmp, "===============");
			fprintf (ftmp, "============");
			fprintf (ftmp, "===============");
			fprintf (ftmp, "===============");
			fprintf (ftmp, "============");
			fprintf (ftmp, "================\n");
	
			fprintf (ftmp,"| RECEPT ");
			fprintf (ftmp,"|     ");
			fprintf (ftmp,"|             ORIGIN CURRENCY  ");
			fprintf (ftmp,"           ");
			fprintf (ftmp,"|              BANK CURRENCY");
			fprintf (ftmp,"              ");
			fprintf (ftmp,"|                        LOCAL C");
			fprintf (ftmp,"URRENCY                  |\n");
	
			fprintf (ftmp, "| NUMBER ");
			fprintf (ftmp, "| CUR ");
			fprintf (ftmp, "| NET RCPT AMT ");
			fprintf (ftmp, "| DISCOUNT AMT ");
			fprintf (ftmp, "| EXCH RATE ");
			fprintf (ftmp, "| RCPT AMT GRS ");
			fprintf (ftmp, "|  BANK CHRG   ");
			fprintf (ftmp, "| EXCH RATE ");
			fprintf (ftmp, "| CUST RCPT AMT");
			fprintf (ftmp, "| GRS RCPT AMT ");
			fprintf (ftmp, "| EXCH RATE ");
			fprintf (ftmp, "|  BANK CHRG   |\n");
	
			fprintf (ftmp, "|--------");
			fprintf (ftmp, "------");
			fprintf (ftmp, "---------------");
			fprintf (ftmp, "---------------");
			fprintf (ftmp, "------------");
			fprintf (ftmp, "---------------");
			fprintf (ftmp, "---------------");
			fprintf (ftmp, "------------");
			fprintf (ftmp, "---------------");
			fprintf (ftmp, "---------------");
			fprintf (ftmp, "------------");
			fprintf (ftmp, "---------------|\n");
		}
		else
		{
			fprintf (ftmp, "=============");
			fprintf (ftmp, "===============");
			fprintf (ftmp, "===============");
			fprintf (ftmp, "===============");
			fprintf (ftmp, "================\n");

			fprintf (ftmp, "| RECEIPT NO ");
			fprintf (ftmp, "| DISCOUNT AMT ");
			fprintf (ftmp, "| CUST RCPT AMT");
			fprintf (ftmp, "| GRS RCPT AMT ");
			fprintf (ftmp, "| BANK CHRG    |\n");

			fprintf (ftmp, "|------------");
			fprintf (ftmp, "---------------");
			fprintf (ftmp, "---------------");
			fprintf (ftmp, "---------------");
			fprintf (ftmp, "---------------|\n");
		}

		if (!first_time)
			fprintf (ftmp,".PA\n");
	}
	
	strcpy (prev_bk, curr_bk);
	first_time = FALSE;
    return (EXIT_SUCCESS);
}

int
ReportHeader (void)
{
	headingflag = TRUE;

	if ( (ftmp = popen ("pformat","w")) == 0)
	{
		sys_err ("Error in pformat During (POPEN)", 1, PNAME);
		shutdown_prog ();
        return (EXIT_SUCCESS);
	}

	fprintf (ftmp, ".START%s<%s>\n", DateToString (comm_rec.dbt_date), PNAME);
	fprintf (ftmp,".LP%d\n", printerNumber);
	fprintf (ftmp,".SO\n");
	fprintf (ftmp,".7\n");
	fprintf (ftmp,".L%d\n", (envDbMcurr) ? 158 : 96);
	fprintf (ftmp,".B1\n");
	fprintf (ftmp,".ESUNDRY RECEIPTS AUDIT REPORT.\n");
	fprintf (ftmp,".B1\n");
	fprintf (ftmp,".E%s AS AT: %s\n",clip (comm_rec.co_short), SystemTime ());
	fprintf (ftmp,".PI12\n");

	if (envDbMcurr)
	{
		fprintf (ftmp, ".R=========");
		fprintf (ftmp, "======");
		fprintf (ftmp, "===============");
		fprintf (ftmp, "===============");
		fprintf (ftmp, "============");
		fprintf (ftmp, "===============");
		fprintf (ftmp, "===============");
		fprintf (ftmp, "============");
		fprintf (ftmp, "===============");
		fprintf (ftmp, "===============");
		fprintf (ftmp, "============");
		fprintf (ftmp, "================\n");
	}
	else
	{
		fprintf (ftmp, ".R=============");
		fprintf (ftmp, "===============");
		fprintf (ftmp, "===============");
		fprintf (ftmp, "===============");
		fprintf (ftmp, "================\n");
	}
    return (EXIT_SUCCESS);
}

void
PrintTot (void)
{	
	double	lcl_amt;
	double	fgn_amt;
	double	fgn_disc;
	double	fgn_exch;

	fgn_amt = atof (lcl_ptr + O_AMT);
	fgn_disc = atof (lcl_ptr + O_DISC);
	fgn_exch = atof (lcl_ptr + O_EXCH);

	gross_rcpt_amt = fgn_amt / fgn_exch;
	lcl_amt = gross_rcpt_amt - (fgn_disc / fgn_exch);
	bank_charges = atof (lcl_ptr + BK_CHRG) / atof (lcl_ptr + BK_L_EXCH);

	if (envDbMcurr)
	{
		fprintf (ftmp,"|%-8.8s", lcl_ptr + REC_NO);
		fprintf (ftmp,"| %3.3s ",lcl_ptr + O_CURR);
		fprintf (ftmp,"|%s",     CF (atof (lcl_ptr + O_AMT)));
		fprintf (ftmp,"|%s",     CF (atof (lcl_ptr + O_DISC)));
		fprintf (ftmp,"|%11.8f",atof (lcl_ptr + BK_EXCH));
		fprintf (ftmp,"|%s",     CF (atof (lcl_ptr + BK_REC_AMT)));
		fprintf (ftmp,"|%s",     CF (atof (lcl_ptr + BK_CHRG)));
		fprintf (ftmp,"|%11.8f",atof (lcl_ptr + O_EXCH));
		fprintf (ftmp,"|%s",     CF (lcl_amt));
		fprintf (ftmp,"|%s",     CF (gross_rcpt_amt));
		fprintf (ftmp,"|%11.8f",atof (lcl_ptr + BK_L_EXCH));
		fprintf (ftmp,"|%s|\n",  CF (bank_charges));
	}
	else
	{
		fprintf (ftmp,"|   %-6.6s   ", lcl_ptr + REC_NO);
		fprintf (ftmp,"|%s",    CF (lcl_amt));
		fprintf (ftmp,"|%s",    CF (atof (lcl_ptr + O_DISC)));
		fprintf (ftmp,"|%s",    CF (gross_rcpt_amt));
		fprintf (ftmp,"|%s|\n", CF (bank_charges));
	}

	tot_amt_gross    += atof (lcl_ptr + BK_REC_AMT);
	tot_bk_chrg      += atof (lcl_ptr + BK_CHRG);
	tot_dbt_rcpt_amt += lcl_amt;
	tot_gr_rcpt_amt  += gross_rcpt_amt;
	tot_lcl_bk_chrg  += bank_charges;

	if (envDbMcurr)
	{
		fprintf (ftmp, "|--------");
		fprintf (ftmp, "------");
		fprintf (ftmp, "---------------");
		fprintf (ftmp, "---------------");
		fprintf (ftmp, "------------");
		fprintf (ftmp, "---------------");
		fprintf (ftmp, "---------------");
		fprintf (ftmp, "------------");
		fprintf (ftmp, "---------------");
		fprintf (ftmp, "---------------");
		fprintf (ftmp, "------------");
		fprintf (ftmp, "---------------|\n");
	}
	else
	{
		fprintf (ftmp, "|------------");
		fprintf (ftmp, "+--------------");
		fprintf (ftmp, "+--------------");
		fprintf (ftmp, "+--------------");
		fprintf (ftmp, "+--------------|\n");
	}

	if (envDbMcurr)
	{
		fprintf (ftmp, "| TOTAL  ");
		fprintf (ftmp, "|     ");
		fprintf (ftmp, "|              ");
		fprintf (ftmp, "|              ");
		fprintf (ftmp, "|           ");
		fprintf (ftmp, "|%s",    CF (tot_amt_gross));
		fprintf (ftmp, "|%s",    CF (tot_bk_chrg));
		fprintf (ftmp, "|           ");
		fprintf (ftmp, "|%s",    CF (tot_dbt_rcpt_amt));
		fprintf (ftmp, "|%s",    CF (tot_gr_rcpt_amt));
		fprintf (ftmp, "|           ");
		fprintf (ftmp, "|%s|\n", CF (tot_lcl_bk_chrg));
	}
	else
	{
		fprintf (ftmp, "|   TOTAL    ");
		fprintf (ftmp, "|%s",    CF (tot_dbt_rcpt_amt));
		fprintf (ftmp, "|              ");
		fprintf (ftmp, "|%s",    CF (tot_gr_rcpt_amt));
		fprintf (ftmp, "|%s|\n", CF (tot_lcl_bk_chrg));
	}

        fprintf (ftmp, ".EOF\n");

	pclose (ftmp);
}

void
shutdown_prog (void)
{
	CloseDB (); 
	sort_delete (fsort, "crc2jnl");
	FinishProgram ();
}

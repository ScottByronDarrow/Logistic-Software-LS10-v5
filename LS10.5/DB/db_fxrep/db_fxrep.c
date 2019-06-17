/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: db_fxrep.c,v 5.3 2001/08/23 11:34:22 scott Exp $
|  Program Name  : (db_fxrep.c   )                                    |
|  Program Desc  : (Customer FX Transaction Exposure Print.     )     |
|                  (                                            )     |
|---------------------------------------------------------------------|
|  Access files  :  comm, cumr, cuin, cudt, cuhd,pocr ,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A)                                              |
|---------------------------------------------------------------------|
|  Author        : Simon Spratt.   | Date Written  : 06/08/96         |
|---------------------------------------------------------------------|
|  Date Modified : (03/09/97)      | Modified  by  : Jiggs A Veloz    |
|                                                                     |
|  Comments      : (06/08/96) - This report prints the booked trans   |
|                :              for debtors and shows the present day |
|                :              exchange equivalents and the FX       | 
|                :              exposure.
| $Log: db_fxrep.c,v $
| Revision 5.3  2001/08/23 11:34:22  scott
| Updated from scotts machine
|
| Revision 5.2  2001/08/09 09:07:08  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:22:01  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:04:34  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:25:05  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:13:47  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.1  2000/09/06 07:48:59  scott
| Updated to add new customer search - same as stock search.
| Some general maintenance was performed to add app.schema to some old code.
|
| Revision 2.0  2000/07/15 08:52:31  gerry
| Forced Revision No. Start 2.0 Rel-15072000
|
| Revision 1.15  2000/03/13 02:35:42  vij
| inserted sleep () calls after displaying error messages.
|
| Revision 1.14  1999/12/06 01:28:32  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.13  1999/11/17 06:39:47  scott
| Updated to remove PNAME as now available with Ctrl-P.
|
| Revision 1.12  1999/10/29 03:15:26  scott
| Updated for warning due to usage of -Wall flag on compiler.
|
| Revision 1.11  1999/10/20 02:06:44  nz
| Updated for final changes on date routines.
|
| Revision 1.10  1999/10/16 04:56:23  nz
| Updated for pjulmdy and pmdyjul routines.
|
| Revision 1.9  1999/10/05 07:34:11  scott
| Updated from ansi project.
|
| Revision 1.8  1999/06/14 23:25:52  scott
| Updated to add log and to change database name to data.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_fxrep.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_fxrep/db_fxrep.c,v 5.3 2001/08/23 11:34:22 scott Exp $";

#include 	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<ml_db_mess.h>
#include 	<ml_std_mess.h>

FILE	*pp;
	
#include	"schema"

struct commRecord	comm_rec;
struct cuinRecord	cuin_rec;
struct cuhdRecord	cuhd_rec;
struct cumrRecord	cumr_rec;
struct cumrRecord	cumr2_rec;
struct pocrRecord	pocr_rec;

	Money	*cumr_balance	=	&cumr_rec.bo_current;

	char	*data = "data",
			*cumr2 = "cumr2";

	int		dtls_cnt;
	long	start_date = 0L;
	double	grand_book_total = 0.00,
			grand_curr_total = 0.00,
			grand_diff_total = 0.00,
			cust_amt_total = 0.00,
			cust_book_total = 0.00,
			cust_curr_total = 0.00,
			cust_diff_total = 0.00,
			curr_amt_total = 0.00,
			curr_book_total = 0.00,
			curr_curr_total = 0.00,
			curr_diff_total = 0.00;
	char	all_trans [2];
	
	static char *inv_type [] = {
			"INVOICE",
			"CREDIT ",
			"JOURNAL",
			"PAYMENT"
	};

	int		firstPrint 		 = 0,
			printerNumber 	 = 1,
			envVarDbNettUsed = TRUE,
			envVarDbMcurr 	 = FALSE;

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	s_dbt_no [7];
	char	e_dbt_no [7];
	char	dbt_name [2] [41];
	long	start_date;
	long	end_date;
	long	lsystemDate;
	int		lpno;
	char	summary [2];
	char 	customer [2];
} local_rec;

static	struct	var	vars []	={	

	{1, LIN, "s_dbt_no", 3, 20, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", " ", "Start Customer :", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.s_dbt_no}, 
	{1, LIN, "s_dbt_name", 3, 30, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.dbt_name [0]}, 
	{1, LIN, "e_dbt_no", 4, 20, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", " ", "End Customer   :", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.e_dbt_no}, 
	{1, LIN, "e_dbt_name", 4, 30, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.dbt_name [1]}, 
	{1, LIN, "st_date",	 6, 20, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "0", "Start Date     :", "Enter Date for Starting Transaction - Default is start ",
		YES, NO,  JUSTLEFT, "", "", (char *) &local_rec.start_date},
	{1, LIN, "end_date",	 7, 20, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", " ", "End   Date     :", "Enter Date for Ending Transaction - Default is Today ",
		YES, NO,  JUSTLEFT, "", "", (char *) &local_rec.end_date},
	{1, LIN, "customer", 9, 20, CHARTYPE, 
		"U", "          ", 
		" ", "N", "Print by customer :  ", "Y(es) to print by customer or or N(o) to print currency totals only. ", 
		YES, NO, JUSTLEFT, "YN", "", local_rec.customer}, 
	{1, LIN, "summary", 11, 20, CHARTYPE, 
		"U", "          ", 
		" ", "S", "Summary or Detail :  ", "S(ummary) by customer or  D(etail) for detailed invoices.", 
		NA, NO, JUSTLEFT, "SD", "", local_rec.summary}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""}, 
};

#include <FindCumr.h>
/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void	ProcessFile 	 (void);
void 	PrintHeading 	 (void);
void 	OpenDB 			 (void);
void 	CloseDB 		 (void);
void 	shutdown_prog 	 (void);
int 	spec_valid 		 (int);
void 	ReportHeading 	 (int);
void 	PrintInvoice 	 (void);
void 	PrintCheque 	 (void);
int 	heading 		 (int);

char	branchNumber [3];

int
main (
 int                argc,
 char*              argv [])
{
	int		envVarDbCo;
	char	*sptr;
	char	prev_currency [4];
	int		first_time;

	local_rec.lsystemDate = TodaysDate ();

	sptr = chk_env ("DB_NETT_USED");
	envVarDbNettUsed = (sptr == (char *)0) ? TRUE : atoi (sptr);

	sptr = chk_env ("DB_MCURR");
	envVarDbMcurr = (sptr == (char *)0) ? TRUE : atoi (sptr);

	sptr = chk_env ("DB_CO");
	envVarDbCo = (sptr == (char *) 0) ? 0 : atoi (sptr);
	strcpy (branchNumber, (envVarDbCo) ? comm_rec.est_no : " 0");

	if (argc != 2)
	{
		/*----------------------------
		| Usage %s printer-no\n\r 	|
		----------------------------*/
		print_at (0,0, mlStdMess036, argv [0]);
        return (EXIT_FAILURE);
	}
	printerNumber = atoi (argv [1]);

	SETUP_SCR (vars);

	init_scr ();			/*  sets terminal from termcap	*/
	set_tty ();                      /*  get into raw mode		*/
	set_masks ();			/*  setup print using masks	*/
	init_vars (1);			/*  set default values		*/

	OpenDB ();

	while (!prog_exit)
	{
		/*=====================
		| Reset control flags |
		=====================*/
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		search_ok = 1;
		init_ok = 1;
		init_vars (1);	

		/*-----------------------------
		| Edit screen 1 linear input. |
		-----------------------------*/
		heading (1);
		entry (1);
		if (restart || prog_exit) 
			continue;

		heading (1);
		scn_display (1);
		edit (1);
		if (restart) 
			continue;

		if (!restart)
		{ 
			dsp_screen ("Printing Customers Foreign Exchange Exposure Report.",
					comm_rec.co_no,comm_rec.co_name);

			ReportHeading (printerNumber);

			strcpy (cumr_rec.co_no, comm_rec.co_no);
			strcpy (cumr_rec.est_no, branchNumber);

			start_date	=	MonthStart (comm_rec.dbt_date);
			first_time = TRUE;

			abc_selfield (cumr, "cumr_id_no6");
			strcpy (cumr_rec.co_no, comm_rec.co_no);
			strcpy (cumr_rec.est_no, branchNumber);
			strcpy (cumr_rec.curr_code, "   ");
			strcpy (cumr_rec.dbt_no, "      ");
			cc = find_rec (cumr, &cumr_rec, GTEQ, "r");
			while (!cc && !strcmp (cumr_rec.co_no, comm_rec.co_no) &&
						!strcmp (cumr_rec.est_no, branchNumber))
			{
				/*------------------------------
				| Don't process child debtors. |
				------------------------------*/
				if (cumr_rec.ho_dbt_hash > 0L)
				{
					cc = find_rec (cumr, &cumr_rec, NEXT, "r");
					continue;
				}
				if (strcmp (cumr_rec.dbt_no, local_rec.s_dbt_no) < 0)
				{
					cc = find_rec (cumr, &cumr_rec, NEXT, "r");
					continue;
				}
				if (first_time)
				{
					first_time = FALSE;
					sprintf (prev_currency, "%-3.3s", cumr_rec.curr_code);
				}
				if (strncmp (prev_currency, cumr_rec.curr_code, 3))
				{
					if (local_rec.customer [0] == 'Y')
						fprintf (pp,"|             |    |    |        |         %3.3s TOTAL   |", prev_currency);
					else
						fprintf (pp,"|                                          %3.3s TOTAL   |", prev_currency);

					if (envVarDbMcurr)
					{
						fprintf (pp,"%14.14s |", 
								comma_fmt (DOLLARS (curr_amt_total), "NNN,NNN,NNN.NN"));
						fprintf (pp,"%14.14s |", 
								comma_fmt (DOLLARS (curr_book_total), "NNN,NNN,NNN.NN"));
						fprintf (pp,"%14.14s |", 
								comma_fmt (DOLLARS (curr_curr_total), "NNN,NNN,NNN.NN"));
						fprintf (pp,"%14.14s |\n", 
								comma_fmt (DOLLARS (curr_diff_total), "NNN,NNN,NNN.NN"));
					}
					else
					{
						fprintf (pp,"%14.14s |\n",
								comma_fmt (DOLLARS (curr_amt_total), "NNN,NNN,NNN.NN"));
					}
					if (local_rec.customer [0] == 'Y')
						fprintf (pp,"|-------------|----|----|--------|----------|----------|");
					else
						fprintf (pp,"|-------------------------------------------------------");
						
					fprintf (pp,"---------------%s", (envVarDbMcurr) ? "|---------------|---------------|---------------|\n" : "|\n");
					curr_amt_total = 0.00;
					curr_book_total = 0.00;
					curr_curr_total = 0.00;
					curr_diff_total = 0.00;
					sprintf (prev_currency, "%-3.3s", cumr_rec.curr_code);
				}
				dsp_process ("Customer : ",cumr_rec.dbt_no);
				ProcessFile ();
				if (!strcmp (cumr_rec.dbt_no, local_rec.e_dbt_no))
				{
					abc_selfield (cumr, "cumr_id_no2");
					break;
				}
				cc = find_rec (cumr, &cumr_rec, NEXT, "r");
			}
			abc_selfield (cumr, "cumr_id_no2");

			/*---------------------
			| print final totals. |
			---------------------*/
			if (local_rec.customer [0] == 'Y')
				fprintf (pp,"|             |    |    |        |         %3.3s TOTAL   |", prev_currency);
			else
				fprintf (pp,"|                                          %3.3s TOTAL   |", prev_currency);
				
			if (envVarDbMcurr)
			{
				fprintf (pp,"%14.14s |", 
						comma_fmt (DOLLARS (curr_amt_total), "NNN,NNN,NNN.NN"));
				fprintf (pp,"%14.14s |", 
						comma_fmt (DOLLARS (curr_book_total), "NNN,NNN,NNN.NN"));
				fprintf (pp,"%14.14s |", 
						comma_fmt (DOLLARS (curr_curr_total), "NNN,NNN,NNN.NN"));
				fprintf (pp,"%14.14s |\n", 
						comma_fmt (DOLLARS (curr_diff_total), "NNN,NNN,NNN.NN"));
			}
			else
			{
				fprintf (pp,"%14.14s |\n",
						comma_fmt (DOLLARS (curr_amt_total), "NNN,NNN,NNN.NN"));
			}
			fprintf (pp,".LRP4\n");
			if (local_rec.customer [0] == 'Y')
				fprintf (pp,"|=============|====|====|========|==========|==========|");
			else
				fprintf (pp,"|======================================================|");
			fprintf (pp,"===============%s", (envVarDbMcurr) ? "|===============|===============|===============|\n" 
													: "|\n");

			if (local_rec.customer [0] == 'Y')
				fprintf (pp,"|             |    |    |       ** GRAND TOTALS **     |");
			else
				fprintf (pp,"|                               ** GRAND TOTALS **     |");
			if (envVarDbMcurr)
			{
				fprintf (pp,"               |%14.14s |", 
						comma_fmt (DOLLARS (grand_book_total), "NNN,NNN,NNN.NN"));
				fprintf (pp,"%14.14s |", 
						comma_fmt (DOLLARS (grand_curr_total), "NNN,NNN,NNN.NN"));
				fprintf (pp,"%14.14s |\n", 
						comma_fmt (DOLLARS (grand_diff_total), "NNN,NNN,NNN.NN"));
			}
			else	
				fprintf (pp,"%14.14s !\n", 
						comma_fmt (DOLLARS (grand_book_total),"NNN,NNN,NNN.NN"));

			fprintf (pp,".EOF\n");
			pclose (pp);
		} 
		prog_exit = 1; 
	}
	shutdown_prog ();   
    return (EXIT_SUCCESS);
}

void
ProcessFile (void)
{

	firstPrint = 0;

	strcpy (pocr_rec.co_no, cumr_rec.co_no);
	strcpy (pocr_rec.code, cumr_rec.curr_code);
	cc = find_rec (pocr, &pocr_rec, COMPARISON, "r");

	cuin_rec.date_of_inv 		= local_rec.start_date;
	cuin_rec.ho_hash 	= cumr_rec.hhcu_hash;
	strcpy (cuin_rec.est, "  ");
	cc = find_rec (cuin, &cuin_rec, GTEQ, "r");

	while (!cc && cuin_rec.ho_hash == cumr_rec.hhcu_hash &&
			cuin_rec.date_of_inv <= local_rec.end_date)
	{
		PrintInvoice ();
		cc = find_rec (cuin, &cuin_rec, NEXT, "r");
	}
	cuhd_rec.hhcu_hash = cumr_rec.hhcu_hash;
	strcpy (cuhd_rec.receipt_no, "        ");
	cuhd_rec.index_date	=	0;
		
	if (local_rec.customer [0] == 'Y')
	{
		if (firstPrint)
		{
			fprintf (pp,"|             |    |    |        |          |  TOTAL   |");
			if (envVarDbMcurr)
			{
				fprintf (pp,"%14.14s |", 
						comma_fmt (DOLLARS (cust_amt_total), "NNN,NNN,NNN.NN"));
				fprintf (pp,"%14.14s |", 
						comma_fmt (DOLLARS (cust_book_total), "NNN,NNN,NNN.NN"));
				fprintf (pp,"%14.14s |", 
						comma_fmt (DOLLARS (cust_curr_total), "NNN,NNN,NNN.NN"));
				fprintf (pp,"%14.14s |\n", 
						comma_fmt (DOLLARS (cust_diff_total), "NNN,NNN,NNN.NN"));
			}
			else
			{
				fprintf (pp,"%14.14s |\n",
						comma_fmt (DOLLARS (cust_amt_total), "NNN,NNN,NNN.NN"));
			}
			fprintf (pp,"|-------------|----|----|--------|----------|----------|");
			fprintf (pp,"---------------%s", (envVarDbMcurr) ? "|---------------|---------------|---------------|\n" : "|\n");
		}
	}
	cust_amt_total = 0.00;
	cust_book_total = 0.00;
	cust_curr_total = 0.00;
	cust_diff_total = 0.00;
}

void
PrintHeading (void)
{
	if (local_rec.customer [0] == 'Y')
	{
		if (envVarDbMcurr)
		{
			fprintf (pp,"| %s (%s)        %s                         |               |               |               |\n",
				cumr_rec.dbt_no, cumr_rec.dbt_name, cumr_rec.curr_code);
		}
		else
		{
			fprintf (pp,"| %s (%s)                    |\n",
				cumr_rec.dbt_no, cumr_rec.dbt_name);
		}
	}
}
/*======================
| Open database Files. |
======================*/
void
OpenDB (void)
{
	abc_dbopen (data);

	/*-------------------------------
	| Read common terminal record . |
	-------------------------------*/
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	abc_alias (cumr2, cumr);
	open_rec (cuin, cuin_list, CUIN_NO_FIELDS, "cuin_ho_cron");
	open_rec (cuhd, cuhd_list, CUHD_NO_FIELDS, "cuhd_id_no");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_id_no2");
	open_rec (pocr, pocr_list, POCR_NO_FIELDS, "pocr_id_no");
	open_rec (cumr2,cumr_list, CUMR_NO_FIELDS, "cumr_ho_dbt_hash");
}

/*=======================
| Close database Files. |
=======================*/
void
CloseDB (void)
{
	abc_fclose (cumr2);
	abc_fclose (cumr);
	abc_fclose (cuin);
	abc_fclose (cuhd);
	abc_fclose (pocr);
	abc_dbclose (data);
}

void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

int
spec_valid (
 int                field)
{
	/*------------------------+
	| Validate Start Customer |
	+------------------------*/
	if (LCHECK ("s_dbt_no"))
	{
		if (dflt_used)
		{
			sprintf (local_rec.dbt_name [0],"%-40.40s","Start Customer");
			strcpy (local_rec.s_dbt_no,"      ");
			DSP_FLD ("s_dbt_name");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}  
		
        if (prog_status != ENTRY &&
		    strcmp (local_rec.s_dbt_no,local_rec.e_dbt_no) > 0)
	 	{ 
			/*---------------------------------------------------------
			| Start Customer %s Must Not Be GREATER THAN End Customer |
			---------------------------------------------------------*/
			print_mess (ML (mlStdMess017));
			return (EXIT_FAILURE);
		}

		abc_selfield (cumr, "cumr_id_no");
        strcpy (cumr_rec.co_no,comm_rec.co_no);
        strcpy (cumr_rec.est_no,branchNumber);
        strcpy (cumr_rec.dbt_no,pad_num (local_rec.s_dbt_no));
		cc = find_rec (cumr,&cumr_rec,COMPARISON,"r");
		if (cc)
		{
			/*----------------------------
			| Customer %s is not on file |
			----------------------------*/
			print_mess (ML (mlStdMess021));
			sleep (sleepTime);
			abc_selfield (cumr, "cumr_id_no2");
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.dbt_name [0],"%-40.40s",cumr_rec.dbt_name);
		DSP_FLD ("s_dbt_name");
		abc_selfield (cumr, "cumr_id_no2");
		return (EXIT_SUCCESS);
	}

	/*----------------------+
	| Validate End Customer |
	+----------------------*/
	if (LCHECK ("e_dbt_no"))
	{
		if (dflt_used)
		{
			sprintf (local_rec.dbt_name [1],"%-40.40s","End   Customer");
			strcpy (local_rec.e_dbt_no,"~~~~~~");
			DSP_FLD ("e_dbt_name");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}
		
		abc_selfield (cumr, "cumr_id_no");
        strcpy (cumr_rec.co_no,comm_rec.co_no);
        strcpy (cumr_rec.est_no, branchNumber);
        strcpy (cumr_rec.dbt_no,pad_num (local_rec.e_dbt_no));
	    cc = find_rec (cumr,&cumr_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess (ML (mlStdMess021));
			sleep (sleepTime);
			abc_selfield (cumr, "cumr_id_no2");
			return (EXIT_FAILURE);
		}
		if (strcmp (local_rec.s_dbt_no,local_rec.e_dbt_no) > 0)
		{
			/*-----------------------------------------------------
			| End Customer %s may not be less than Start Customer |
			-----------------------------------------------------*/
			errmess (ML (mlStdMess018));
			sleep (sleepTime);
			abc_selfield (cumr, "cumr_id_no2");
			return (EXIT_FAILURE);
		}
		
		sprintf (local_rec.dbt_name [1],"%-40.40s",cumr_rec.dbt_name);
		DSP_FLD ("e_dbt_name");
		abc_selfield (cumr, "cumr_id_no2");
		return (EXIT_SUCCESS);
	}

	/*----------------------
	| Validate Start Date. |
	----------------------*/
	if (LCHECK ("st_date"))
	{
		if (dflt_used)
		{
			local_rec.start_date = 0L;
			return (EXIT_SUCCESS);
		}
		if (prog_status != ENTRY && 
			local_rec.start_date > local_rec.end_date)
		{
			print_mess (ML (mlStdMess017)); 
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	/*--------------------
	| Validate End Date. |
	--------------------*/
	if (LCHECK ("end_date"))
	{
		if (dflt_used)
		{
			local_rec.end_date = local_rec.lsystemDate;
			return (EXIT_SUCCESS);
		}
		if (local_rec.start_date > local_rec.end_date)
		{
			print_mess (ML (mlStdMess018));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	if (LCHECK ("customer"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.customer, "N");
			strcpy (local_rec.summary, "S");
			entry_exit = TRUE;
			return (EXIT_SUCCESS);
		}
		/*------------------------------------------------------------------
		| If not by customer (by currency only) allow only summary printing |
		 ------------------------------------------------------------------*/
		if (local_rec.customer [0] == 'N')
		{
			strcpy (local_rec.summary, "S");
			entry_exit = TRUE;
		}
		else
			FLD ("summary") = YES;

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("summary"))
	{
		if (dflt_used)
			strcpy (local_rec.summary, "S");
			
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}


void
ReportHeading (
 int                prnt_no)
{
	char	wk_date [2] [11];

	/*----------------------
	| Open pipe to pformat | 
 	----------------------*/
	if ( (pp = popen ("pformat","w")) == NULL)
		sys_err ("Error in pformat During (POPEN)", 1, PNAME);

	/*---------------------------------
	| Start output to standard print. |
	---------------------------------*/

	fprintf (pp,".START%s<%s>\n", DateToString (comm_rec.dbt_date), PNAME);
	fprintf (pp,".LP%d\n",prnt_no);
	if (local_rec.summary [0] == 'D')
	{
		if (local_rec.customer [0] == 'Y')
			fprintf (pp,".12\n");
		else
			fprintf (pp,".11\n");
	}
	else
		fprintf (pp,".8\n");
	fprintf (pp,".L132\n");
	fprintf (pp,".E%s\n",clip (comm_rec.co_name));
	fprintf (pp,".EFOREIGN CURRENCY EXPOSURE REPORT\n");

	sprintf (wk_date [0], DateToString (local_rec.start_date));
	sprintf (wk_date [1], DateToString (local_rec.end_date));

	if (local_rec.summary [0] == 'D')
		fprintf (pp,".CAll Transactions from %s to %s.\n", wk_date [0],wk_date [1]);
	if (local_rec.customer [0] == 'Y')
		fprintf (pp,".C From Customer : %s %s  TO  Customer %s %s\n",
					local_rec.s_dbt_no,
					clip (local_rec.dbt_name [0]),
					local_rec.e_dbt_no,
					clip (local_rec.dbt_name [1]));

	fprintf (pp,".EAS AT : %s\n", SystemTime ());

	fprintf (pp,".R========================================================");
	fprintf (pp,"===============%s", (envVarDbMcurr) ? "=================================================\n" : "=\n");

	fprintf (pp,"========================================================");
	fprintf (pp,"===============%s", (envVarDbMcurr) ? "=================================================\n" : "=\n");

	if (local_rec.summary [0] == 'D')
	{
		fprintf (pp,"| TRANSACTION | BR | DP |  REF.  |   DATE.  |   DATE.  |");
		fprintf (pp,"    AMOUNT     %s", (envVarDbMcurr) ? "|  BOOK AMOUNT  |  CURR AMOUNT  |  DIFF AMOUNT  |\n" : "|\n");

		fprintf (pp,"|     TYPE    | NO | NO |        |  POSTED  |   TRANS  |");
		fprintf (pp,"               %s", (envVarDbMcurr) ? "|               |               |               |\n" : "|\n");

		fprintf (pp,"|-------------|----|----|--------|----------|----------|");
		fprintf (pp,"---------------%s", (envVarDbMcurr) ? "|---------------|---------------|---------------|\n" : "|\n");

	fprintf (pp,".PI12\n");
	}

}

void
PrintInvoice (void)
{
	double	balance = 0.00;
	double	lcl_bal = 0.00;
	double	nlcl_bal = 0.00;
	double	lcl_diff = 0.00;

	/*----------------------------------------------------
	| for each invoice, print details if dbt - crd <> 0. |
	----------------------------------------------------*/
	balance = (envVarDbNettUsed) ? cuin_rec.amt - cuin_rec.disc 
			      		  : cuin_rec.amt;

	/*-----------------------------------------
	| Set the original booked exchange balance |
	 -----------------------------------------*/
	if (balance != 0.00)
		lcl_bal = balance / cuin_rec.exch_rate;
	else
		lcl_bal = 0.00;

	/*--------------------------------------------------------------
	| Print the new local equivalent balance for envVarDbMcurr transactions |
	| If the exchange rate is fixed at invoice input - leave as is. |
	 --------------------------------------------------------------*/
	if (cuin_rec.er_fixed [0] == 'Y')
	{
		if (balance != 0.00)
			nlcl_bal = balance / pocr_rec.ex1_factor;
		else
			nlcl_bal = 0.00;
	}
	else
	{
		if (balance != 0.00)
			nlcl_bal = balance / cuin_rec.exch_rate;
		else
			nlcl_bal = 0.00;
	}

	lcl_diff = lcl_bal - nlcl_bal;

	if (firstPrint == 0)
	{
		PrintHeading ();
		firstPrint = 1;
	}
	
	if (local_rec.customer [0] == 'Y')
	{
		if (local_rec.summary [0] == 'D')
		{
			fprintf (pp,"|   %7.7s   |", inv_type [atoi (cuin_rec.type) -1]);
			fprintf (pp," %2.2s |", cuin_rec.est);
			fprintf (pp," %2.2s |", cuin_rec.dp);
			fprintf (pp,"%8.8s|", cuin_rec.inv_no);
			fprintf (pp,"%10.10s|", DateToString (cuin_rec.date_of_inv));
			fprintf (pp,"%10.10s|", DateToString (cuin_rec.date_posted));
			if (envVarDbMcurr)
			{
				fprintf (pp,"%14.14s |", comma_fmt (DOLLARS (balance),"NNN,NNN,NNN.NN"));
				fprintf (pp,"%14.14s |",comma_fmt (DOLLARS (lcl_bal),"NNN,NNN,NNN.NN"));
				fprintf (pp,"%14.14s |",comma_fmt (DOLLARS (nlcl_bal),"NNN,NNN,NNN.NN"));
				fprintf (pp,"%14.14s |\n",comma_fmt (DOLLARS (lcl_diff),"NNN,NNN,NNN.NN"));
			}
			else
				fprintf (pp,"%14.14s |\n",comma_fmt (DOLLARS (balance),"NNN,NNN,NNN.NN"));
		}
	}
	cust_amt_total 	+= balance;
	cust_book_total	+= lcl_bal;
	cust_curr_total += nlcl_bal;
	cust_diff_total	+= lcl_diff;

	curr_amt_total 	+= balance;
	curr_book_total	+= lcl_bal;
	curr_curr_total += nlcl_bal;
	curr_diff_total	+= lcl_diff;

	grand_book_total += lcl_bal;
	grand_curr_total += nlcl_bal;
	grand_diff_total += lcl_diff;
}

void
PrintCheque (void)
{
	double	balance = 0.00;
	double	lcl_bal = 0.00;
	double	nlcl_bal = 0.00;
	double	lcl_diff = 0.00;

	/*----------------------------------------------------
	| for each invoice, print details if dbt - crd <> 0. |
	----------------------------------------------------*/
	balance = cuhd_rec.tot_amt_paid - cuhd_rec.disc_given;
	balance *= -1;

	if (balance != 0.00)
		lcl_bal = balance / pocr_rec.ex1_factor;
	else
		lcl_bal = 0.00;

	nlcl_bal = lcl_bal;

	lcl_diff = lcl_bal - nlcl_bal;

	if (all_trans [0] == 'C')
		if (cuhd_rec.date_payment < start_date)
			return;

	if (firstPrint == 0)
	{
		PrintHeading ();
		firstPrint = 1;
	}

	
	fprintf (pp,"|   %7.7s   |", (cuhd_rec.type [0] == '1') 
						? inv_type [3]
						: inv_type [0]);
	fprintf (pp," %2.2s |", "  ");
	fprintf (pp," %2.2s |", "  ");
	fprintf (pp,"%8.8s|", cuhd_rec.receipt_no);
	fprintf (pp,"%10.10s|", DateToString (cuhd_rec.date_payment));
	fprintf (pp,"%10.10s|", DateToString (cuhd_rec.date_posted));
	if (envVarDbMcurr)
	{
		fprintf (pp,"%14.14s |", comma_fmt (DOLLARS (balance),"NNN,NNN,NNN.NN"));
		fprintf (pp,"%14.14s |",comma_fmt (DOLLARS (lcl_bal),"NNN,NNN,NNN.NN"));
		fprintf (pp,"%14.14s |",comma_fmt (DOLLARS (nlcl_bal),"NNN,NNN,NNN.NN"));
		fprintf (pp,"%14.14s |\n",comma_fmt (DOLLARS (lcl_diff),"NNN,NNN,NNN.NN"));
	}
	else
	fprintf (pp,"%14.14s |\n",comma_fmt (DOLLARS (balance),"NNN,NNN,NNN.NN"));
	cust_amt_total 	+= balance;
	cust_book_total	+= lcl_bal;
	cust_curr_total += nlcl_bal;
	cust_diff_total	+= lcl_diff;

	curr_amt_total 	+= balance;
	curr_book_total	+= lcl_bal;
	curr_curr_total += nlcl_bal;
	curr_diff_total	+= lcl_diff;

	grand_book_total += lcl_bal;
	grand_curr_total += nlcl_bal;
	grand_diff_total += lcl_diff;
}
  
/*=================================================================
| Heading concerns itself with clearing the screen, painting the  |
| screen overlay in preparation for input                         |
=================================================================*/
int
heading (
 int                scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();

		rv_pr (ML (mlDbMess052), 20,0,1);

		move (0,1);
		line (80);

		move (1,input_row);
		if (scn == 1)
		{
			box (0,2,80,9);
			move (1,5);
			line (79);
			move (1,8);
			line (79);
			move (1,10);
			line (79);
		}

		move (0,19);
		line (80);
		print_at (20,0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
		print_at (21,0, ML (mlStdMess039), comm_rec.est_no,comm_rec.est_name);
		move (0,22);
		line (80);
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}

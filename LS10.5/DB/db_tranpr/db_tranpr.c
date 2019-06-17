/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: db_tranpr.c,v 5.3 2001/11/27 08:11:35 scott Exp $
|  Program Name  : (db_tranpr.c)
|  Program Desc  : (Detailed Customer Transaction Print)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
| $Log: db_tranpr.c,v $
| Revision 5.3  2001/11/27 08:11:35  scott
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_tranpr.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_tranpr/db_tranpr.c,v 5.3 2001/11/27 08:11:35 scott Exp $";

#include 	<ml_db_mess.h>
#include 	<ml_std_mess.h>
#include 	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>

#define		CF(x)	comma_fmt (DOLLARS (x), "NNN,NNN,NNN.NN")

FILE	*pp;
	
#include	"schema"

struct commRecord	comm_rec;
struct cuinRecord	cuin_rec;
struct cuhdRecord	cuhd_rec;
struct cumrRecord	cumr_rec;
struct cumrRecord	cumr2_rec;
struct pocrRecord	pocr_rec;

	char	*data = "data",
			*cumr2 = "cumr2";

	long	startDate = 0L;

	double	grandTotal 		= 0.00, 
			fgnCustTotal 	= 0.00,
			locCustTotal 	= 0.00;
	
	static char *inv_type [] = {
			"INVOICE",
			"CREDIT ",
			"JOURNAL",
			"PAYMENT"
	};

	int		firstPrint 		= 0,
			printerNo 		= 1,
			envDbNettUsed 	= TRUE,
			envDbMcurr 		= FALSE,
			envDbCo			= 0,
			dtls_cnt		= 0;

	extern	int	TruePosition;

/*
 * Local & Screen Structures.
 */
struct {
	char	dummy [11];
	char	startCustomerNo [7];
	char	endCustomerNo [7];
	char	customerName [2][41];
	long	startDate;
	long	endDate;
	long	lsystemDate;
	int		lpno;
} local_rec;

static	struct	var	vars []	={	

	{1, LIN, "startCustomerNo", 3, 2, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", " ", "Start Customer   ", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.startCustomerNo}, 
	{1, LIN, "startCustomerName", 3, 34, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", " ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.customerName [0]}, 
	{1, LIN, "endCustomerNo", 4, 2, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", " ", "End Customer     ", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.endCustomerNo}, 
	{1, LIN, "endCustomrName", 4, 34, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", " ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.customerName [1]}, 
	{1, LIN, "st_date",	 6, 2, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "0", "Start Date       ", "Enter Date for Starting Transaction - Default is start ",
		YES, NO,  JUSTLEFT, "", "", (char *) &local_rec.startDate},
	{1, LIN, "endDate",	 7, 2, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", " ", "End   Date       ", "Enter Date for Ending Transaction - Default is Today ",
		YES, NO,  JUSTLEFT, "", "", (char *) &local_rec.endDate},
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", ""}, 
};

#include <FindCumr.h>

/*
 * Local Function Prototypes.
 */
void 	ProcessFile 		(void);
void 	PrintHeading 		(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
void 	shutdown_prog 		(void);
void 	ReportHeading 		(int);
void 	PrintInvoice 		(void);
void 	PrintCheque 		(void);
int 	heading 			(int);
int 	spec_valid 			(int);

int
main (
	int		argc,
	char	*argv [])
{
	char	*sptr;

	TruePosition	=	TRUE;

	local_rec.lsystemDate = TodaysDate ();

	sptr = chk_env ("DB_NETT_USED");
	envDbNettUsed = (sptr == (char *)0) ? TRUE : atoi (sptr);

	sptr = chk_env ("DB_MCURR");
	envDbMcurr = (sptr == (char *)0) ? TRUE : atoi (sptr);

 	envDbCo = atoi (get_env ("DB_CO"));

	if (argc != 2)
	{
		print_at (0,0,mlStdMess036,argv [0]);
        return (EXIT_FAILURE);
	}
	printerNo = atoi (argv [1]);

	SETUP_SCR (vars);

	init_scr 	();			/*  sets terminal from termcap	*/
	set_tty 	();         /*  get into raw mode		*/
	set_masks 	();			/*  setup print using masks	*/
	init_vars 	(1);		/*  set default values		*/

	OpenDB ();


	while (!prog_exit)
	{
		/*
		 * Reset control flags
		 */
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;
		init_ok 	= TRUE;
		init_vars (1);	

		/*
		 * Edit screen 1 linear input.
		 */
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
			dsp_screen ("Printing Customer Transaction Listing.",
					comm_rec.co_no,comm_rec.co_name);

			ReportHeading (printerNo);

			strcpy (cumr_rec.co_no, comm_rec.co_no);
			strcpy (cumr_rec.est_no, (envDbCo) ? comm_rec.est_no : " 0");

			startDate	=	MonthStart (comm_rec.dbt_date);

			abc_selfield (cumr, "cumr_id_no");
			strcpy (cumr_rec.co_no, comm_rec.co_no);
			strcpy (cumr_rec.est_no, (envDbCo) ? comm_rec.est_no : " 0");
			strcpy (cumr_rec.dbt_no, local_rec.startCustomerNo);
			cc = find_rec (cumr, &cumr_rec, GTEQ, "r");
			while (!cc && !strcmp (cumr_rec.co_no, comm_rec.co_no))
			{
				if (envDbCo && strcmp (cumr_rec.est_no, comm_rec.est_no) != 0)
				{
					cc = find_rec (cumr, &cumr_rec, NEXT, "r");
					continue;
				}

				/*
				 * Don't process child customer.
				 */
				if (cumr_rec.ho_dbt_hash > 0L)
				{
					cc = find_rec (cumr, &cumr_rec, NEXT, "r");
					continue;
				}
				dsp_process ("Customer : ",cumr_rec.dbt_no);
				ProcessFile ();
				if (!strcmp (cumr_rec.dbt_no, local_rec.endCustomerNo))
				{
					abc_selfield (cumr, "cumr_id_no2");
					break;
				}
				cc = find_rec (cumr, &cumr_rec, NEXT, "r");
			}
			abc_selfield (cumr, "cumr_id_no2");

			/*
			 * print final totals.
			 */
			fprintf (pp,".LRP4\n");
			fprintf (pp,"|=============|====|====|========|==========|==========|");
			fprintf (pp,"===============%s", (envDbMcurr) ? "|===============|\n" 
													: "|\n");

			fprintf (pp,"|             |    |    |       ** GRAND TOTALS **     |");
			if (envDbMcurr)
				fprintf (pp,"               |%14.14s |\n", CF (grandTotal));
			else	
				fprintf (pp,"%14.14s |\n", CF (grandTotal));

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
	if (cc)
		pocr_rec.ex1_factor	=	1.00;

	cuin_rec.date_of_inv	= local_rec.startDate;
	cuin_rec.ho_hash 		= cumr_rec.hhcu_hash;
	strcpy (cuin_rec.est, "  ");
	cc = find_rec (cuin, &cuin_rec, GTEQ, "r");
	while (!cc && cuin_rec.ho_hash == cumr_rec.hhcu_hash &&
			(cuin_rec.date_of_inv >= local_rec.startDate &&
			cuin_rec.date_of_inv <= local_rec.endDate))
	{
		PrintInvoice ();
		cc = find_rec (cuin, &cuin_rec, NEXT, "r");
	}

	cuhd_rec.hhcu_hash = cumr_rec.hhcu_hash;
	strcpy (cuhd_rec.receipt_no, "      ");
	cuhd_rec.index_date	=	0L;
	cc = find_rec (cuhd, &cuhd_rec, GTEQ, "r");
	while (!cc && cumr_rec.hhcu_hash == cuhd_rec.hhcu_hash) 
	{
		PrintCheque ();
		cc = find_rec (cuhd, &cuhd_rec, NEXT, "r");
	}

	if (firstPrint)
	{
		fprintf (pp,"|             |    |    |        |          |  TOTAL   |");
		if (envDbMcurr)
		{
			fprintf (pp,"%14.14s| ", CF (fgnCustTotal));
			fprintf (pp,"%14.14s |\n", CF (locCustTotal));
		}
		else
		{
			fprintf (pp,"%14.14s |\n", CF (fgnCustTotal));
		}
		fprintf (pp,"|-------------|----|----|--------|----------|----------|");
		fprintf (pp,"---------------%s", (envDbMcurr) ? "|---------------|\n" : "|\n");
	}
	fgnCustTotal = 0;
	locCustTotal = 0;

}

void
PrintHeading (void)
{
	if (envDbMcurr)
	{
		fprintf (pp,"| %s (%s)        %s                         |\n",
			cumr_rec.dbt_no, cumr_rec.dbt_name, cumr_rec.curr_code);
	}
	else
	{
		fprintf (pp,"| %s (%s)                    |\n",
			cumr_rec.dbt_no, cumr_rec.dbt_name);
	}
}
/*======================
| Open database Files. |
======================*/
void
OpenDB (void)
{
	abc_dbopen (data);

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
	if (LCHECK ("startCustomerNo"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.customerName [0], ML ("Start Customer"));
			strcpy (local_rec.startCustomerNo,"      ");
			DSP_FLD ("startCustomerName");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			CumrSearch 
			(
				comm_rec.co_no, (envDbCo) ? comm_rec.est_no : " 0",
				temp_str
			);
			return (EXIT_SUCCESS);
		}  
		
        if (prog_status != ENTRY &&
		    strcmp (local_rec.startCustomerNo,local_rec.endCustomerNo) > 0)
	 	{ 
			print_mess (ML (mlStdMess017));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		abc_selfield (cumr, "cumr_id_no");
        strcpy (cumr_rec.co_no,comm_rec.co_no);
		strcpy (cumr_rec.est_no, (envDbCo) ? comm_rec.est_no : " 0");
        strcpy (cumr_rec.dbt_no,pad_num (local_rec.startCustomerNo));
		cc = find_rec (cumr,&cumr_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess (ML (mlStdMess021));
			abc_selfield (cumr, "cumr_id_no2");
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.customerName [0], cumr_rec.dbt_name);
		DSP_FLD ("startCustomerName");
		abc_selfield (cumr, "cumr_id_no2");
		return (EXIT_SUCCESS);
	}

	/*----------------------+
	| Validate End Customer |
	+----------------------*/
	if (LCHECK ("endCustomerNo"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.customerName [1], ML ("End Customer"));
			strcpy (local_rec.endCustomerNo,"~~~~~~");
			DSP_FLD ("endCustomrName");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			CumrSearch 
			(
				comm_rec.co_no,
				(envDbCo) ? comm_rec.est_no : " 0",
				temp_str
			);
			return (EXIT_SUCCESS);
		}
		
		abc_selfield (cumr, "cumr_id_no");
        strcpy (cumr_rec.co_no,comm_rec.co_no);
		strcpy (cumr_rec.est_no, (envDbCo) ? comm_rec.est_no : " 0");
        strcpy (cumr_rec.dbt_no,pad_num (local_rec.endCustomerNo));
	    cc = find_rec (cumr,&cumr_rec,COMPARISON,"r");
		if (cc)
		{
			print_mess (ML (mlStdMess021));
			abc_selfield (cumr, "cumr_id_no2");
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		if (strcmp (local_rec.startCustomerNo,local_rec.endCustomerNo) > 0)
		{
			errmess (ML (mlStdMess018));
			sleep (sleepTime);
			abc_selfield (cumr, "cumr_id_no2");
			return (EXIT_FAILURE);
		}
		
		strcpy (local_rec.customerName [1], cumr_rec.dbt_name);
		DSP_FLD ("endCustomrName");
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
			local_rec.startDate = 0L;
			return (EXIT_SUCCESS);
		}
		if (prog_status != ENTRY && 
			local_rec.startDate > local_rec.endDate)
		{
			print_mess (ML (mlStdMess019));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	/*--------------------
	| Validate End Date. |
	--------------------*/
	if (LCHECK ("endDate"))
	{
		if (dflt_used)
		{
			local_rec.endDate = local_rec.lsystemDate;
			return (EXIT_SUCCESS);
		}
		if (local_rec.startDate > local_rec.endDate)
		{
			print_mess (ML (mlStdMess019));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

void
ReportHeading (
	int		prnt_no)
{
	char	wkDate [2][11];

	/*----------------------
	| Open pipe to pformat | 
 	----------------------*/
	if ((pp = popen ("pformat","w")) == NULL)
		sys_err ("Error in pformat During (POPEN)", 1, PNAME);

	/*---------------------------------
	| Start output to standard print. |
	---------------------------------*/
	fprintf (pp,".START%s<%s>\n", DateToString (comm_rec.dbt_date), PNAME);
	fprintf (pp,".LP%d\n",prnt_no);
	fprintf (pp,".12\n");
	fprintf (pp,".L110\n");
	fprintf (pp,".E%s\n",clip (comm_rec.co_name));
	fprintf (pp,".ETRANSACTION LISTING\n");

	sprintf (wkDate [0], DateToString (local_rec.startDate));
	sprintf (wkDate [1], DateToString (local_rec.endDate));

	fprintf (pp,".CAll Transactions from %s to %s.\n", wkDate [0],wkDate [1]);

	fprintf (pp,".C From Customer : %s %s  TO  Customer %s %s\n",
					local_rec.startCustomerNo,
					clip (local_rec.customerName [0]),
					local_rec.endCustomerNo,
					clip (local_rec.customerName [1]));
	fprintf (pp,".EAS AT : %s\n", SystemTime ());

	fprintf (pp,".R========================================================");
	fprintf (pp,"===============%s", (envDbMcurr) ? "=================\n" : "=\n");

	fprintf (pp,"========================================================");
	fprintf (pp,"===============%s", (envDbMcurr) ? "=================\n" : "=\n");

	fprintf (pp,"| TRANSACTION | BR | DP |  REF.  |   DATE.  |   DATE.  |");
	fprintf (pp,"    AMOUNT     %s", (envDbMcurr) ? "| LOCAL  AMOUNT |\n" : "|\n");

	fprintf (pp,"|     TYPE    | NO | NO |        |  POSTED  |   TRANS  |");
	fprintf (pp,"               %s", (envDbMcurr) ? "|               |\n" : "|\n");

	fprintf (pp,"|-------------|----|----|--------|----------|----------|");
	fprintf (pp,"---------------%s", (envDbMcurr) ? "|---------------|\n" : "|\n");

	fprintf (pp,".PI12\n");

}

void
PrintInvoice (void)
{
	double	fgnBalance = 0.00,
			locBalance = 0.00;

	/*
	 * for each invoice, print details if dbt - crd <> 0.
	 */
	fgnBalance = (envDbNettUsed) ? cuin_rec.amt - cuin_rec.disc : cuin_rec.amt;

	if (fgnBalance != 0.00)
		locBalance = fgnBalance / pocr_rec.ex1_factor;
	else
		locBalance = 0.00;

	if (firstPrint == 0)
	{
		PrintHeading ();
		firstPrint = 1;
	}
	
	fprintf (pp,"|   %7.7s   |", inv_type [atoi (cuin_rec.type) -1]);
	fprintf (pp," %2.2s |", cuin_rec.est);
	fprintf (pp," %2.2s |", cuin_rec.dp);
	fprintf (pp,"%8.8s|", cuin_rec.inv_no);
	fprintf (pp,"%10.10s|",DateToString (cuin_rec.date_posted));
	fprintf (pp,"%10.10s|",DateToString (cuin_rec.date_of_inv));
	if (envDbMcurr)
	{
		fprintf (pp,"%14.14s |", CF (fgnBalance));
		fprintf (pp,"%14.14s |\n",CF (locBalance));
	}
	else
		fprintf (pp,"%14.14s |\n",CF (fgnBalance));
	fgnCustTotal 		+= fgnBalance;
	locCustTotal 	+= locBalance;
	grandTotal 	+= fgnBalance;
}

void
PrintCheque (void)
{
	double	fgnBalance = 0.00,
			locBalance = 0.00;

	/*
	 * for each invoice, print details if dbt - crd <> 0.
	 */
	fgnBalance = cuhd_rec.tot_amt_paid - cuhd_rec.disc_given;
	fgnBalance *= -1;

	if (fgnBalance != 0.00)
		locBalance = fgnBalance / pocr_rec.ex1_factor;
	else
		locBalance = 0.00;

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
	fprintf (pp,"%10.10s|",DateToString (cuhd_rec.date_posted));
	fprintf (pp,"%10.10s|", DateToString (cuhd_rec.date_payment));
	if (envDbMcurr)
	{
		fprintf (pp,"%14.14s |", CF (fgnBalance));
		fprintf (pp,"%14.14s |\n",CF (locBalance));
	}
	else
		fprintf (pp,"%14.14s |\n",CF (fgnBalance));
	fgnCustTotal += fgnBalance;
	locCustTotal += locBalance;
	grandTotal += fgnBalance;
}
  
/*
 * Heading concerns itself with clearing the screen, painting the 
 * screen overlay in preparation for input                       
 */
int
heading (
 int                scn)
{
	if (restart) 
    	return (EXIT_FAILURE);

	if (scn != cur_screen)
		scn_set (scn);
	clear ();

	rv_pr (ML (mlDbMess052),20,0,1);

	line_at (1,0,80);

	move (1,input_row);
	if (scn == 1)
	{
		box (0,2,80,5);
		line_at (5,1,79);
	}

	line_at (19,0,80);
	strcpy (err_str,ML (mlStdMess038));
	print_at (20,0,err_str,comm_rec.co_no, comm_rec.co_name);
	strcpy (err_str,ML (mlStdMess039));
	print_at (21,0,err_str,comm_rec.est_no,comm_rec.est_name);
	line_at (22,0,80);
	line_cnt = 0;
	scn_write (scn);
    return (EXIT_FAILURE);
}


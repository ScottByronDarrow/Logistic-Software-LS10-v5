/*=====================================================================
|  Copyright (C) 1996 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( fe_exp_rep.c   )                                 |
|  Program Desc  : ( Print Currency Exposure Report               )   |
|                  (                                              )   |
|---------------------------------------------------------------------|
|  Access files  :  comm, cumr, cuin, cudt, cuhd, cucc,     ,         |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A ,     ,     ,     ,     ,     ,     ,         |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Author        : Dirk Heinsius   | Date Written 17/08/94            |
|---------------------------------------------------------------------|
|  Date Modified : (30/11/95)      | Modified  by  : Scott B Darrow.  |
|  Date Modified : (11/09/97)      | Modified  by  : Leah Manibog.    |
|  Date Modified : (14/10/97)      | Modified  by  : Leah Manibog.    |
|  Date Modified : (31/10/1997)    | Modified by : Campbell Mander.   |
|                                                                     |
|  Comments      :                                                    |
|  (30/11/95)    : PDL - Updated for new general ledger interface.    |
|                :       Program will work with 9 and 16 char accounts|
|  (11/09/97)    : Updated for Multilingual Conversion and    	      |
|  (14/10/97)    : Fixed MLDB error.					    	      |
|  (31/10/1997)  : SEL. 9.9.3 Update for Multi-lingual, Y2K and 8     |
|                : character invoice numbers.                         |
|                :                                                    |
|                                                                     |
| $Log: fe_exp_rep.c,v $
| Revision 5.3  2002/07/17 09:57:11  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.2  2001/08/09 09:13:15  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:24:56  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:05:59  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:26:32  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:14:40  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 08:56:17  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.18  1999/12/06 01:46:59  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.17  1999/11/17 06:40:05  scott
| Updated to remove PNAME as now available with Ctrl-P.
|
| Revision 1.16  1999/11/08 05:01:00  scott
| Updated due to warnings when using -Wall compile flag.
|
| Revision 1.15  1999/10/20 02:06:47  nz
| Updated for final changes on date routines.
|
| Revision 1.14  1999/10/16 04:56:26  nz
| Updated for pjulmdy and pmdyjul routines.
|
| Revision 1.13  1999/10/01 07:48:38  scott
| Updated for standard function calls.
|
| Revision 1.12  1999/09/29 10:10:40  scott
| Updated to be consistant on function names.
|
| Revision 1.11  1999/09/17 07:26:29  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.10  1999/09/16 07:18:58  scott
| Updated from Ansi Project
|
| Revision 1.8  1999/06/15 00:12:15  scott
| Updated to add log file.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: fe_exp_rep.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/FE/fe_exp_rep/fe_exp_rep.c,v 5.3 2002/07/17 09:57:11 scott Exp $";

#include 	<ml_std_mess.h>
#include 	<ml_fe_mess.h>
#include 	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<get_lpno.h>

#define	DTLS	4000	/* Total no. of details. */

#define	LCL		0
#define	FGN		1

			/* Not used at present but left in code for  */
		 	/* possible use in future. see also LCL, FGN */
#define	LCL_CURR	(local_rec.disp_curr[0] == 'L' || !MCURR)

FILE	*pp;
FILE	*fsort;

	/*====================================
	| file comm	{System Common file} |
	====================================*/
	struct dbview comm_list[] ={
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_dbt_date"}
	};

	int comm_no_fields = 6;

	struct {
		int		term;
		char	tco_no[3];
		char	tco_name[41];
		char	test_no[3];
		char	test_name[41];
		long	tdbt_date;
	} comm_rec;

	/*================================================
	| Customer Invoice Accounting Invoice/Credit file |
	================================================*/
	struct dbview cuin_list[] ={
		{"cuin_hhcu_hash"},
		{"cuin_ho_hash"},
		{"cuin_hhci_hash"},
		{"cuin_type"},
		{"cuin_est"},
		{"cuin_dp"},
		{"cuin_inv_no"},
		{"cuin_date_of_inv"},
		{"cuin_pay_terms"},
		{"cuin_due_date"},
		{"cuin_currency"},
		{"cuin_exch_rate"},
		{"cuin_er_fixed"},
		{"cuin_disc"},
		{"cuin_amt"},
		{"cuin_stat_flag"},
	};

	int		cuin_no_fields = 16;

	struct	{
		long	hhcu_hash;
		long	ho_hash;
		long	hhci_hash;
		char	type[2];
		char	est[3];
		char	dp[3];
		char	inv_no[9];
		long	doi;
		char	pay_terms[4];
		long	due_date;
		char	currency[4];
		double	exch_rate;
		char	er_fixed[2];
		double	disc;	/* money */
		double	amt;	/* money */
		char	stat_flag[2];
	} cuin_rec;

	/*=====================================
	| Customer Payments Header File Record |
	=====================================*/
	struct dbview cuhd_list[] ={
		{"cuhd_hhcu_hash"},
		{"cuhd_hhcp_hash"},
	};

	int		cuhd_no_fields = 2;

	struct	{
		long	hhcu_hash;
		long	hhcp_hash;
	} cuhd_rec;

	/*================================
	| Customer Payments Detail Record |
	================================*/
	struct dbview cudt_list[] ={
		{"cudt_hhcp_hash"},
		{"cudt_hhci_hash"},
		{"cudt_amt_paid_inv"},
		{"cudt_loc_paid_inv"},
	};

	int		cudt_no_fields = 4;

	struct	{
		long	hhcp_hash;
		long	hhci_hash;
		double	amt_paid_inv;	/* money */
		double	loc_paid_inv;	/* money */
	} cudt_rec;

	/*====================================
	| file cumr {Customer Master Record} |
	====================================*/
	struct dbview cumr_list[] ={
		{"cumr_co_no"},
		{"cumr_est_no"},
		{"cumr_department"},
		{"cumr_dbt_no"},
		{"cumr_hhcu_hash"},
		{"cumr_dbt_name"},
		{"cumr_dbt_acronym"},
		{"cumr_curr_code"},
		{"cumr_payment_flag"},
		{"cumr_credit_limit"},
		{"cumr_ho_dbt_hash"},
		{"cumr_date_lastpay"},
		{"cumr_amt_lastpay"},
		{"cumr_bo_current"},
		{"cumr_bo_per1"},
		{"cumr_bo_per2"},
		{"cumr_bo_per3"},
		{"cumr_bo_per4"},
		{"cumr_bo_fwd"},
		{"cumr_stat_flag"}
	};

	int cumr_no_fields = 20;

	struct {
		char	co_no[3];
		char	est_no[3];
		char	dp_no[3];
		char	dbt_no[7];
		long	hhcu_hash;
		char	name[41];
		char	acronym[10];
		char	curr_code[4];
		int		payment_flag;
		double	credit_limit;
		long	ho_dbt_hash;
		long	date_lastpay;
		double	amt_lastpay;
		double	bo_per[6];
		char	stat_flag[2];
	} cumr_rec, cumr2_rec;

	/*=================================
	| Customer Credit Control Record. |
	=================================*/
	struct dbview cucc_list[] ={
		{"cucc_hhcu_hash"},
		{"cucc_record_no"},
		{"cucc_con_person"},
		{"cucc_comment"},
		{"cucc_cont_date"},
		{"cucc_hold_flag"},
		{"cucc_hold_ref"}
	};

	int cucc_no_fields = 7;

	struct {
		long	hhcu_hash;
		long	record_no;
		char	con_person[21];
		char	comment[81];
		long	cont_date;
		char	hold_flag[2];
		char	hold_ref[9];
	} cucc_rec;

	/*======================
	| Currency File Record |
	======================*/
	struct dbview pocr_list[] ={
		{"pocr_co_no"},
		{"pocr_code"},
		{"pocr_description"},
		{"pocr_ldate_up"},
		{"pocr_stat_flag"},
	};

	int		pocr_no_fields = 5;

	struct	{
		char	co_no[3];
		char	code[4];
		char	description[41];
		long	ldate_up;
		char	stat_flag[2];
	} pocr_rec;

	/*==================================
	| Forward Exchange Assignment File |
	==================================*/
	struct dbview feln_list [] =
	{
		{"feln_hhfe_hash"},
		{"feln_index_by"},
		{"feln_index_hash"},
		{"feln_value"}
	};

	int	feln_no_fields = 4;

	struct tag_felnRecord
	{
		long	hhfe_hash;
		char	index_by[2];
		long	index_hash;
		double	value;		/* money */
	} feln_rec;

	/*===================================
	| Forward Exchange Transaction File |
	===================================*/
	struct dbview fetr_list [] =
	{
		{"fetr_hhfe_hash"},
		{"fetr_index_by"},
		{"fetr_index_hash"},
		{"fetr_hhcp_hash"},
		{"fetr_value"}			/* money */
	};

	int	fetr_no_fields = 5;

	struct tag_fetrRecord
	{
		long	hhfe_hash;
		char	index_by [2];
		long	index_hash;
		long	hhcp_hash;
		double	value;
	} fetr_rec, fetr2_rec;

	char	*data  = "data",
			*comm  = "comm",
			*cuin  = "cuin",
			*cuhd  = "cuhd",
			*cudt  = "cudt",
			*cucc  = "cucc",
			*cumr  = "cumr",
			*cumr2 = "cumr2",
			*feln  = "feln",
			*fetr  = "fetr",
			*pocr  = "pocr";

/*=====================================================================
| The structures 'dtls' are initialised in function 'get_cheq'        |
| the number of details is stored in external variable 'dtls_cnt'.    |
=====================================================================*/
struct	{                       /*===========================*/
	long	hhci_hash;			/* detail invoice reference. */
	double	inv_amt;			/* detail invoice amount.	 */
	double	lcl_amt;			/* Local invoice amount.	 */
	double	exch_var;			/* Local exchange variation. */
} dtls[DTLS];           		/*===========================*/

	int		envDbCo,
	   		envDbFind,
	   		dtls_cnt,
	   		first_pr = 0,
	   		topOfPage = TRUE,
	   		first_curr = TRUE,
	   		lp_no = 1,
	   		line_printed,
	   		DB_NETT = TRUE,
	   		FE_INSTALL = FALSE,
	   		MCURR = FALSE;

	double	atof(const char *),
	      	inv_tot,
	      	lcl_tot;

	double	g_tot[2],
			g_tots = 0.00,
			total[2][6],
			totals[6]  = {0,0,0,0,0,0},
			debits[6]  = {0,0,0,0,0,0},
			credits[6] = {0,0,0,0,0,0};
	
	double	percent[6];

	long	mend_date = 0L;
	long	lsystemDate = 0L;

	static char *inv_type[] = {
		"INV",
		"CRD",
		"JNL",
		"CHQ",
		"JNL",
		"JNL",
	};

	char	branchNo[3],
	    	systemDate[11],
	    	pay_date[11],
	    	prev_crrcy[4],
	    	curr_crrcy[4];

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy[11];
	int		lpno;
	char	lp_str[3];
	char 	back[5];
	char	onite[5];
	char	disp_curr[2];
	char	disp_desc[41];
	char	curr_code[4];
	char	curr_desc[41];
	char	cust_code[7];
	char	cust_desc[41];
} local_rec;

static	struct	var	vars[] =
{
	{1, LIN, "curr_code",	4, 17, CHARTYPE,
		"UUU", "          ",
		" ", " ", "Currency : ", "Enter Start Currency",
		YES, NO,  JUSTLEFT, "", "", local_rec.curr_code},
	{1, LIN, "curr_desc",	4, 24, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.curr_desc},

	{1, LIN, "cust_code",	5, 17, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", "Customer : ", "Enter Start Currency",
		YES, NO,  JUSTLEFT, "", "", local_rec.cust_code},
	{1, LIN, "cust_desc",	5, 24, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.cust_desc},

	{1, LIN, "disp_curr",	6, 17, CHARTYPE,
		"U", "          ",
		" ", "F", "Display  : ", "Display amounts in Local or Foreign currency",
		ND, NO,  JUSTLEFT, "FL", "", local_rec.disp_curr},
	{1, LIN, "disp_desc",	6, 24, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		ND, NO,  JUSTLEFT, "", "", local_rec.disp_desc},

	{1, LIN, "lpno",	 	7, 17, INTTYPE,
		"NN", "          ",
		" ", "1", "Printer Number", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.lpno},
	{1, LIN, "back",		8, 17, CHARTYPE,
		"U", "          ",
		" ", "N", "Background ", " ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.back},
	{1, LIN, "onight",	9, 17, CHARTYPE,
		"U", "          ",
		" ", "N", "Overnight ", " ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.onite},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};


/*=====================
| Function Prototypes |
=====================*/
void run_prog (char *prog_name);
int spec_valid (int field);
void srch_cumr (char *key_val);
void SrchPocr (char *key_val);
void proc_cumr (void);
void proc_file (void);
void prnt_head (void);
void OpenDB (void);
void CloseDB (void);
void shutdown_prog (void);
void head (int prnt_no);
void prnt_line (void);
void chk_break (void);
void curr_header (char *crrcy);
void prnt_tot (void);
void get_cheq (void);
void get_pdate (void);
int heading (int scn);
int pay_per (char *p_terms, long due_date, long cur_date);


/*=========================
| Main Processing Routine |
=========================*/
int
main (
 int argc, 
 char * argv[])
{
	char	*sptr;

 	envDbCo = atoi (get_env ("DB_CO"));
 	envDbFind  = atoi (get_env ("DB_FIND"));

	if (argc != 1 && argc != 4)
	{

/*		printf ("Usage db_ptermprn <lpno>\007\n");
		printf ("                  <currency>\n");
		printf ("                  <customer> \n\r");
*/
        return (EXIT_FAILURE);
	}

	SETUP_SCR (vars);

	sptr = chk_env ("DB_NETT_USED");
	DB_NETT = (sptr == (char *)0) ? TRUE : atoi(sptr);

	sptr = chk_env ("DB_MCURR");
	MCURR = (sptr == (char *)0) ? FALSE : atoi(sptr);

	sptr = chk_env ("FE_INSTALL");
	FE_INSTALL = (sptr == (char *)0) ? TRUE : atoi(sptr);

	OpenDB ();

	/*-------------------------------
	| Read common terminal record . |
	-------------------------------*/
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);

	strcpy (systemDate, DateToString (TodaysDate()));
	lsystemDate = TodaysDate ();

	strcpy(branchNo, (envDbCo) ? comm_rec.test_no : " 0");

	if ( argc == 4 )
	{
		lp_no = atoi(argv[1]);
		sprintf (local_rec.curr_code, "%-3.3s", argv[2]);
		sprintf (local_rec.cust_code, "%-6.6s", argv[3]);
		
		head (lp_no);
		dsp_screen ("Printing Currency Exposure Report.",
					 comm_rec.tco_no, comm_rec.tco_name);
		proc_cumr ();

		fprintf (pp, ".EOF\n");
		pclose  (pp);
		shutdown_prog ();
		return (EXIT_SUCCESS);
	}

	/*---------------------------
	| Setup required parameters |
	---------------------------*/
	init_scr ();			/*  sets terminal from termcap	*/
	set_tty ();             /*  get into raw mode		*/
	set_masks ();			/*  setup print using masks	*/
	init_vars (1);			/*  set default values		*/

	if (!MCURR)
	{
		FLD ("curr_code") = ND;
		FLD ("curr_desc") = ND;
		vars[label("lpno")].row = 6;
		vars[label("back")].row = 7;
		vars[label("onight")].row = 8;
	}

	while (prog_exit == 0)
	{
		/*---------------------
		| Reset control flags |
		---------------------*/
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		search_ok = 1;
		init_vars (1);		/*  set default values		*/

		/*----------------------------
		| Entry screen 1 linear input |
		----------------------------*/
		heading (1);
		entry (1);
		if (restart || prog_exit)
			continue;

		/*----------------------------
		| Edit screen 1 linear input |
		----------------------------*/
		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
			continue;

		strcpy(err_str, ML(mlFeMess029));
		run_prog (argv[0]);
		prog_exit = 1;
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}


void
run_prog (
 char *prog_name)
{
	sprintf(local_rec.lp_str,"%d",local_rec.lpno);
	
	rset_tty ();

	clear ();

	strcpy(err_str, ML(mlStdMess035));
	print_at (0,0, err_str);

	fflush (stdout);
	shutdown_prog ();

	if (local_rec.onite[0] == 'Y')
	{
		if (fork() == 0)
			execlp("ONIGHT",
					"ONIGHT",
					prog_name,
					local_rec.lp_str,
					local_rec.curr_code,
					local_rec.cust_code,
					err_str, (char *)0);
					/*"Print Currency Exposure Report.", (char *)0);*/
		else
            return;
	}
    else if (local_rec.back[0] == 'Y')
	{
		if (fork() == 0)
			execlp(prog_name,
					prog_name,
					local_rec.lp_str,
					local_rec.curr_code,
					local_rec.cust_code,
					(char *)0);
			else
                return;
	}
	else 
	{
		execlp(prog_name,
				prog_name,
				local_rec.lp_str,
				local_rec.curr_code,
				local_rec.cust_code,
				(char *)0);
	}
}


int
spec_valid (
 int field)
{
	if (LCHECK ("lpno"))
	{
		if (SRCH_KEY)
		{
			local_rec.lpno = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if (!valid_lp (local_rec.lpno))
		{
			/*print_mess ("\007 Invalid Printer. ");*/
			print_mess (ML(mlStdMess020));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("back"))
	{
		strcpy (local_rec.back, (local_rec.back[0] == 'Y') ? "Y(es" : "N(o ");
		DSP_FLD ("back");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("onight"))
	{
		strcpy (local_rec.onite, (local_rec.onite[0] == 'Y') ? "Y(es" : "N(o ");
		DSP_FLD ("onight");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("curr_code"))
	{
		if (dflt_used)
		{
			strcpy  (local_rec.curr_code, "ALL");
			sprintf (local_rec.curr_desc, "%-40.40s", "All Currencies");
			DSP_FLD ("curr_code");
			DSP_FLD ("curr_desc");
			return  (0);
		}

		if (SRCH_KEY)
		{
			SrchPocr (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy  (pocr_rec.co_no, comm_rec.tco_no);
		sprintf (pocr_rec.code,  "%-3.3s", local_rec.curr_code);
		cc = find_rec (pocr, &pocr_rec, EQUAL, "r");
		if (cc)
		{
			/*print_mess ("\007 Currency Code Not Found ");*/
			print_mess (ML(mlStdMess040));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.curr_desc, "%-40.40s", pocr_rec.description);
		DSP_FLD ("curr_desc");
		
		return (EXIT_SUCCESS);
	}

	if (LCHECK("cust_code"))
	{
		if (dflt_used)
		{
			strcpy  (local_rec.cust_code, "ALL   ");
			sprintf (local_rec.cust_desc, "%-40.40s", "All Customers");
			DSP_FLD ("cust_code");
			DSP_FLD ("cust_desc");
			return  (0);
		}

		if (SRCH_KEY)
		{
			srch_cumr (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy  (cumr_rec.co_no,  comm_rec.tco_no);
		strcpy  (cumr_rec.est_no, branchNo);
		sprintf (cumr_rec.dbt_no, "%-6.6s", local_rec.cust_code);
		cc = find_rec (cumr, &cumr_rec, EQUAL, "r");
		if (cc)
		{
			/*print_mess ("\007 Customer Code Not Found ");*/
			print_mess (ML(mlStdMess021));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.cust_desc, "%-40.40s", cumr_rec.name);
		DSP_FLD ("cust_desc");
		
		return (EXIT_SUCCESS);
	}

	return(0);
}


void
srch_cumr (
 char *key_val)
{
    work_open ();
	if (MCURR && !strcmp (local_rec.curr_code, "ALL   "))
		save_rec ("#Cust No","#Curr Name ");                       
	else
		save_rec ("#Cust No","#Name ");                       
	strcpy  (cumr_rec.co_no,  comm_rec.tco_no);
	strcpy  (cumr_rec.est_no, branchNo);
	sprintf (cumr_rec.dbt_no, "%-6.6s", key_val);
	cc = find_rec (cumr, &cumr_rec, GTEQ, "r");

    while (!cc 
	&&     !strcmp (cumr_rec.co_no,  comm_rec.tco_no) 
	&&     !strncmp(cumr_rec.dbt_no, key_val, strlen (key_val)))
    {
		if (!envDbFind && strcmp (cumr_rec.est_no, branchNo))
		{
			cc = find_rec (cumr, &cumr_rec, NEXT, "r");
			continue;
		}

		if (MCURR)
		{
			if (strcmp (local_rec.curr_code, "ALL")
			&&  strcmp (local_rec.curr_code, cumr_rec.curr_code))
			{
				cc = find_rec (cumr, &cumr_rec, NEXT, "r");
				continue;
			}
		}
		
		if (MCURR && !strcmp (local_rec.curr_code, "ALL   "))
			sprintf (err_str, "(%-3.3s) %s", cumr_rec.curr_code, cumr_rec.name);
		else
			sprintf (err_str, "%s", cumr_rec.name);
		cc = save_rec (cumr_rec.dbt_no, err_str);
		if (cc)
			break;
		cc = find_rec (cumr, &cumr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
}


void
SrchPocr (
 char *key_val)
{
    work_open ();
	save_rec ("#Cde","#Currency ");                       
	strcpy  (pocr_rec.co_no,comm_rec.tco_no);
	sprintf (pocr_rec.code, "%-3.3s", key_val);
	cc = find_rec (pocr, &pocr_rec, GTEQ, "r");
    while (!cc 
	&&     !strcmp (pocr_rec.co_no, comm_rec.tco_no) 
	&&     !strncmp(pocr_rec.code,  key_val, strlen (key_val)))
    {                        
		cc = save_rec (pocr_rec.code, pocr_rec.description);
		if (cc)
			break;
		cc = find_rec (pocr, &pocr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
}


void
proc_cumr (
 void)
{
	char	*sptr;
    char    cTempBuf[128];

	fsort = sort_open ("currcy");

	strcpy (cumr_rec.co_no, comm_rec.tco_no);
	strcpy (cumr_rec.est_no, branchNo);
	if (strcmp (local_rec.cust_code, "ALL   "))
		strcpy (cumr_rec.dbt_no, local_rec.cust_code);
	else
		strcpy (cumr_rec.dbt_no, "      ");

	cc = find_rec (cumr, &cumr_rec, GTEQ, "r");
	while (!cc 
	&&     !strcmp(cumr_rec.co_no, comm_rec.tco_no)
	&&    (!strcmp(local_rec.cust_code, cumr_rec.dbt_no) 
	||     !strcmp(local_rec.cust_code, "ALL   ")))
	{
		/*--------------------------------
		| Exclude child customer for now. |
		--------------------------------*/
		if (cumr_rec.ho_dbt_hash > 0L)
		{
			cc = find_rec (cumr, &cumr_rec, NEXT, "r");
			continue;
		}
		if (MCURR && (strcmp(cumr_rec.curr_code, local_rec.curr_code) != 0 &&
		              strcmp(local_rec.curr_code, "ALL") != 0))
		{
			cc = find_rec (cumr, &cumr_rec, NEXT, "r");
			continue;
		}
        sprintf (cTempBuf, "%-3.3s %-9.9s %010ld\n",
						(MCURR) ? cumr_rec.curr_code : " ",
						cumr_rec.dbt_no,
						cumr_rec.hhcu_hash);
		sort_save (fsort, cTempBuf);

		cc = find_rec (cumr, &cumr_rec, NEXT, "r");
	}

	fsort = sort_sort (fsort, "currcy");

	abc_selfield (cumr, "cumr_hhcu_hash" );

	strcpy (prev_crrcy, "");
	sptr = sort_read (fsort);
	while (sptr)
	{
		cc = find_hash (cumr, &cumr_rec, EQUAL, "r", atol (sptr + 14));
		if (cc)
		{
			sptr = sort_read (fsort);
			continue;
		}
		dsp_process ("Customer : ", cumr_rec.dbt_no);

		sprintf (curr_crrcy, "%-3.3s", sptr);

		proc_file ();

		sptr = sort_read (fsort);
	}
	sort_delete  (fsort, "currcy");
	abc_selfield (cumr,  (!envDbFind) ? "cumr_id_no" : "cumr_id_no3");
}


void
proc_file (
 void)
{
	int		i;

	double	tot_balance = 0.00;
	double	cumr_bo [6];

	dsp_process ("Customer", cumr_rec.acronym);

	for (i = 0; i < 6; i++)
		cumr_bo [i] = cumr_rec.bo_per [i];

	cc = find_hash (cumr2, &cumr2_rec, GTEQ, "r", cumr_rec.hhcu_hash);
	while (!cc && cumr_rec.hhcu_hash == cumr2_rec.ho_dbt_hash)
	{
		for (i = 0; i < 6; i++)
			cumr_bo [i] += cumr2_rec.bo_per [i];
		
		cc = find_hash (cumr2, &cumr2_rec, NEXT,"r", cumr_rec.hhcu_hash);
	}
	tot_balance = (cumr_bo [0] + cumr_bo [1] + cumr_bo [2] + 
                   cumr_bo [3] + cumr_bo [4] + cumr_bo [5]);

	if (tot_balance == 0.00)
		return;

	inv_tot = 0.00;
	lcl_tot = 0.00;
	line_printed = FALSE;
	first_pr = 0;
	get_cheq ();

	cuin_rec.ho_hash = cumr_rec.hhcu_hash;
	cuin_rec.doi = 0L;
	strcpy (cuin_rec.est, "  ");

	cc = find_rec (cuin, &cuin_rec, GTEQ, "r");
	while (!cc && cumr_rec.hhcu_hash == cuin_rec.ho_hash) 
	{
		if (FE_INSTALL)
		{
			/*-------------------------------
			| If invoice protected by       |
			| forward exchange do not print |
			-------------------------------*/
			strcpy (feln_rec.index_by, "C");
			feln_rec.index_hash = cuin_rec.hhci_hash;
			cc = find_rec (feln, &feln_rec, EQUAL, "r");
			if (cc)  
			{
				prnt_line ();
			}
		}
		else
		{
			prnt_line ();
		}

		cc = find_rec (cuin, &cuin_rec, NEXT, "r");
	}

	if (line_printed)
		prnt_tot ();
}


void
prnt_head (
 void)
{

	fprintf(pp,".LRP5\n");
	fprintf (pp, "|----");
	fprintf (pp, "|------------");
	fprintf (pp, "|------------");
	fprintf (pp, "|------------");
	fprintf (pp, "|---------------");
	fprintf (pp, "|---------------");
	fprintf (pp, "|---------------");
	fprintf (pp, "|---------------");
	fprintf (pp, "|---------------");
	fprintf (pp, "|\n");

	fprintf (pp, "|%s %s", cumr_rec.dbt_no, cumr_rec.name);
	
	strcpy  (pocr_rec.co_no, comm_rec.tco_no);
	sprintf (pocr_rec.code, "%-3.3s", cumr_rec.curr_code);
	cc = find_rec (pocr, &pocr_rec, COMPARISON, "r");
	if (cc)
		sprintf (pocr_rec.description, "%-40.40s", "Currency Not Found");

	fprintf (pp, "(%s) %-40.40s ", cumr_rec.curr_code, pocr_rec.description);
	fprintf (pp, "%29.29s|\n", " ");
}

/*======================
| Open database Files. |
======================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);

	open_rec (comm,  comm_list, comm_no_fields, "comm_term");
	open_rec (cuin,  cuin_list, cuin_no_fields, "cuin_ho_cron");
	open_rec (cudt,  cudt_list, cudt_no_fields, "cudt_hhcp_hash");
	open_rec (cuhd,  cuhd_list, cuhd_no_fields, "cuhd_hhcu_hash");
	open_rec (cumr,  cumr_list, cumr_no_fields, (!envDbFind) ? "cumr_id_no" 
													       : "cumr_id_no3");
	open_rec (cucc,  cucc_list, cucc_no_fields, "cucc_id_no2");
	open_rec (pocr,  pocr_list, pocr_no_fields, "pocr_id_no");

	abc_alias (cumr2, cumr);
	open_rec (cumr2, cumr_list, cumr_no_fields, "cumr_ho_dbt_hash");

	if (FE_INSTALL)
	{
		open_rec (feln,  feln_list, feln_no_fields, "feln_id_no");
		open_rec (fetr,  fetr_list, fetr_no_fields, "fetr_id_no");
	}

}


/*=======================
| Close database Files. |
=======================*/
void
CloseDB (
 void)
{
	abc_fclose (cumr);
	abc_fclose (cuin);
	abc_fclose (cudt);
	abc_fclose (cuhd);
	abc_fclose (comm);
	abc_fclose (cucc);
	abc_fclose (pocr);

	abc_fclose (cumr2);

	if (FE_INSTALL)
	{
		abc_fclose (feln);
		abc_fclose (fetr);
	}
	abc_dbclose (data);
}

void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}


void
head (
 int prnt_no)
{
	/*----------------------
	| Open pipe to pformat | 
 	----------------------*/
	if ((pp = popen ("pformat","w")) == NULL)
		file_err (cc, "PFORMAT", "POPEN");

	/*---------------------------------
	| Start output to standard print. |
	---------------------------------*/
	fprintf (pp, ".START%s\n", DateToString (comm_rec.tdbt_date));
	fprintf (pp, ".LP%d\n", prnt_no);
	fprintf (pp, ".9\n");
	fprintf (pp, ".L130\n");
	fprintf (pp, ".PI10\n");
	fprintf (pp, ".E%s\n", clip(comm_rec.tco_name));
	fprintf (pp, ".B1\n");
	fprintf (pp, ".E%s\n", "CURRENCY EXPOSURE REPORT.");

	fprintf (pp, ".EAS AT : %-24.24s\n", SystemTime ());

	fprintf (pp, ".B1\n");

	fprintf (pp, ".R=====");
	fprintf (pp, "=============");
	fprintf (pp, "===============");
	fprintf (pp, "===============");
	fprintf (pp, "================");
	fprintf (pp, "================");
	fprintf (pp, "================");
	fprintf (pp, "================");
	fprintf (pp, "================");
	fprintf (pp, "=\n");

	if (MCURR)
		return;

	/*-------------------------
	| Headings for non-MCURR. |
	-------------------------*/
	fprintf (pp, "=====");
	fprintf (pp, "=============");
	fprintf (pp, "===============");
	fprintf (pp, "===============");
	fprintf (pp, "===============");
	fprintf (pp, "===============");
	fprintf (pp, "===============");
	fprintf (pp, "===============");
	fprintf (pp, "===============");
	fprintf (pp, "\n");

	fprintf (pp, "| BR ");
	fprintf (pp, "|  INVOICE   ");
	fprintf (pp, "|   INVOICE    ");
	fprintf (pp, "|   INVOICE    ");
	fprintf (pp, "|  TOTAL OWING  ");
	fprintf (pp, "|    OVERDUE    ");
	fprintf (pp, "|    1 MONTH    ");
	fprintf (pp, "|    2 MONTHS   ");
	fprintf (pp, "|    3+ MONTHS  ");
	fprintf (pp, "|\n");

	fprintf (pp, "| NO ");
	fprintf (pp, "|   NUMBER   ");
	fprintf (pp, "|     DATE     ");
	fprintf (pp, "|   DUE DATE   ");
	fprintf (pp, "|               ");
	fprintf (pp, "|               ");
	fprintf (pp, "|               ");
	fprintf (pp, "|               ");
	fprintf (pp, "|               ");
	fprintf (pp, "|\n");

	fprintf (pp, "|----");
	fprintf (pp, "|------------");
	fprintf (pp, "|--------------");
	fprintf (pp, "|--------------");
	fprintf (pp, "|--------------");
	fprintf (pp, "|--------------");
	fprintf (pp, "|--------------");
	fprintf (pp, "|--------------");
	fprintf (pp, "|--------------");
	fprintf (pp, "|\n");

}


void
prnt_line(
 void)
{
	int		x, 
			i; 
	int		inv_printed = FALSE;
	double	balance = 0.00;
	double	in_balance = 0.00;
	double	tmp_value;
	double	fgn_value;
	double	fgn_payments;
	double	lcl_payments;
	double	var_exchange;
    /*char	wk_date[11];*/

	/*----------------------------------------------------
	| for each invoice, print details if dbt - crd <> 0. |
	----------------------------------------------------*/
	balance    = ( DB_NETT ) ? cuin_rec.amt - cuin_rec.disc 
			         	     : cuin_rec.amt;

	in_balance = ( DB_NETT ) ? cuin_rec.amt - cuin_rec.disc 
			         	     : cuin_rec.amt;
	
	fgn_payments = 0.00;
	lcl_payments = 0.00;
	var_exchange = 0.00;
	for (i = 0; i < dtls_cnt; i++)
	{
		if ( cuin_rec.hhci_hash == dtls[i].hhci_hash)
		{
			fgn_payments += dtls[i].inv_amt;
			lcl_payments += dtls[i].lcl_amt;
			var_exchange += dtls[i].exch_var;
		}
	}

	/*--------------------------------------------------------
	| Subtract payments from invoice amount.                 |
	| Convert balance into local currency if Multi-Currency. |
	| Store currency summary information if Multi-Currency.  |
	--------------------------------------------------------*/
	fgn_value = balance;
	if (MCURR)
	{
		/*-------------------------------------
		| Convert balance into local currency |
		-------------------------------------*/
/*		balance /= (cuin_rec.exch_rate == 0.00) ? 1.00 : cuin_rec.exch_rate;*/
		if (cuin_rec.exch_rate == 0.00) 
			cuin_rec.exch_rate = 1.00;
		balance  = balance/cuin_rec.exch_rate;
		if (LCL_CURR)
			in_balance = balance;
	}
	
	/*------------------------------------------------------
	| Subtract payments (local currency) from local value. |
	------------------------------------------------------*/
	balance   -= (lcl_payments - var_exchange);
	fgn_value -= fgn_payments;

	if (fgn_value == 0.00)
		return;

	/*------------------------------
	| Check for break in currency. |
	------------------------------*/
	chk_break();

	if (first_pr == 0)
	{
		prnt_head();
		first_pr = 1;
	}

	get_pdate();

	fprintf(pp,".LRP3\n");
	fprintf(pp,"| %2.2s |%s %-8.8s| %s | %s |",
			cuin_rec.est,
	 		inv_type[atoi(cuin_rec.type) - 1],
			cuin_rec.inv_no, 
			DateToString(cuin_rec.doi),
			pay_date);

	inv_printed = FALSE ;

	x = pay_per (cuin_rec.pay_terms,
		     	 StringToDate (pay_date), 
		     	 lsystemDate) + 2;

	if (x < 1)
		x = 1;

	if (x > 4)
		x = 4;

	for (i = 0; i < 5; i++) 
	{
		if (x == i || i == 0)
		{
			tmp_value = (LCL_CURR) ? balance : fgn_value;
			fprintf(pp,
				" %14.14s|", 
				comma_fmt(DOLLARS(tmp_value),"NNN,NNN,NNN.NN"));
			if (balance < 0.00)
				credits[i] += balance;
			else
				debits[i]  += balance;

			total[LCL][i] += balance;
			total[FGN][i] += fgn_value;
		} 
		else
			fprintf(pp,"               |");
	}
	fprintf(pp,"\n");
	line_printed = TRUE;
}


void
chk_break (
 void)
{
	if (!MCURR) return;

	/*---------------------
	| Change in currency. |
	---------------------*/
	if (strcmp (prev_crrcy, curr_crrcy))
	{
		curr_header (curr_crrcy);

		strcpy (prev_crrcy, curr_crrcy);
		first_curr = FALSE;
	}
}

void
curr_header (
 char *crrcy)
{
	strcpy  (pocr_rec.co_no, comm_rec.tco_no);
	sprintf (pocr_rec.code, "%-3.3s", crrcy);
	cc = find_rec (pocr, &pocr_rec, COMPARISON, "r");
	if (cc)
		sprintf (pocr_rec.description, "%-40.40s", "Currency Not Found");

	fprintf (pp, ".DS5\n");

	fprintf (pp,
		".E CURRENCY : %-3.3s - %-s\n",
		crrcy,
		clip (pocr_rec.description));

	if (first_pr == 0)
		fprintf (pp, "\n");

	fprintf (pp, "=====");
	fprintf (pp, "=============");
	fprintf (pp, "=============");
	fprintf (pp, "=============");
	fprintf (pp, "================");
	fprintf (pp, "================");
	fprintf (pp, "================");
	fprintf (pp, "================");
	fprintf (pp, "================");
	fprintf (pp, "=\n");

	fprintf (pp, "| BR ");
	fprintf (pp, "|  INVOICE   ");
	fprintf (pp, "|  INVOICE   ");
	fprintf (pp, "|  INVOICE   ");
	fprintf (pp, "|  TOTAL OWING  ");
	fprintf (pp, "|    OVERDUE    ");
	fprintf (pp, "|    1 MONTH    ");
	fprintf (pp, "|    2 MONTHS   ");
	fprintf (pp, "|    3+ MONTHS  ");
	fprintf (pp, "|\n");

	fprintf (pp, "| NO ");
	fprintf (pp, "|   NUMBER   ");
	fprintf (pp, "|    DATE    ");
	fprintf (pp, "|  DUE DATE  ");
	fprintf (pp, "|               ");
	fprintf (pp, "|               ");
	fprintf (pp, "|               ");
	fprintf (pp, "|               ");
	fprintf (pp, "|               ");
	fprintf (pp, "|\n");

	if (first_pr != 0)
	{
		fprintf (pp, "|----");
		fprintf (pp, "|------------");
		fprintf (pp, "|------------");
		fprintf (pp, "|------------");
		fprintf (pp, "|---------------");
		fprintf (pp, "|---------------");
		fprintf (pp, "|---------------");
		fprintf (pp, "|---------------");
		fprintf (pp, "|---------------");
		fprintf (pp, "|\n");
	}

	if (!first_curr)
		fprintf (pp, ".PA\n");

	fflush (pp);
}


void
prnt_tot (
 void)
{
	/* Totals printing disabled at present */
	/*return (EXIT_SUCCESS);*/

	fprintf (pp, "|  TOTAL %-35.35s", " ");
	fprintf (pp, "|%14.14s ", comma_fmt(DOLLARS(inv_tot), "NNN,NNN,NNN.NN"));
	if (MCURR)
		fprintf (pp, "|%14.14s ", comma_fmt(DOLLARS(lcl_tot), "NNN,NNN,NNN.NN"));
	
	fprintf (pp, "|%-47.47s", " ");
	fprintf (pp, "|              |          |\n");
}

void
get_cheq (
 void)
{
	dtls_cnt = 0;

	cc = find_hash (cuhd, &cuhd_rec, GTEQ, "r", cumr_rec.hhcu_hash);
	while (!cc && cuhd_rec.hhcu_hash == cumr_rec.hhcu_hash)
	{
		cc = find_hash (cudt, &cudt_rec, GTEQ, "r", cuhd_rec.hhcp_hash);
		while (!cc && cuhd_rec.hhcp_hash == cudt_rec.hhcp_hash)
		{
			dtls[dtls_cnt].hhci_hash = cudt_rec.hhci_hash;
			dtls[dtls_cnt].inv_amt = cudt_rec.amt_paid_inv;
			dtls[dtls_cnt].lcl_amt = cudt_rec.loc_paid_inv;
			++dtls_cnt;

			cc = find_hash (cudt, &cudt_rec, NEXT, "r", cuhd_rec.hhcp_hash);
		}
		cc = find_hash (cuhd, &cuhd_rec, NEXT, "r", cumr_rec.hhcu_hash);
	}
}

/*===============================================
| Determine Payment Date From Invoice Date.     |
===============================================*/
void
get_pdate(
 void)
{
	int		tmp_dmy[3];
	int		days = 0,	/* Payment days as an integer	*/
			mth_offset,	/* Month offset for payment          	*/
			mth,		/* Month and year figures for payment	*/
			yr;
	long	tmp_date;

	days = atoi(cuin_rec.pay_terms);

	if (cuin_rec.pay_terms[2] >= 'A')
	{
		/*-------------------------------------
		| Calc. for format NNA to NNF input . |
		-------------------------------------*/
		mth_offset = cuin_rec.pay_terms[2] - 'A' + 1;

		DateToDMY (cuin_rec.doi, &tmp_dmy[0],&tmp_dmy[1],&tmp_dmy[2]);
		mth = tmp_dmy[1]; /* Get month */
		yr  = tmp_dmy[2]; /* Get year  */
		mth = mth + mth_offset;
		if (mth > 12)
		{
			mth = mth - 12;
			yr++;
		}

		tmp_dmy[0] = (days);
		tmp_dmy[1] = (mth);
		tmp_dmy[2] = (yr);
		tmp_date = DMYToDate (tmp_dmy[0],tmp_dmy[1],tmp_dmy[2]);

		strcpy (pay_date, DateToString(tmp_date));

		return;
	}
	strcpy (pay_date,DateToString( cuin_rec.doi + days));
	return;
}

int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();

		rv_pr (ML(mlFeMess020), 25, 0, 1);
		move (1, 1);
		line (79);

		move (1, input_row);

		if (MCURR)
		{
			box (0, 3, 80, 6);
	
			move (1, 6);
			line (79);
		}
		else
		{
			box (0, 3, 80, 5);
	
			move (1, 5);
			line (79);
		}

		move (0, 20);
		line (80);

		strcpy(err_str, ML(mlStdMess038));
		print_at (21,0,  err_str,
						 comm_rec.tco_no,
					     comm_rec.tco_name);

		strcpy(err_str, ML(mlStdMess039));
		print_at (21,45, err_str,
						 comm_rec.test_no,
						 comm_rec.test_name);
		move (0, 22);
		line (80);
		line_cnt = 0;
		scn_write (scn);
	}

    return (EXIT_SUCCESS);
}


int
pay_per (
 char   *p_terms, 
 long   int due_date, 
 long   int cur_date)
{
	int		mth_term = 0,
			period = 0,
			/*cd,*/
			cm;
			/*cy;*/
	int		scal_dmy[3],
			scur_dmy[3];
	char	cur_str[9],
			inv_str[9];

	static	int	days[12] = {31,28,31,30,31,30,31,31,30,31,30,31};

	/*-------------------------------------------
	| Adjust current date to last day of month. |
	-------------------------------------------*/
	DateToDMY (cur_date, &scur_dmy[0],&scur_dmy[1],&scur_dmy[2]);
	DateToDMY (due_date, &scal_dmy[0],&scal_dmy[1],&scal_dmy[2]);

	strcpy (cur_str, DateToString (cur_date));
	strcpy (inv_str, DateToString (due_date));

	days[1] = ((scur_dmy[2] % 4) == 0) ? 29 : 28;

	/*------------------------------------
	| get current month from module date |
	------------------------------------*/
	scur_dmy[0] = (days[scur_dmy[1] - 1]);

	cm = scal_dmy[1];

	mth_term = atoi (p_terms);
	if (mth_term <= 0)
		mth_term = 1;

	period = -1;

	if (due_date < cur_date)
		return (period);

	while (cur_date < due_date)
	{
		cur_date += days[cm - 1];
		period++;
	}

	return (period);
}




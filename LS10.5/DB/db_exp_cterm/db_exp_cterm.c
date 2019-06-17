/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: db_exp_cterm.c,v 5.2 2002/07/17 09:57:06 scott Exp $
|  Program Name  : (db_exp_cterm.c)
|  Program Desc  : (Expiring Credit Terms)
|---------------------------------------------------------------------|
|  Date Written  : (16/06/1998)    | Author      : Ana Marie C. Tario |
|---------------------------------------------------------------------|
| $Log: db_exp_cterm.c,v $
| Revision 5.2  2002/07/17 09:57:06  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.1  2001/12/07 03:43:31  scott
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_exp_cterm.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_exp_cterm/db_exp_cterm.c,v 5.2 2002/07/17 09:57:06 scott Exp $";

#include <pslscr.h>
#include <get_lpno.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <ml_std_mess.h>

	int		envVarDbCo		=	0;
	char	branchNumber [3];
	FILE	*fout;
	FILE	*fsort;

#include	"schema"

struct commRecord	comm_rec;
struct esmrRecord	esmr_rec;
struct cumrRecord	cumr_rec;

	struct	{
		int		pay_code;
		char	*pay_desc;
	} pay_mode [] = {
		{1, "Cash         "},
		{2, "Bank Draft   "},
		{3, "Cheque       "},
		{4, "Direct Credit"},
		{0,""},
	};
	char	*data	= "data";

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char	br_no [3];
	char	br_desc [41];
	int 	lpno;
	char 	back [5];
	char	onite [5];
	char 	dummy [11];
	long    lsystemDate; 	
	char    systemDate [9]; 	
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "br_no", 5, 26, CHARTYPE,
		"NN", "          ",
		" ", "  ", " Branch No.            : ", "Enter Branch No. [Search]. Default is All",
		YES, NO,  JUSTRIGHT, "", "", local_rec.br_no},
	{1, LIN, "br_desc",	 5, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.br_desc},
	{1, LIN, "lpno",	 6, 26, INTTYPE,
		"NN", "          ",
		" ", "1", " Printer               : ", "Printer Number ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.lpno},
	{1, LIN, "back",	 7, 26, CHARTYPE,
		"U", "          ",
		" ", "N(o", " Background            : ", "Print Report In The Background ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.back},
	{1, LIN, "onight", 8, 26, CHARTYPE,
		"U", "          ",
		" ", "N(o", " Overnight             : ", "Print Report In Overnight Batch",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.onite},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
void shutdown_prog (void);
void OpenDB (void);
void CloseDB (void);
int spec_valid (int field);
void head_print (void);
void process (void);
void Print_Heading (void);
int heading (int scn);
void SrchEsmr (char *key_val);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int                argc,
 char*              argv [])
{
	char	*sptr;
	SETUP_SCR (vars);

	init_scr ();			/*  sets terminal from termcap	*/
	set_tty (); 			/*  get into raw mode		*/
	set_masks ();			/*  setup print using masks	*/
	init_vars (1);			/*  set default values		*/

	/*---------------------------
	| open main database files. |
	---------------------------*/
	OpenDB ();

    /*=============================================
    | Get common info from commom database file . |
    =============================================*/
    read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	sptr = chk_env ("DB_CO");
	envVarDbCo = (sptr == (char *) 0) ? 0 : atoi (sptr);
	strcpy (branchNumber, (envVarDbCo == 0) ? " 0" : comm_rec.est_no);

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));
	local_rec.lsystemDate = TodaysDate ();

	entry_exit 	= FALSE;
	edit_exit 	= FALSE;
	prog_exit 	= FALSE;
	restart 	= FALSE;
	search_ok 	= 1;

	while (!prog_exit)
	{
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= 1;

		init_vars (1);
		heading (1);
		entry (1);

		if (restart)
			continue;

		if (prog_exit)
		{
			entry_exit = TRUE;
			continue;
		} 

		heading (1);
		scn_display (1);
		edit (1);

		if (restart)
			continue;

		prog_exit = TRUE;
	}

	if (!entry_exit)
	{
		head_print ();
		process ();
		fprintf (fout,".EOF\n");
		fflush (fout);
		pclose (fout);
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*=========================
| Program exit sequence	. |
=========================*/
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*=======================
| Open data base files. |
=======================*/
void
OpenDB (void)
{
	abc_dbopen (data);

	open_rec (cumr,cumr_list, CUMR_NO_FIELDS,"cumr_id_no");
	open_rec (esmr,esmr_list, ESMR_NO_FIELDS,"esmr_id_no");
}	

/*========================
| Close data base files. |
========================*/
void
CloseDB (void)
{
	abc_fclose (cumr);
	abc_fclose (esmr);
	abc_dbclose (data);
}

int
spec_valid (
 int                field)
{

	/*------------------------------------------
	| Validate Branch Number And Allow Search. |
	------------------------------------------*/
	if (LCHECK ("br_no"))
	{
		if (dflt_used)
		{
			strcpy  (local_rec.br_no, "  ");
			sprintf (local_rec.br_desc, "%-40.40s", "All");
			DSP_FLD ("br_no");
			DSP_FLD ("br_desc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
	 		SrchEsmr (temp_str);
  			return (EXIT_SUCCESS);
		}
		
		strcpy (esmr_rec.co_no, comm_rec.co_no);
		strcpy (esmr_rec.est_no, local_rec.br_no);
		cc = find_rec (esmr, &esmr_rec, EQUAL, "r");
		if (cc)
		{
			sprintf (err_str, "Business Unit %s is not on file.", 
					 local_rec.br_no);
			errmess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.br_desc, esmr_rec.est_name);
		DSP_FLD ("br_no");
		DSP_FLD ("br_desc");
	}

	if (LCHECK ("lpno"))
	{
		if (SRCH_KEY)
		{
			local_rec.lpno = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if (!valid_lp (local_rec.lpno))
		{
			print_mess ("\007 Invalid Printer Number. ");
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("back"))
	{
		strcpy (local_rec.back, (local_rec.back [0] == 'Y') ? "Y(es" : "N(o ");
		DSP_FLD ("back");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("onight"))
	{
		strcpy (local_rec.onite, (local_rec.onite [0] == 'Y') ? "Y(es" : "N(o ");
		DSP_FLD ("onight");
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
 }

void
head_print (void)
{

	if ((fout = popen ("pformat","w")) == 0)
		file_err (errno, "pformat", "POPEN");

	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (fout,".LP%d\n",local_rec.lpno);
	fprintf (fout,".8\n");
	fprintf (fout,".PI12\n");
	fprintf (fout,".L160\n");
	fprintf (fout,".B1\n");

	fprintf (fout,".E%s\n",clip (comm_rec.co_name));
	fprintf (fout,".ECREDIT LIMIT DUE FOR REVIEW\n");
	fprintf (fout,".EExpiration Date is less than 30 days from now.\n");
	fprintf (fout,".EAs At %s\n",SystemTime ());

	fprintf (fout,".B1\n");
	fprintf (fout,".R =======================================================================================================================================\n");

	fflush (fout);
}

void
process (void)
{
	char	prev_br [3];
	int		first_time = 1;
	char    payment_mode [14];

	strcpy (cumr_rec.co_no,	comm_rec.co_no);
	strcpy (cumr_rec.est_no,	local_rec.br_no);
	sprintf (cumr_rec.dbt_no,	"      ");
	strcpy (prev_br,"");
	cc = find_rec ("cumr",&cumr_rec,GTEQ,"r");

 	dsp_screen ("Printing Customers with Credit Limit Due for Review.",
           comm_rec.co_no, comm_rec.co_name);

	while 	 (!cc && !strcmp (cumr_rec.co_no,comm_rec.co_no))
	{
		if (!strcmp (cumr_rec.ca_no, "          ")
			|| cumr_rec.cash_flag [0] == 'Y'
			|| (strcmp (cumr_rec.est_no,local_rec.br_no) != 0 
			&& strcmp (local_rec.br_no,"  ") != 0))
		{
			cc = find_rec ("cumr",&cumr_rec,NEXT,"r");
			continue;
		}
		dsp_process ("Branch no : ", cumr_rec.est_no);

		switch (cumr_rec.pay_method [0])
		{
		case 'C':
			strcpy (payment_mode, ML ("Cheque       "));
			break;
		case 'B':
			strcpy (payment_mode, ML ("Bank Draft   "));
			break;
		case 'D':
			strcpy (payment_mode, ML ("Direct Credit"));
			break;
		default :
			strcpy (payment_mode, "             ");
		}

		if ((cumr_rec.crd_expiry - local_rec.lsystemDate) < 30) 
		{
			if (strcmp (prev_br, cumr_rec.est_no))
			{
				if (!first_time)
				{
					Print_Heading ();
					fprintf (fout, ".PA\n");
				}
				else
					Print_Heading ();
			}

			
			fprintf (fout, "| %-6.6s  ",  cumr_rec.dbt_no);
			fprintf (fout, "| %-40.40s ", cumr_rec.dbt_name);
			fprintf (fout, "|  %-11.11s  ", cumr_rec.ca_no);
			fprintf (fout, "| %-13.13s ", payment_mode);
			fprintf (fout, "| %-3.3s  ",    cumr_rec.crd_prd);
			fprintf (fout, "|     %-1.1s     " ,    cumr_rec.stop_credit);
			fprintf (fout, "| %-14.14s ",    
				comma_fmt (DOLLARS (cumr_rec.credit_limit), "NNN,NNN,NNN.NN"));
			if (cumr_rec.crd_expiry < 1L)
				fprintf (fout, "| %-11.11s |\n", ML ("NOT SET"));
			else
				fprintf (fout, "| %-10.10s  |\n",DateToString (cumr_rec.crd_expiry)); 

			strcpy (prev_br, cumr_rec.est_no);
			first_time = 0;
		}

		cc = find_rec ("cumr",&cumr_rec,NEXT,"r");

	}
}


void
Print_Heading (void)
{
	char 	branch_name [41];

	fprintf (fout, ".DS5\n");

	strcpy (esmr_rec.co_no, comm_rec.co_no);
	strcpy (esmr_rec.est_no, cumr_rec.est_no);
	cc = find_rec (esmr, &esmr_rec, EQUAL, "r");
	if (!cc)
		strcpy (branch_name, esmr_rec.est_name);
	else
		strcpy (branch_name, "Unknown");

	fprintf (fout, ".EBranch No. %s - %-40.40s\n\n",
			 cumr_rec.est_no, clip (branch_name));

	fprintf (fout, "============");
	fprintf (fout, "===========================================");
	fprintf (fout, "================");
	fprintf (fout, "=======");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "=================");
	fprintf (fout, "===================\n");

	fprintf (fout, "|CUSTOMER ");
	fprintf (fout, "|   CUSTOMER      NAME                     ");
	fprintf (fout, "| CREDIT ACCT # ");
	fprintf (fout, "|   PAY  MODE   ");
	fprintf (fout, "| TERM ");
	fprintf (fout, "|STOP CREDIT");
	fprintf (fout, "|  CREDIT LIMIT  ");
	fprintf (fout, "| EXPIRY DATE |\n");

	fprintf (fout, "|---------");
	fprintf (fout, "|------------------------------------------");
	fprintf (fout, "|---------------");
	fprintf (fout, "|---------------");
	fprintf (fout, "|------");
	fprintf (fout, "|-----------");
	fprintf (fout, "|----------------");
	fprintf (fout, "|-------------|\n");
}

/*================================================================
| Heading concerns itself with clearing the screen,painting the  |
| screen overlay in preparation for input.                        |
=================================================================*/
int
heading (
 int                scn)
{
	if (!restart) 
	{
		clear ();

		rv_pr (ML (" Credit Limit Due For Review "),25,0,1);

		move (0,1);
		line (80);

		box (0,4,80,4);
		move (0,21);
		line (80);
		print_at (22,0,ML (mlStdMess038), comm_rec.co_no,comm_rec.co_name);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}

/*================================
| Search for Branch master file. |
================================*/
void
SrchEsmr (
 char*              key_val)
{
	work_open ();
	strcpy  (esmr_rec.co_no,  comm_rec.co_no);
	sprintf (esmr_rec.est_no,  "%-2.2s", key_val);
	save_rec ("#Br", "#Branch Description");
	cc = find_rec (esmr, &esmr_rec, GTEQ, "r");
	while (!cc 
	&&     !strcmp  (esmr_rec.co_no, comm_rec.co_no) 
	&&     !strncmp (esmr_rec.est_no, key_val, strlen (key_val)))
	{
		cc = save_rec (esmr_rec.est_no, esmr_rec.est_name);
		if (cc)
			break;

		cc = find_rec (esmr, &esmr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy  (esmr_rec.co_no, comm_rec.co_no);
	sprintf (esmr_rec.est_no, "%-2.2s", temp_str);
	cc = find_rec (esmr, &esmr_rec, EQUAL, "r");
	if (cc)
	 	file_err (cc, "esmr", "DBFIND");
}

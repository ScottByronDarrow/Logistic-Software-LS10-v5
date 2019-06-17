/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: db_group_prt.c,v 5.2 2002/07/17 09:57:06 scott Exp $
|  Program Name  : (db_group_prt.c)
|  Program Desc  : (Print Customer Groups)
|---------------------------------------------------------------------|
|  Author        : Basil Wood      | Date Written  : 05/04/96         |
|---------------------------------------------------------------------|
| $Log: db_group_prt.c,v $
| Revision 5.2  2002/07/17 09:57:06  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.1  2001/12/07 03:47:44  scott
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_group_prt.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_group_prt/db_group_prt.c,v 5.2 2002/07/17 09:57:06 scott Exp $";

#include <pslscr.h>
#include <twodec.h>
#include <dbgroup.h>
#include <dsp_screen.h>
#include <dsp_process2.h>

#include <assert.h>

#define	CAL(amt,pc)		 (amt * DOLLARS (pc))

#define	INVOICE		 (cuin_rec.type [0] == '1')
#define	CREDIT		 (cuin_rec.type [0] == '2')
#define	OPEN_ITEM	 (cumr_rec.stmt_type [0] == 'O')

/* *INDENT-OFF* */

#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct cudtRecord	cudt_rec;
struct cuinRecord	cuin_rec;
struct cumrRecord	cumr_rec;
struct cumrRecord	cumr2_rec;
struct esmrRecord	esmr_rec;
static char
       *data = "data",
       *cumr2 = "cumr2",
       *cumrhh = "cumrhh";

static DGroupSet GroupSet;

int     envVarCoClose = TRUE,
        envVarDbCo = 0,
        envVarDbFind = 0;

long    envVarPurgeMon;
char    min_purge_date [11];

char    branchNumber [3],
        wk_amt [4] [15];

int     envVarDbNettUsed = TRUE,
        envVarCnNettUsed = TRUE;

char    disp_str [300];

double  group_tot [4],
        cust_tot [4],
        grand_tot [4];

static char *tran_type [] =
{
	"IN", "CR", "JL", "CH", "JL", "?"
};

static int is_head;

/*============================
| Local & Screen Structures.
============================*/
struct
{
	char    dummy [11];
	char    s_cust [7];
	char    e_cust [7];
	char    cust_name [2] [41];
	int     lpno;
	char    lp_str [3];
	char    back [5];
	char    onite [5];
	long	hhcu_hash;	/* current customer */
}
local_rec;

static	struct	var	vars []	=	
{
	{1, LIN, "from_cust", 4, 18, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", " ", "Start Customer", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.s_cust}, 
	{1, LIN, "name1", 4, 35, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", " ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.cust_name [0]}, 
	{1, LIN, "to_cust", 5, 18, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", " ", "End   Customer", " ", 
		YES, NO, JUSTLEFT, "", "", local_rec.e_cust}, 
	{1, LIN, "name2", 5, 35, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", " ", " ", 
		NA, NO, JUSTLEFT, "", "", local_rec.cust_name [1]}, 
	{1, LIN, "lpno", 7, 18, INTTYPE, 
		"NN", "          ", 
		" ", "1", "Printer Number", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.lpno}, 
	{1, LIN, "back", 8, 18, CHARTYPE, 
		"U", "          ", 
		" ", "N", "Background ", " ", 
		YES, NO, JUSTLEFT, "YN", "", local_rec.back}, 
	{1, LIN, "onight", 8, 60, CHARTYPE, 
		"U", "          ", 
		" ", "N", "Overnight ", " ", 
		YES, NO, JUSTLEFT, "YN", "", local_rec.onite}, 
	{0, LIN, "", 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}, 

};

#include <get_lpno.h>
#include <FindCumr.h>
/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
static void 	RunProgram 					(char *);
static void 	OpenDB 						(void);
static void 	CloseDB 					(void);
static void 	PrintGroupTotal 			(FILE *);
static void 	PrintCustomerTotal 			(FILE *);
static void 	PrintGrandTotal 			(FILE *);
static void 	PrintInvoiceDetails 		(FILE *, DGroup *, int);
static int 		ContainsCustTransactions 	(DGroup *);
static void 	PrintLine 					(FILE *);
static void 	HeadingOutput 				(FILE *);
static void 	ProcessCustomers 			(void);
static void	 	PrintGroups 				(FILE *, int);
int 			heading 					(int);
int 			spec_valid 					(int);

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int                argc,
 char*              argv [])
{
	char   *sptr;

	if (argc != 1 && argc != 4)
	{
		printf ("Usage : %s [<lpno> <Start Customer> <End Customer>]\007\n\r",
				argv [0]);
        return (EXIT_FAILURE);
	}

	sptr = chk_env ("CO_CLOSE");
	if (sptr != (char *) 0)
		envVarCoClose = (*sptr == '1') ? TRUE : FALSE;

	envVarDbCo = atoi (get_env ("DB_CO"));
	envVarDbFind = atoi (get_env ("DB_FIND"));

	sptr = chk_env ("DB_NETT_USED");
	envVarDbNettUsed = (sptr == (char *) 0) ? TRUE : atoi (sptr);

	sptr = chk_env ("CN_NETT_USED");
	envVarCnNettUsed = (sptr == (char *) 0) ? envVarDbNettUsed : atoi (sptr);

	SETUP_SCR (vars);

	init_scr ();				/*  sets terminal from termcap  */
	set_tty ();
	set_masks ();				/*  setup print using masks */
	init_vars (1);				/*  set default values      */

	/*---------------------------
	| Open main database files
	---------------------------*/
	OpenDB ();

	/*-------------------------------
	| Read common terminal record
	-------------------------------*/
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	envVarPurgeMon = atol (get_env ("PURGE_MON"));
	sprintf (min_purge_date, "%-10.10s", DateToString (comm_rec.dbt_date - envVarPurgeMon));

	/*-------------------------------
	| Read other records
	-------------------------------*/
	if (envVarCoClose)
	{
		strcpy (comr_rec.co_no, comm_rec.co_no);
		cc = find_rec (comr, &comr_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, "comr", "DBFIND");
	}
	else
	{
		strcpy (esmr_rec.co_no, comm_rec.co_no);
		strcpy (esmr_rec.est_no, comm_rec.est_no);
		cc = find_rec (esmr, &esmr_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, "esmr", "DBFIND");
	}

	strcpy (branchNumber, (envVarDbCo) ? comm_rec.est_no : " 0");

	if (argc == 4)
	{
		local_rec.lpno = atoi (argv [1]);

		sprintf (local_rec.s_cust, "%-6.6s", argv [2]);
		sprintf (local_rec.e_cust, "%-6.6s", argv [3]);

		sprintf (err_str, "Printing Customers Groups");

		dsp_screen (err_str, comm_rec.co_no, comm_rec.co_name);

		ProcessCustomers ();
		CloseDB (); 
		FinishProgram ();
	}
	else
	{
		prog_exit = FALSE;
		while (!prog_exit)
		{
			/*---------------------
			| Reset control flags
			---------------------*/
			entry_exit = FALSE;
			edit_exit = FALSE;
			restart = FALSE;
			search_ok = TRUE;
			init_vars (1);

			/*----------------------------
			| Entry screen 1 linear input
			----------------------------*/
			heading (1);
			entry (1);
			if (restart || prog_exit)
				continue;

			/*----------------------------
			| Edit screen 1 linear input
			----------------------------*/
			heading (1);
			scn_display (1);
			edit (1);
			if (restart)
				continue;

			RunProgram (argv [0]);
			prog_exit = 1;
		}
		clear ();
		CloseDB (); 
		FinishProgram ();
	}

	return (EXIT_SUCCESS);
}

static void
RunProgram (
 char   *prog_name)
{
	sprintf (local_rec.lp_str, "%d", local_rec.lpno);

	CloseDB (); 
	FinishProgram ();

	if (local_rec.onite [0] == 'Y')
	{
		if (fork () == 0)
			execlp ("ONIGHT",
					"ONIGHT",
					prog_name,
					local_rec.lp_str,
					local_rec.s_cust,
					local_rec.e_cust,
					"Print Customers Groupings",
					NULL);
	}
    else if (local_rec.back [0] == 'Y')
	{
		if (fork () == 0)
			execlp (prog_name,
					prog_name,
					local_rec.lp_str,
					local_rec.s_cust,
					local_rec.e_cust,
					NULL);
	}
	else
	{
		execlp (prog_name,
				prog_name,
				local_rec.lp_str,
				local_rec.s_cust,
				local_rec.e_cust,
				NULL);
	}
}

/*========================
| Open data base files . |
========================*/
static void
OpenDB (void)
{
	abc_dbopen (data);

	abc_alias (cumr2, cumr);
	abc_alias (cumrhh, cumr);

	open_rec (cumr, cumr_list, CUMR_NO_FIELDS,
			  (!envVarDbFind) ? "cumr_id_no" : "cumr_id_no3");
	open_rec (cumr2, cumr_list, CUMR_NO_FIELDS, "cumr_ho_dbt_hash");
	open_rec (cumrhh, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (cudt, cudt_list, CUDT_NO_FIELDS, "cudt_id_no");
	open_rec (cuin, cuin_list, CUIN_NO_FIELDS, "cuin_hhci_hash");

	if (envVarCoClose)
		open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");
	else
		open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
}

/*=========================
| Close data base files . |
=========================*/
static void
CloseDB (void)
{
	if (envVarCoClose)
		abc_fclose (comr);
	else
		abc_fclose (esmr);

    abc_fclose (cumr2);
    abc_fclose (cumrhh);
	abc_fclose (cumr);
	abc_fclose (cuin);
	abc_fclose (cudt);

	abc_dbclose (data);
}

int
spec_valid (
 int                field)
{
	/*-------------------
	| Validate Customer |
	-------------------*/
	if (LCHECK ("from_cust"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.s_cust, "      ");
			sprintf (local_rec.cust_name [0], "%-40.40s", "First Customer");
			DSP_FLD ("from_cust");
			DSP_FLD ("name1");
			return 0;
		}

		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return 0;
		}
		if (prog_status != ENTRY && strcmp (local_rec.s_cust, local_rec.e_cust) > 0)
		{
			print_err ("Start Customer %s Must Not Be GREATER THAN End Customer", local_rec.s_cust);
			return 1;
		}

		strcpy (cumr_rec.co_no, comm_rec.co_no);
		strcpy (cumr_rec.est_no, branchNumber);
		strcpy (cumr_rec.dbt_no, pad_num (local_rec.s_cust));
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
		{
			print_err ("Customer %s is not on file", local_rec.s_cust);
			return 1;
		}
		strcpy (local_rec.cust_name [0], cumr_rec.dbt_name);
		DSP_FLD ("name1");
		return 0;
	}

	if (LCHECK ("to_cust"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.e_cust, "~~~~~~");
			sprintf (local_rec.cust_name [1], "%-40.40s", "Last Customer");
			DSP_FLD ("to_cust");
			DSP_FLD ("name2");
			return 0;
		}

		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return 0;
		}
		strcpy (cumr_rec.co_no, comm_rec.co_no);
		strcpy (cumr_rec.est_no, branchNumber);
		strcpy (cumr_rec.dbt_no, pad_num (local_rec.e_cust));
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
		{
			print_err ("Customer %s is not on file", local_rec.e_cust);
			return 1;
		}
		if (strcmp (local_rec.s_cust, local_rec.e_cust) > 0)
		{
			print_err ("End Customer %s may not be less than %s", local_rec.e_cust, local_rec.s_cust);
			return 1;
		}
		strcpy (local_rec.cust_name [1], cumr_rec.dbt_name);
		DSP_FLD ("name2");
		return 0;
	}

	if (LCHECK ("lpno"))
	{
		if (SRCH_KEY)
		{
			local_rec.lpno = get_lpno (0);
			return 0;
		}

		if (!valid_lp (local_rec.lpno))
		{
			print_err ("Invalid Printer Number.");
			return 1;
		}

		return 0;
	}

	if (LCHECK ("back"))
	{
		strcpy (local_rec.back, (local_rec.back [0] == 'Y') ? "Y(es" : "N(o ");
		DSP_FLD ("back");
		return 0;
	}

	if (LCHECK ("onight"))
	{
		strcpy (local_rec.onite, (local_rec.onite [0] == 'Y') ? "Y(es" : "N(o ");
		DSP_FLD ("onight");
		return 0;
	}

	return 0;
}

static void
PrintGroupTotal (
 FILE*              fout)
{
	int     i;

	strcpy (wk_amt [0], comma_fmt (DOLLARS (group_tot [0]), "NN,NNN,NNN.NN"));
	strcpy (wk_amt [1], comma_fmt (DOLLARS (group_tot [1]), "NN,NNN,NNN.NN"));
	strcpy (wk_amt [2], comma_fmt (DOLLARS (group_tot [2]), "NN,NNN,NNN.NN"));

	fprintf (fout, "|            |  |  |          |     | TOTAL FOR %9.9s |%13.13s |        |          |%13.13s |%13.13s |\n",
			 "GROUP",
			 wk_amt [0], wk_amt [1], wk_amt [2]);
	PrintLine (fout);

	for (i = 0; i < 4; i++)
	{
		cust_tot [i] += group_tot [i];
		group_tot [i] = 0.00;
	}
}

static void
PrintCustomerTotal (
 FILE*              fout)
{                                     
	int i;
	
	strcpy (wk_amt [0], comma_fmt (DOLLARS (cust_tot [0]), "NN,NNN,NNN.NN"));
	strcpy (wk_amt [1], comma_fmt (DOLLARS (cust_tot [1]), "NN,NNN,NNN.NN"));
	strcpy (wk_amt [2], comma_fmt (DOLLARS (cust_tot [2]), "NN,NNN,NNN.NN"));

	fprintf (fout, "|            |  |  |          |     | TOTAL FOR %9.9s |%13.13s |        |          |%13.13s |%13.13s |\n",
			 "CUSTOMER.",
			 wk_amt [0], wk_amt [1], wk_amt [2]);

	PrintLine (fout);

	for (i = 0; i < 4; i++)
	{
		grand_tot [i] += cust_tot [i];
		cust_tot [i] = 0.00;
	}
}

static void
PrintGrandTotal (FILE *fout)
{
	strcpy (wk_amt [0], comma_fmt (DOLLARS (grand_tot [0]), "NN,NNN,NNN.NN"));
	strcpy (wk_amt [1], comma_fmt (DOLLARS (grand_tot [1]), "NN,NNN,NNN.NN"));
	strcpy (wk_amt [2], comma_fmt (DOLLARS (grand_tot [2]), "NN,NNN,NNN.NN"));

	fprintf (fout, "|            |  |  |          |     | TOTAL FOR %9.9s |%13.13s |        |          |%13.13s |%13.13s |\n",
			 "GRAND.   ",
			 wk_amt [0], wk_amt [1], wk_amt [2]);

	PrintLine (fout);

	grand_tot [0] = 0.00;
	grand_tot [1] = 0.00;
	grand_tot [2] = 0.00;
}

/*=========================================================
| Based on so_cc_disp's proc_invoices () and get_invoices ()
=========================================================*/
static void
PrintInvoiceDetails (
 FILE*              fout,
 DGroup*            pGroup,
 int                is_highlighted)
{
	char    inv_date [11],
			post_date [11],
			due_date [11];
	char    tem_line [200];
	double  balance;
	DGroupItem *pItem;
	int     paid_flag = FALSE;
	char    doc_no [9],
			doc_date [11];

	if (CREDIT)
		balance = (envVarCnNettUsed) ? cuin_rec.amt - cuin_rec.disc
			: cuin_rec.amt;
	else
		balance = (envVarDbNettUsed) ? cuin_rec.amt - cuin_rec.disc
			: cuin_rec.amt;

	strcpy (inv_date , 	DateToString (cuin_rec.date_of_inv));
	strcpy (post_date , DateToString (cuin_rec.date_posted));
	strcpy (due_date , 	DateToString (cuin_rec.due_date));

	sprintf (tem_line, "%s %s|%s|%s|%-10.10s| %s |%-10.10s|%-10.10s|%13.13s |",
			 tran_type [atoi (cuin_rec.type) - 1],
			 cuin_rec.inv_no,
			 cuin_rec.est,
			 cuin_rec.dp,
			 inv_date ,
			 cuin_rec.pay_terms,
			 due_date,
			 post_date ,
			 comma_fmt (DOLLARS (balance), "NN,NNN,NNN.NN"));

	group_tot [0] += balance;

	strcpy (doc_no, "NONE    ");
	doc_date [0] = '\0';
	strcpy (wk_amt [1], "");

	for (pItem = GetFirstGroupItem (pGroup);
		 pItem != NULL;
		 pItem = GetNextGroupItem (pItem))
	{
		if (pItem->source == DG_cuhd)
		{
			int     fwd_payment = (pItem->date > comm_rec.dbt_date);

			cudt_rec.hhcp_hash = pItem->hhcp_hash;
			cudt_rec.hhci_hash = cuin_rec.hhci_hash;
			for (cc = find_rec (cudt, &cudt_rec, GTEQ, "r");
				 !cc &&
				 cudt_rec.hhcp_hash == pItem->hhcp_hash &&
				 cudt_rec.hhci_hash == cuin_rec.hhci_hash;
				 cc = find_rec (cudt, &cudt_rec, NEXT, "r"))
			{
				if (paid_flag)
				{
					fprintf (fout, "|%s%s%8.8s|%10.10s|%13.13s |%14s|\n",
							 is_highlighted ? "*" : " ",
							 tem_line,
							 doc_no,
							 doc_date,
							 wk_amt [1],
							 fwd_payment ? "*" : " ");

					sprintf (tem_line, "           |  |  |          |     |          |          |              |");
				}
				else
					paid_flag = TRUE;

				group_tot [1] += cudt_rec.amt_paid_inv;
				balance -= cudt_rec.amt_paid_inv;

				sprintf (doc_no, "%8.8s", pItem->doc_no);
				strcpy (doc_date, DateToString (pItem -> date));

				strcpy (wk_amt [1], comma_fmt (DOLLARS (cudt_rec.amt_paid_inv), "NN,NNN,NNN.NN"));
			}
		}
	}

	fprintf (fout, "|%s%s%8.8s|%-10.10s|%13.13s |%13.13s |\n",
			 is_highlighted ? "*" : " ",
			 tem_line,
			 doc_no,
			 doc_date,
			 wk_amt [1],
			 comma_fmt (DOLLARS (balance), "NN,NNN,NNN.NN"));

	group_tot [2] += balance;
}

static int
ContainsCustTransactions (
 DGroup*            pGroup)
{
	DGroupItem *pItem;

	for (pItem = GetFirstGroupItem (pGroup);
		 pItem != NULL;
		 pItem = GetNextGroupItem (pItem))
	{
		if (pItem->child_hhcu == local_rec.hhcu_hash)
			return TRUE;
	}
	return FALSE;
}

static void
PrintLine (
 FILE*              fout)
{
	fprintf (fout, "|------------");
	fprintf (fout, "|--");
	fprintf (fout, "|--");
	fprintf (fout, "|----------");
	fprintf (fout, "|-----");
	fprintf (fout, "|----------");
	fprintf (fout, "|----------");
	fprintf (fout, "|--------------");
	fprintf (fout, "|--------");
	fprintf (fout, "|----------");
	fprintf (fout, "|--------------");
	fprintf (fout, "|--------------|\n");
}

static void
HeadingOutput (
 FILE*              fout)
{
	char    dbt_no [sizeof cumr_rec.dbt_no];
	char    acronym [sizeof cumr_rec.dbt_acronym];
	char    name [sizeof cumr_rec.dbt_name];

	fprintf (fout, ".16\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L140\n");
	fprintf (fout, ".B1\n");
	fprintf (fout, ".E%s\n", clip (comm_rec.co_name));
	fprintf (fout, ".B1\n");

	strcpy (dbt_no, cumr_rec.dbt_no);
	clip (dbt_no);
	strcpy (acronym, cumr_rec.dbt_acronym);
	clip (acronym);
	strcpy (name, cumr_rec.dbt_name);
	clip (name);

	fprintf (fout, ".ECustomer: %s (%s) %s\n", dbt_no, acronym, name);
	fprintf (fout, ".EStatement Type: %s\n",
			 (OPEN_ITEM ? "O (pen)" : "B (alance)"));
	fprintf (fout, ".EMinimum Age for Purge: %ld days\n", envVarPurgeMon);
	fprintf (fout, ".EMinimum Purge Date: %s\n", min_purge_date);
	fprintf (fout, ".EAS AT %-24.24s\n", SystemTime ());
	fprintf (fout, ".B1\n");

	fprintf (fout, ".R=============");
	fprintf (fout, "===");
	fprintf (fout, "===");
	fprintf (fout, "===========");
	fprintf (fout, "======");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "===============");
	fprintf (fout, "=========");
	fprintf (fout, "===========");
	fprintf (fout, "===============");
	fprintf (fout, "================\n");

	fprintf (fout, "=============");
	fprintf (fout, "===");
	fprintf (fout, "===");
	fprintf (fout, "===========");
	fprintf (fout, "======");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "===============");
	fprintf (fout, "=========");
	fprintf (fout, "===========");
	fprintf (fout, "===============");
	fprintf (fout, "================\n");

	fprintf (fout, "|   INVOICE  ");
	fprintf (fout, "|BR");
	fprintf (fout, "|DP");
	fprintf (fout, "| DATE  OF ");
	fprintf (fout, "| PAY ");
	fprintf (fout, "| INVOICE  ");
	fprintf (fout, "|   DATE   ");
	fprintf (fout, "|    AMOUNT    ");
	fprintf (fout, "| CHEQUE ");
	fprintf (fout, "|  CHEQUE  ");
	fprintf (fout, "|     AMOUNT   ");
	fprintf (fout, "|    BALANCE   |\n");

	fprintf (fout, "|   NUMBER   ");
	fprintf (fout, "|NO");
	fprintf (fout, "|NO");
	fprintf (fout, "|  INVOICE ");
	fprintf (fout, "|TERMS");
	fprintf (fout, "| DUE DATE ");
	fprintf (fout, "|  POSTED  ");
	fprintf (fout, "|   (DEBIT)    ");
	fprintf (fout, "| NUMBER ");
	fprintf (fout, "|   DATE   ");
	fprintf (fout, "|    (CREDIT)  ");
	fprintf (fout, "|    DUE       |\n");

	PrintLine (fout);
}

/*==========================
| Process Selected Customers
==========================*/
static void
ProcessCustomers (void)
{
	if (OpenStatementGroups (comm_rec.dbt_date))
	{
		FILE   *fout;
		int		first = TRUE;

		if ( (fout = popen ("pformat", "w")) == NULL)
			sys_err ("Error in pformat During (POPEN)", errno, PNAME);

		fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
		fprintf (fout, ".LP%d\n", local_rec.lpno);

		strcpy (cumr_rec.co_no, comm_rec.co_no);
		strcpy (cumr_rec.est_no, branchNumber);
		strcpy (cumr_rec.dbt_no, local_rec.s_cust);
		for (cc = find_rec (cumr, &cumr_rec, GTEQ, "r");
			 (!cc &&
			  !strcmp (cumr_rec.co_no, comm_rec.co_no) &&
			  (envVarDbFind || !strcmp (cumr_rec.est_no, branchNumber)) &&
			  strcmp (cumr_rec.dbt_no, local_rec.s_cust) >= 0 &&
			  strcmp (cumr_rec.dbt_no, local_rec.e_cust) <= 0);
			 cc = find_rec (cumr, &cumr_rec, NEXT, "r"))
		{                      
			local_rec.hhcu_hash = cumr_rec.hhcu_hash;
			
			is_head = (cumr_rec.hhcu_hash == cumr_rec.ho_dbt_hash ||
					   !cumr_rec.ho_dbt_hash);

			if (is_head)
			{
				PrintGroups (fout, first);
				first = FALSE;
			}
		}

		PrintGrandTotal (fout);
		
		fprintf (fout, ".EOF\n");
		pclose (fout);

		CloseStatementGroups ();
	}
}

/*=======================
| Print Group results
=======================*/
static void
PrintGroups (
 FILE*              fout,
 int                first)
{
    DGroup	*pGroup;
    int		i;
	
	assert (local_rec.hhcu_hash == cumr_rec.hhcu_hash);
	
	HeadingOutput (fout);

	if (!first)
		fprintf (fout, ".PA\n"); /* start each cust on new page */

	LoadStatementGroups (
			&GroupSet, 
			is_head ? cumr_rec.hhcu_hash : cumr_rec.ho_dbt_hash, 
			envVarCoClose ? comr_rec.stmt_date : esmr_rec.stmt_date, 
			 (_DG_PURGE | _DG_GROUPBYPARENT | _DG_INCLUDEZEROS),
			NULL);
    
	for (i = 0; i < 4; i++)
	{
		group_tot [i] = 0.00;
		cust_tot [i] = 0.00;
		grand_tot [i] = 0.00;
	}

	/*-----------------------------------------------------------------
	| Display transactions by group so that the user can see which
	| transactions form part of the group, which transactions have 
	| individual outstanding totals and what the outstanding group
	| total balance is.
	| If the selected customer is a head office account then all child
	| account transactions are included.
	| If the selected customer is a child account then transactions
	| that are linked to other child accounts are highlighted. This
	| is implemented by loading and grouping all transactions for the
	| head office account, excluding groups that do not contain any
	| transactions for the selected child customer, and highlighting 
	| transactions that do not belong to the child customer.
	-----------------------------------------------------------------*/
	for (pGroup = GetFirstGroup (&GroupSet);
		 pGroup != NULL;
		 pGroup = GetNextGroup (pGroup))
	{       
		if (is_head || ContainsCustTransactions (pGroup))
		{
			DGroupItem	*pItem;
			
			for (pItem = GetFirstGroupItem (pGroup);
				 pItem != NULL;
				 pItem = GetNextGroupItem (pItem))
			{                          
				if (pItem->source == DG_cuin)
				{                            
					int highlight_flag = FALSE;
					
					cc = find_hash (cuin, &cuin_rec, EQUAL, "r", pItem->hhci_hash);
					if (cc)
						file_err (cc, "cuin", "DBFIND");

					if (!is_head)
					{
						highlight_flag = 
							 (pItem->child_hhcu != local_rec.hhcu_hash);
					}
					
					PrintInvoiceDetails (fout, pGroup, highlight_flag);
				}
			}
		
			PrintGroupTotal (fout);
		}
	}
	PrintCustomerTotal (fout);

	FreeStatementGroups (&GroupSet);
}

int
heading (
 int                scn)
{
	if (!restart)
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();
		centre_at (0, 80, "%R  Print Customers Groupings ");
		move (0, 1);
		line (80);

		box (0, 3, 80, 5);
		move (1, 6);
		line (79);

		move (0, 20);
		line (80);
		move (0, 21);
		printf (" Co. : %s : Br : %s %s ", comm_rec.co_no, comm_rec.est_no, comm_rec.est_name);
		move (0, 22);
		line (80);
		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}

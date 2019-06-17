/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: db_group_dsp.c,v 5.2 2002/07/16 01:04:20 scott Exp $
|  Program Name  : (db_group_dsp.c)
|  Program Desc  : (Display customer group transactions)
|---------------------------------------------------------------------|
|  Author        : Basil Wood      | Date Written  : 05/04/96         |
|---------------------------------------------------------------------|
| $Log: db_group_dsp.c,v $
| Revision 5.2  2002/07/16 01:04:20  scott
| Updated from service calls and general maintenance.
|
| Revision 5.1  2001/12/07 03:45:47  scott
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: db_group_dsp.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/DB/db_group_dsp/db_group_dsp.c,v 5.2 2002/07/16 01:04:20 scott Exp $";

#include <pslscr.h>
#include <twodec.h>
#include <dbgroup.h>
#include <ml_std_mess.h>

#include <assert.h>
#include <DateToString.h>

#define	CAL(amt,pc)		(amt * DOLLARS(pc))

#define	INVOICE		(cuin_rec.type [0] == '1')
#define	CREDIT		(cuin_rec.type [0] == '2')
#define	OPEN_ITEM	(cumr_rec.stmt_type [0] == 'O')

static DGroupSet GroupSet;

#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct cudtRecord	cudt_rec;
struct cuinRecord	cuin_rec;
struct cumrRecord	cumr_rec;
struct esmrRecord	esmr_rec;

static char
       *data = "data",
       *cumrhh = "cumrhh";

int     display_ok,
        line_printed = FALSE,
        envVarCoClose = TRUE,
        envVarDbCo = 0,
        envVarDbFind = 0,
        clear_ok = TRUE;
        
char    branchNumber [3],
        wk_amt [4] [15];
                       
int     envVarDbNettUsed = TRUE,
        envVarCnNettUsed = TRUE;

char    disp_str [300];

double  group_tot [4],
        grand_tot [4];

static char *tran_type [] =
{
	"IN", "CR", "JL", "CH", "JL", "?"
};

static int     is_head;

/*============================
| Local & Screen Structures. |
============================*/
struct
{
	char    dbt_no [sizeof cumr_rec.dbt_no];
	long    hhcu_hash;
}
local_rec;

static struct var vars [] =
{
	{1, LIN, "dbt_no", 2, 18, CHARTYPE,
	"UUUUUU", "          ",
	" ", "0", "Customer       :", " ",
	YES, NO, JUSTLEFT, "", "", local_rec.dbt_no},

	{0}
};

#include <get_lpno.h>
#include <FindCumr.h>


 /* Initialize multi-lingual strings
  * Just make sure that the entries in LScreen match the indices listed
  *  in LScreenIdx
  */
enum LScreenIdx
{
	LTITLE,	LCHILD,	LACRO,	LNAME,	LMINA,
	LMINP,	LTYPE,	LOTYPE,	LBTYPE,	LCOMP,
	LBRAN
};

static char * LScreen [] =
{
	"C u s t o m e r  G r o u p i n g   D i s p l a y",	/* LTITLE */	
	"(Child)",											/* LCHILD */
	"Acronym:",											/* LACRO */
	"Name:",											/* LNAME */
	"Minimum Age for Purge:",							/* LMINA */
	"Minimum Purge Date:",								/* LMINP */
	"Statement Type:",									/* LTYPE */
	"O(pen)",											/* LOTYPE */
	"B(alance)",										/* LBTYPE */
	"Company :",										/* LCOMP */
	"Branch :",											/* LBRAN */
};


/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
static 	int 	ContainsCustTransactions 	(DGroup *);
static 	void 	OpenDB 						(void);
static 	void 	CloseDB 					(void);
static 	void 	re_draw 					(void);
static 	void 	DisplayGroupTotal 			(void);
static 	void 	DisplayGrandTotal 			(void);
static 	void 	DisplayInvoiceDetails 		(DGroup *, int i);
static 	void 	DisplayGroups 				(void);
static 	void 	DisplayCustomer 			(void);
int 	spec_valid 							(int);
int 	heading 							(int);
int 	Dsp_heading 						(void);
int 	head_print 							(void);

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int                argc,
 char*              argv [])
{
	char   *sptr;

	switch (argc)
	{
	case 1:
		local_rec.hhcu_hash = 0L;
		break;
	case 2:
		local_rec.hhcu_hash = atol (argv [1]);
		break;
	default:
		printf ("Usage : %s <hhcu_hash>\007\n", argv [0]);
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

	input_row = 2,
		error_line = 20;

	init_scr ();				/*  sets terminal from termcap  */
	set_tty ();
	swide ();
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
	
	/*-------------------------------
	| Read other records
	-------------------------------*/
	if (envVarCoClose)
	{
		strcpy(comr_rec.co_no, comm_rec.co_no);
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

	if (OpenStatementGroups (comm_rec.dbt_date))
	{
		if (!local_rec.hhcu_hash)
		{
			prog_exit = FALSE;
			while (!prog_exit)
			{                 
				entry_exit 	= FALSE;
				prog_exit 	= FALSE;
				restart 	= FALSE;
				display_ok 	= FALSE;
				search_ok 	= TRUE;
				clear_ok 	= TRUE;
				init_vars (1);
			
				heading (1);
				entry (1);
				if (prog_exit || restart)
					continue;
			
				display_ok = TRUE;	/* Display results */
				clear_ok = FALSE;
				heading (1);
				DisplayGroups ();
				crsr_on ();
			}
		}
		else
		{
			display_ok = TRUE;
			clear_ok = TRUE;
			heading (1);
			DisplayGroups ();
		} 
		CloseStatementGroups();
	}
	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

/*========================
| Open data base files . |
========================*/
static void
OpenDB (void)
{
	abc_dbopen (data);

	abc_alias (cumrhh, cumr);

	open_rec (cumr, cumr_list, CUMR_NO_FIELDS,
			  (!envVarDbFind) ? "cumr_id_no" : "cumr_id_no3");
	open_rec (cumrhh, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (cudt, cudt_list, CUDT_NO_FIELDS, "cudt_id_no");
	open_rec (cuin, cuin_list, CUIN_NO_FIELDS, "cuin_hhci_hash");

	if (envVarCoClose)
		open_rec (comr,  comr_list, COMR_NO_FIELDS, "comr_co_no");
	else
		open_rec (esmr,  esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
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
	/*-------------------------
	| Validate Customer number. |
	-------------------------*/
	if (LCHECK ("dbt_no"))
	{
		if (SRCH_KEY)
		{
			CumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (cumr_rec.co_no, comm_rec.co_no);
		strcpy (cumr_rec.est_no, branchNumber);
		strcpy (cumr_rec.dbt_no, pad_num (local_rec.dbt_no));

		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess021));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		local_rec.hhcu_hash = cumr_rec.hhcu_hash;
		
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

static void
re_draw (void)
{
	clear_ok = TRUE;
	heading (1);
}

static void
DisplayGroupTotal (void)
{
	int     i;

	strcpy (wk_amt [0], comma_fmt (DOLLARS (group_tot [0]), "NN,NNN,NNN.NN"));
	strcpy (wk_amt [1], comma_fmt (DOLLARS (group_tot [1]), "NN,NNN,NNN.NN"));
	strcpy (wk_amt [2], comma_fmt (DOLLARS (group_tot [2]), "NN,NNN,NNN.NN"));

	sprintf (disp_str, "            ^E  ^E  ^E          ^E     TOTAL FOR %-9.9s   ^E%13.13s ^E        ^E          ^E%13.13s ^E%13.13s ",
			 "GROUP",
			 wk_amt [0], wk_amt [1], wk_amt [2]);

	Dsp_saverec (disp_str);

	Dsp_saverec ("^^GGGGGGGGGGGGEGGEGGEGGGGGGGGGGEGGGGGEGGGGGGGGGGEGGGGGGGGGGEGGGGGGGGGGGGGGEGGGGGGGGEGGGGGGGGGGEGGGGGGGGGGGGGGEGGGGGGGGGGGGGG");
	for (i = 0; i < 4; i++)
	{
		grand_tot [i] += group_tot [i];
		group_tot [i] = 0.00;
	}
}

static void
DisplayGrandTotal (void)
{
	strcpy (wk_amt [0], comma_fmt (DOLLARS (grand_tot [0]), "NN,NNN,NNN.NN"));
	strcpy (wk_amt [1], comma_fmt (DOLLARS (grand_tot [1]), "NN,NNN,NNN.NN"));
	strcpy (wk_amt [2], comma_fmt (DOLLARS (grand_tot [2]), "NN,NNN,NNN.NN"));

	sprintf (disp_str, "            ^E  ^E  ^E          ^E     GRAND TOTAL           ^E%13.13s ^E        ^E          ^E%13.13s ^E%13.13s ",
			 wk_amt [0], wk_amt [1], wk_amt [2]);

	Dsp_saverec (disp_str);

	Dsp_saverec ("^^GGGGGGGGGGGGJGGJGGJGGGGGGGGGGJGGGGGJGGGGGGGGGGJGGGGGGGGGGJGGGGGGGGGGGGGGJGGGGGGGGJGGGGGGGGGGJGGGGGGGGGGGGGGJGGGGGGGGGGGGGG");

	grand_tot [0] = 0.00;
	grand_tot [1] = 0.00;
	grand_tot [2] = 0.00;
}

/*=========================================================
| Based on so_cc_disp's proc_invoices() and get_invoices()
=========================================================*/
static void
DisplayInvoiceDetails (
 DGroup*            pGroup,
 int                is_highlighted)
{              
	char		inv_date [11],
				post_date [11];
	char		tem_line [200];
	char		env_line [200];
	double		balance;
	char		pay_date [11];
	DGroupItem	*pItem;
	int			paid_flag = FALSE;
	char		doc_no [9],
				doc_date [11];
		
	if (CREDIT)
		balance = (envVarCnNettUsed) ? cuin_rec.amt - cuin_rec.disc
			: cuin_rec.amt;
	else
		balance = (envVarDbNettUsed) ? cuin_rec.amt - cuin_rec.disc
			: cuin_rec.amt;

	strcpy (inv_date, DateToString (cuin_rec.date_of_inv));
	strcpy (post_date, DateToString (cuin_rec.date_posted));
	strcpy (pay_date, DateToString (cuin_rec.due_date));

	sprintf (tem_line, "%s %s^E%s^E%s^E%10.10s^E %s ^E%s^E%s^E%13.13s ^E",
			 tran_type [atoi (cuin_rec.type) - 1],
			 cuin_rec.inv_no,
			 cuin_rec.est,
			 cuin_rec.dp,
			 inv_date,
			 cuin_rec.pay_terms,
			 pay_date,
			 post_date,
			 comma_fmt (DOLLARS (balance), "NN,NNN,NNN.NN"));

	group_tot [0] += balance;

	strcpy (doc_no, "NONE    ");
	doc_date [0] = '\0';

	strcpy (wk_amt [1], "");

	for (pItem = GetFirstGroupItem (pGroup);
		 pItem != NULL;
		 pItem = GetNextGroupItem(pItem))
	{
		if (pItem->source == DG_cuhd)
		{
			int fwd_payment = (pItem->date > comm_rec.dbt_date);
		
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
					sprintf (env_line, "%s%s%8.8s^E%10.10s^E%13.13s ^E%14s",
							is_highlighted ? "*" : " ",
							tem_line,
							doc_no,
							doc_date,
							wk_amt [1],
							fwd_payment ? "*" : " ");
					Dsp_saverec (env_line);

					strcpy (tem_line, "           ^E  ^E  ^E          ^E     ^E          ^E          ^E              ^E");
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

	sprintf (env_line, "%s%s%8.8s^E%10.10s^E%13.13s ^E%13.13s ",
			 is_highlighted ? "*" : " ",
			 tem_line,
			 doc_no,
			 doc_date,
			 wk_amt [1],
			 comma_fmt (DOLLARS (balance), "NN,NNN,NNN.NN"));

	Dsp_saverec (env_line);

	group_tot [2] += balance;
}

static int
ContainsCustTransactions (
 DGroup *           pGroup)
{
	DGroupItem	*pItem;
			
	for (pItem = GetFirstGroupItem (pGroup);
		 pItem != NULL;
		 pItem = GetNextGroupItem(pItem))
	{            
		if (pItem->child_hhcu == local_rec.hhcu_hash)
			return TRUE;              
	}
	return FALSE;
}

/*=======================
| Display Group results
=======================*/
static void
DisplayGroups (void)
{
    DGroup	*pGroup;
    int		i;
	
	assert (local_rec.hhcu_hash == cumr_rec.hhcu_hash);
	
	sprintf (disp_str, "Customer %s (%s)",
			 	cumr_rec.dbt_no, cumr_rec.dbt_name);

	Dsp_prn_open (3, 5, 12, disp_str, comm_rec.co_no, comm_rec.co_name, comm_rec.est_no, comm_rec.est_name, NULL, NULL);
	Dsp_saverec ("   INVOICE  |BR|DP| DATE  OF | PAY | INVOICE  |   DATE   |    AMOUNT    | CHEQUE |  CHEQUE  |     AMOUNT   |    BALANCE   ");
	Dsp_saverec ("   NUMBER   |NO|NO| INVOICE  |TERMS| DUE DATE |  POSTED  |   (DEBIT)    | NUMBER |   DATE   |    (CREDIT)  |    DUE       ");
	Dsp_saverec (" [PRINT] [NEXT] [PREVIOUS] [INPUT/END]");

	LoadStatementGroups (
			&GroupSet, 
			is_head ? cumr_rec.hhcu_hash : cumr_rec.ho_dbt_hash, 
			envVarCoClose ? comr_rec.stmt_date : esmr_rec.stmt_date, 
			(_DG_PURGE | _DG_GROUPBYPARENT | _DG_INCLUDEZEROS),
			NULL);
    
	for (i = 0; i < 4; i++)
	{
		group_tot [i] = 0.00;
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
				 pItem = GetNextGroupItem(pItem))
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
					
					DisplayInvoiceDetails (pGroup, highlight_flag);
				}
			}
		
			DisplayGroupTotal ();
		}
	}
	DisplayGrandTotal ();

	Dsp_srch ();
	Dsp_close ();
	
	FreeStatementGroups (&GroupSet);
}

static void	
DisplayCustomer (void)
{   
	long purge_mon = atol (get_env ("PURGE_MON"));
	char min_purge_date [11];

	cc = find_hash (cumrhh, &cumr_rec, EQUAL, "r", local_rec.hhcu_hash);
	if (cc)
		file_err(cc, "cumrhh", "DBFIND");

	strcpy (local_rec.dbt_no, cumr_rec.dbt_no);
	DSP_FLD ("dbt_no");

	is_head = (cumr_rec.hhcu_hash == cumr_rec.ho_dbt_hash ||
			   !cumr_rec.ho_dbt_hash);

	sprintf (min_purge_date, "%-10.10s", 
				DateToString (comm_rec.dbt_date - purge_mon));

	print_at (2, 30, "%7s", is_head ? "" : ML (LScreen [LCHILD]));
	print_at (2, 40, "%s  %s", ML (LScreen [LACRO]), cumr_rec.dbt_acronym);
	print_at (2, 80, "%s  %s", ML (LScreen [LNAME]), cumr_rec.dbt_name);

	print_at (4, 10, "%s %ld", ML (LScreen [LMINA]), purge_mon);
	print_at (4, 50, "%s %s", ML (LScreen [LMINP]), min_purge_date);
	print_at (4, 90, "%s %s", ML (LScreen [LTYPE]), (OPEN_ITEM ? ML(LScreen [LOTYPE]) :
							 ML (LScreen [LBTYPE])));

	box (0, 1, 130, 3);
}

int
heading (
 int                scn)
{
	char	tempStr [64];

	if (!restart)
	{
		if (clear_ok)
			FLD ("dbt_no") = YES;
		else
			FLD ("dbt_no") = NA;

		scn_set (scn);

		if (clear_ok)
		{
			swide ();
			clear ();
		}

		if (scn == 1)
		{
			crsr_off ();
			strcpy (tempStr, ML (LScreen [LTITLE]));
			rv_pr (tempStr,40, 0, 1);

			box (0, 1, 130, 1);

			move (0, 22); line (130);

			move (0, 23);
			printf ("%s %s %s  /  %s %s %s", ML (LScreen [LCOMP]), comm_rec.co_no, comm_rec.co_short, ML(LScreen [LBRAN]), comm_rec.est_no, comm_rec.est_short);
			line_cnt = 0;
		}
		scn_write (scn);
	}

	if (display_ok)
	{
		DisplayCustomer ();
		clear_ok = FALSE;
	}
	return 0;
}

int
Dsp_heading (void)
{
	re_draw ();
	return 0;
}

int
head_print (void)
{
	Dsp_print ();

	clear_ok = TRUE;
	heading (1);
    return (EXIT_SUCCESS);
}

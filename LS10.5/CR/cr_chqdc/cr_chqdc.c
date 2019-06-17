/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: cr_chqdc.c,v 5.4 2001/09/24 06:16:41 robert Exp $
|  Program Name  : (cr_chqdc.c) 
|  Program Desc  : (Direct Credit Schedule)
|---------------------------------------------------------------------|
|  Author        : Jonathan Chen   | Date Written : 07/05/93          |
|---------------------------------------------------------------------|
| $Log: cr_chqdc.c,v $
| Revision 5.4  2001/09/24 06:16:41  robert
| Updated to avoid overlapping description
|
| Revision 5.3  2001/08/20 23:15:44  scott
| Updated for development related to bullet proofing
|
| Revision 5.2  2001/08/09 08:51:36  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:01:13  scott
| RELEASE 5.0
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cr_chqdc.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CR/cr_chqdc/cr_chqdc.c,v 5.4 2001/09/24 06:16:41 robert Exp $";

#include <pslscr.h>
#include <ml_std_mess.h>
#include <ml_cr_mess.h>

#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct suhdRecord	suhd_rec;
struct sumrRecord	sumr_rec;
struct pocrRecord	pocr_rec;
struct crbkRecord	crbk_rec;

/*==========
 Table names
============*/
static char
	*data	= "data";

/*======
 Globals
========*/
#define	SCREENWIDTH	80

#define	PAYMETHOD_XFER	'T'

static char	branchNo [3];	/* Branch / branchNolishment default            */

static struct
{
	int	envDbCo,
		multcurr;
}	env;

static struct
{
	char	*lpno,
			*pid;
}	args;

extern	int	TruePosition;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	today [11];

	double	new_rate;
	long	xfer_date;
	char	crd_no [7];
	long	ref_no;
	char	order_by [10];
	char	order_by_value [2];
	char 	override_exch [2];

} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "xfer_date",	4, 2, EDATETYPE,
		"DD/DD/DD", "        ",
		" ", local_rec.today, "Transfer Date        ", "",
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.xfer_date},
	{1, LIN, "ref_no",	 5, 2, INTTYPE,
		"NNNNNNNNNNNNNN", "          ",
		"0", "", "Reference No         ", "Transfer Reference Number",
		 NE, NO,  JUSTRIGHT, "", "", (char *)&local_rec.ref_no},
	{1, LIN, "order_by",	 6, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Order By             ", "Enter : N)umber [default], A)cronym",
		 NE, NO,  JUSTLEFT, "", "", local_rec.order_by_value},
	{1, LIN, "order_by_desc",	 6, 30, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.order_by},
	{1, LIN, "bank_id",	 7, 2, CHARTYPE,
		"UUUUU", "          ",
		" ", "", "Bank Code            ", "Enter code or [SEARCH] key",
		 NE, NO,  JUSTLEFT, "", "", crbk_rec.bank_id},
	{1, LIN, "bk_name",	 7, 30, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", crbk_rec.bank_name},
	{1, LIN, "br_name",	 8, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Branch Name          ", " ",
		 NA, NO,  JUSTLEFT, "", "", crbk_rec.branch_name},
	{1, LIN, "acct_name",	 9, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Account Name         ", " ",
		 NA, NO,  JUSTLEFT, "", "", crbk_rec.acct_name},
	{1, LIN, "bank_no",	 10, 2, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", "", "Bank Number          ", " ",
		 NA, NO,  JUSTLEFT, "", "", crbk_rec.bank_no},
	{1, LIN, "bk_acno",	 10, 40, CHARTYPE,
		"AAAAAAAAAAAAAAA", "          ",
		" ", "", "     Account No.", " ",
		 NA, NO,  JUSTLEFT, "", "", crbk_rec.bank_acct_no},
	{1, LIN, "bk_curr",	12, 2, CHARTYPE,
		"UUU", "          ",
		" ", " ", "Currency Code        ", " ",
		 NA, NO,  JUSTLEFT, "", "", crbk_rec.curr_code},
	{1, LIN, "curr_desc",	12, 40, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", pocr_rec.description},
	{1, LIN, "ovr_exch",	13, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Override Exch Rate   ", "Enter Y(es) or default to N(o)",
		 NE, NO,  JUSTLEFT, "NY", "", local_rec.override_exch},
	{1, LIN, "new_rate",	13, 40, DOUBLETYPE,
		"NNNN.NNNNNNNN", "          ",
		" ", "1.0000", "Exchange Rate        ", "Default is current rate for currency ",
		 NE, NO, JUSTRIGHT, ".0001", "9999", (char *) &local_rec.new_rate},

	{0}
};

/*===========================
| Local function prototypes |
===========================*/
void	OpenDB		 (void);
void	CloseDB		 (void);
void	ReadComr	 (void);
int		spec_valid	 (int);
int		SrchCrbk	 (char *);
int		heading		 (int);
void	RunPrintOut	 (void);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char *	argv [])
{
	char	*sptr;
	if (argc != 3)
	{
		print_at (0,0, mlCrMess138, argv [0]);
		return (EXIT_FAILURE);
	}

	TruePosition	=	TRUE;

	/*=============
	| Handle args |
	=============*/
	args.lpno	= argv [1];
	args.pid	= argv [2];

	/*==========
	 Handle env
	============*/
	env.envDbCo = atoi (get_env ("CR_CO"));

	/*-------------------------
	| Multi-currency debtors. |
	-------------------------*/
	sptr = chk_env ("CR_MCURR");
	if (sptr)
		env.multcurr = (*sptr == 'Y' || *sptr == 'y') ? TRUE : FALSE;
	else
		env.multcurr = FALSE;

	strcpy (local_rec.today, DateToString (TodaysDate ()));

	SETUP_SCR (vars);

	/*------------------------------------------------------
	| Reset screen control if not multi-currency suppliers.|
	------------------------------------------------------*/
	if (!env.multcurr)
	{
		FLD ("bk_curr")		= ND;
		FLD ("curr_desc")	= ND;
		FLD ("ovr_exch")	= ND;
		FLD ("new_rate")	= ND;
	}

	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	init_scr ();
	set_tty ();
	set_masks ();
	init_vars (1);

	/*------------------------------
	| Init and read in common data |
	------------------------------*/
	OpenDB ();
	ReadComr ();

	strcpy (branchNo, env.envDbCo ? comm_rec.est_no : " 0");

	while (!prog_exit)
	{
		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		search_ok	= TRUE;
		init_vars (1);

		/*------------------------------
		| Reset default screen control.|
		------------------------------*/
		if (env.multcurr)
		{
			FLD ("ovr_exch") = YES;
			FLD ("new_rate") = NO;
		}

		/*------------------------------
		| Enter screen 1 linear input. |
		------------------------------*/
		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		heading (1);
		scn_display (1);
		edit (1);

		if (restart)
			continue;

		break;	/* user is proceeding with print */
	}

	CloseDB (); 
	FinishProgram ();

	if (!prog_exit)
		RunPrintOut ();	/* run print program */

	return (EXIT_SUCCESS);
}
/*======================
| Open database files. |
======================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (crbk, crbk_list, CRBK_NO_FIELDS, "crbk_id_no");
	open_rec (pocr, pocr_list, POCR_NO_FIELDS, "pocr_id_no");
	open_rec (suhd, suhd_list, SUHD_NO_FIELDS, "suhd_cheq_no");
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, "sumr_hhsu_hash");
}

/*=======================
| Close database files. |
=======================*/
void
CloseDB (
 void)
{
	abc_fclose (crbk);
	abc_fclose (pocr);
	abc_fclose (suhd);
	abc_fclose (sumr);

	abc_dbclose (data);
}

void
ReadComr (
 void)
{
	open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");
	strcpy (comr_rec.co_no, comm_rec.co_no);
	cc = find_rec (comr, (char *) &comr_rec, EQUAL, "r");

	if (cc)
		file_err (cc, comr, "DBFIND");
		
	abc_fclose (comr);
}

/*============================
| Special Validation Section.|
============================*/
int
spec_valid (
 int field)
{
	/*-------------
	 Validate Date
	---------------*/
	if (LCHECK ("xfer_date"))
	{
		if (local_rec.xfer_date < MonthStart (comm_rec.crd_date))
		   return print_err (ML (mlCrMess135));

		if (local_rec.xfer_date > MonthEnd (comm_rec.crd_date + 28))
		   return print_err (ML (mlCrMess136));

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("ref_no"))
	{
		char	ref_no [sizeof suhd_rec.cheq_no];

		sprintf (ref_no, "C%014ld", local_rec.ref_no);
		strcpy (suhd_rec.cheq_no, ref_no);

		cc = find_rec (suhd, (char *) &suhd_rec, GTEQ, "r");
		while (!cc && !strcmp (suhd_rec.cheq_no, ref_no))
		{
			sumr_rec.hhsu_hash = suhd_rec.hhsu_hash;
			cc = find_rec (sumr, (char *) &sumr_rec, EQUAL, "r");
			if (cc)
				file_err (cc, sumr, "DBFIND");

			if (!strcmp (sumr_rec.co_no, comm_rec.co_no))
				return (print_err (ML (mlCrMess137)));

			cc = find_rec (suhd, (char *) &suhd_rec, NEXT, "r");
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("order_by"))
	{
		strcpy (local_rec.order_by, *local_rec.order_by_value == 'N' ?
											"Number " : "Acronym");
		DSP_FLD ("order_by_desc");
		return (EXIT_SUCCESS);
	}

	/*-----------------------------------------
	| Validate Bank Id Code And Allow Search. |
	-----------------------------------------*/
	if (LCHECK ("bank_id"))
	{
		if (SRCH_KEY && !SrchCrbk (temp_str))
			return (EXIT_FAILURE);

		strcpy (crbk_rec.co_no, comm_rec.co_no);
		if (find_rec (crbk, (char *) &crbk_rec, EQUAL, "r"))
		{
			print_err (ML (mlStdMess043));
			return (EXIT_FAILURE);
		}

		DSP_FLD ("bk_name");
		DSP_FLD ("br_name");
		DSP_FLD ("acct_name");
		DSP_FLD ("bank_no");
		DSP_FLD ("bk_acno");
		DSP_FLD ("bk_acno");

		/*--------------------------------
		| Read Supplier Currency Record. |
		--------------------------------*/
		DSP_FLD ("bk_curr");
		strcpy (pocr_rec.co_no, comm_rec.co_no);
		strcpy (pocr_rec.code, crbk_rec.curr_code);
		cc = find_rec (pocr, (char *) &pocr_rec, COMPARISON, "r");
		if (cc)
		{
			print_err (ML (mlStdMess040));
			return (EXIT_FAILURE);
		}

		DSP_FLD ("curr_desc");

		local_rec.new_rate = pocr_rec.ex1_factor;
		DSP_FLD ("new_rate");

		return (EXIT_SUCCESS);
	}

	/*-------------------------------
	| Validate Exchange Override.   |
	-------------------------------*/
	if (LCHECK ("ovr_exch"))
	{
		if (!env.multcurr)
			return (EXIT_SUCCESS);

		if (local_rec.override_exch [0] == 'N')
		{
			local_rec.new_rate = pocr_rec.ex1_factor;
			if (!local_rec.new_rate)
			  	local_rec.new_rate = 1.0000;

			FLD ("new_rate") = NA;
			skip_entry = 1;
		}
		else
			FLD ("new_rate") = NO;

		DSP_FLD ("new_rate");
		return (EXIT_SUCCESS);
	}

	/*-------------------------------
	| Validate Exchange Rate.       |
	-------------------------------*/
	if (LCHECK ("new_rate"))
	{
		if (!env.multcurr)
			return (EXIT_SUCCESS);

		if (dflt_used)
			local_rec.new_rate = pocr_rec.ex1_factor;

		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*=========================================
| Search routine for Suppliers Bank File. |
=========================================*/
int
SrchCrbk (
 char *	key_val)
{
	work_open ();
	save_rec ("#Bank", "#Name");

	strcpy (crbk_rec.co_no, comm_rec.co_no);
	strcpy (crbk_rec.bank_id, key_val);
	cc = find_rec (crbk, (char *) &crbk_rec, GTEQ, "r");
	while (!cc &&
		!strncmp (crbk_rec.bank_id, key_val, strlen (key_val)) &&
		!strcmp (crbk_rec.co_no, comm_rec.co_no))
	{
		if (save_rec (crbk_rec.bank_id, crbk_rec.bank_name))
			break;
		cc = find_rec (crbk, (char *) &crbk_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return (FALSE);

	strcpy (crbk_rec.bank_id, temp_str);
	return (TRUE);
}

/*===============================================
| Screen Heading Display Routine.               |
===============================================*/
int
heading (
 int scn)
{
	if (!restart)
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();

		rv_pr (ML (mlCrMess134), (SCREENWIDTH - strlen (ML (mlCrMess134))) / 2, 0, 1);

		move (0, 1);
		line (SCREENWIDTH);

		move (1, input_row);
		if (env.multcurr)
		{
			move (0, 11);
			line (SCREENWIDTH);
			box (0, 3, SCREENWIDTH, 10);
		}
		else
			box (0, 3, SCREENWIDTH, 7);

		move (0, 20);
		line (SCREENWIDTH);

		strcpy (err_str, ML (mlStdMess038));
		print_at (21,0, err_str, comm_rec.co_no, clip (comm_rec.co_name));

		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}

void
RunPrintOut (
 void)
{
	char *	cmd = "cr_dc_prn";
	char	ref_no [sizeof suhd_rec.cheq_no],
			Xfer_date [20],
			rate [20];

	sprintf (Xfer_date, "%ld", local_rec.xfer_date);
	sprintf (rate, "%.4f", local_rec.new_rate);
	sprintf (ref_no, "C%014ld", local_rec.ref_no);
	if (execlp (cmd,
				cmd,
				args.lpno,
				args.pid,
				Xfer_date,
				ref_no,
				local_rec.order_by,
				crbk_rec.bank_id,
				rate,
				NULL) == -1)
	{
		sys_err ("!execlp ()", errno, PNAME);
	}
}

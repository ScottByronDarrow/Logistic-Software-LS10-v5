/*=====================================================================
|  Copyright (C) 1999 - 1999 Logistic Software Limited   .            |
|=====================================================================|
|  Program Name  : ( cr_dc_prn.c    )                                 |
|  Program Desc  : ( Direct Credit Print                            ) |
|                  (                                                ) |
|---------------------------------------------------------------------|
|  Access files  :  comm, sumr                                        |
|  Database      : (data)                                             |
|---------------------------------------------------------------------|
|  Updates Files :  N/A                                               |
|  Database      : (N/A )                                             |
|---------------------------------------------------------------------|
|  Date Written  : (11/05/93)      | Author      : Jonathan Chen      |
|---------------------------------------------------------------------|
|  Date Modified : (09/06/93)      | Modified by : Trevor van Bremen  |
|  Date Modified : (10/06/93)      | Modified by : Trevor van Bremen  |
|  Date Modified : (27/01/94)      | Modified by : Campbell Mander.   |
|  Date Modified : (30/11/95)      | Modified by : Scott B Darrow.    |
|  Date Modified : (06/04/96)      | Modified by : Scott B Darrow.    |
|  Date Modified : (20/05/97)      | Modified by : Scott B Darrow.    |
|  Date Modified : (15/09/97)      | Modified by : Rowena S Maandig   |
|  Date Modified : (14/09/1999)    | Modified by : Ramon A. Pacheco   |
|                                                                     |
|  Comments      :                                                    |
|  (09/06/93)    : PSL 9086. Fix for correct casting.                 |
|  (10/06/93)    : AMB 9104. introduced new env var CR_DCREMIT        |
|  (27/01/94)    : HGP 9846. Remove suhd_loc_disc_taken from dbview.  |
|  (30/11/95)    : PDL - Updated for new general ledger interface.    |
|                :       Program will work with 9 and 16 char accounts|
|                :                                                    |
|   (06/04/96)   : PDL - Updated to change cheque length from 6-8.    |
|   (20/05/97)   : PDL - Updated to change cheque length from 8-13    |
|   (15/09/97)   : Updated to incorporate multilingual conversion.    |
|   (14/09/1999) : Ported to ANSI standards.                          |
|                :                                                    |
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cr_dc_prn.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CR/cr_dc_prn/cr_dc_prn.c,v 5.2 2001/08/09 08:51:50 scott Exp $";

#define		NO_SCRGEN
#include 	<pslscr.h>
#include	<twodec.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include	<get_lpno.h>
#include 	<ml_std_mess.h>
#include 	<ml_cr_mess.h>

#include 	"schema"

	/*====================================
	| file comm	{System Common file} |
	====================================*/
	struct dbview comm_list [] =
	{
		{"comm_term"},
		{"comm_co_no"},
		{"comm_co_name"},
		{"comm_est_no"},
		{"comm_est_name"},
		{"comm_crd_date"},
	};

	int	comm_no_fields = 6;

	struct
	{
		int		termno;
		char	tco_no [3];
		char	tco_name [41];
		char	test_no [3];
		char	test_name [41];
		long	tcrd_date;
	} comm_rec;

	struct	sumrRecord	sumr_rec;
	struct	suinRecord	suin_rec;
	struct	crbkRecord	crbk_rec;
	struct	sudtRecord	sudt_rec;
	struct	suhdRecord	suhd_rec;

/*===========
 Table names
=============*/
static char
	*data	= "data";

	int		workFileNo			= 0;

/*======
 Globals
========*/
#define	PAYMETHOD_XFER	'T'

static char	*rem_prt_cmd = "cr_dcremtprn";

static char	branchNumber [3];

static struct
{
	int		lpno;
	long	pid;
	long	xferdate;
	char	*refno;
	char	*order_by;
	char	bank_id [6];
	double	rate;

}	args;

	struct {
		long	hhsuHash;
	} workFileRec;

static struct
{
	int	envDbCo,
		cr_remit,
		multcurr;
}	env;

/*===========================
| Local function prototypes |
===========================*/
void	OpenDB			(void);
void	CloseDB			(void);
void	ReadCrbk		(void);
FILE *	InitPrintOut	(void);
void	EndPrintOut		(FILE *, double);
void	PrintLine		(FILE *);
void	FlushToPrinter	(FILE *);
void	ProcessSumr		(void);
void	ProcessSuin		(void);
void	AddSuhd			(void);
void	AddSudt			(double);
int		SortByNumber	(void);
int		ValidSupp		(void);

/*==============
 The real thing!
================*/
int
main (
 int	argc,
 char *	argv [])
{
	FILE *pOut = NULL;	/* temp file for printout */

	/*============
	 Handle args
	=============*/
	if (argc != 8)
	{
		print_at (0, 0, mlCrMess700, argv [0]);
		return (EXIT_FAILURE);
	}

	/*=========
	 Load args
	==========*/
	args.lpno		= atoi (argv [1]);
	args.pid		= atol (argv [2]);
	args.xferdate	= atol (argv [3]);
	args.refno		= argv [4];
	args.order_by	= argv [5];
	sprintf (args.bank_id, "%.5s", argv [6]);
	sscanf (argv [7], "%lf", &args.rate);

	/*===========
	 Look at env
	============*/
	env.envDbCo = atoi (get_env ("CR_CO"));
	env.cr_remit = atoi (get_env ("CR_DCREMIT"));
	env.multcurr = *get_env ("CR_MCURR") == 'Y';

	set_tty ();

	/*=======
	 Init db
	========*/
	OpenDB ();
	read_comm (comm_list, comm_no_fields, (char *) &comm_rec);
	ReadCrbk ();

	strcpy (branchNumber, env.envDbCo ? comm_rec.test_no : " 0");

	memset (&sumr_rec, 0, sizeof (sumr_rec));
	strcpy (sumr_rec.co_no, comm_rec.tco_no);
	strcpy (sumr_rec.est_no, branchNumber);

	if (!(cc = find_rec (sumr, &sumr_rec, GTEQ, "u")))
	{
		double	total = 0;

		dsp_screen ("Direct Schedule Print",comm_rec.tco_no, comm_rec.tco_name);
		while (!cc &&
			!strcmp (sumr_rec.co_no, comm_rec.tco_no) &&
			!strcmp (sumr_rec.est_no, branchNumber))
		{
			if (ValidSupp ())
			{
				dsp_process ("Supplier : ", sumr_rec.crd_no);

				ProcessSumr ();

				if (!pOut)
					pOut = InitPrintOut ();
				PrintLine (pOut);
				total += suhd_rec.tot_amt_paid;

				/*------------------------------------------------------------
				| Add supplier hash to period recalc work file (cr_percalc). |
				------------------------------------------------------------*/
				workFileRec.hhsuHash = sumr_rec.hhsu_hash;
				cc = RF_ADD (workFileNo, (char *) &workFileRec);
				if (cc) 
					file_err (cc, "cr_per", "WKADD");

				strcpy (sumr_rec.stat_flag, "0");
				if ((cc = abc_update (sumr, &sumr_rec)))
					file_err (cc, "sumr", "!abc_update()");
			} else
				abc_unlock (sumr);
			cc = find_rec (sumr, &sumr_rec, NEXT, "u");
		}
		abc_unlock (sumr);
		EndPrintOut (pOut, total);

		FlushToPrinter (pOut);
	}

	CloseDB (); 
	FinishProgram ();

	if (pOut && env.cr_remit)
	{
		char	pidstr [10];

		sprintf (pidstr, "%ld", args.pid);

		execlp (rem_prt_cmd, pidstr, NULL);
	}

	return (EXIT_SUCCESS);
}

/*======================
| Open database Files. |
======================*/
void
OpenDB (
 void)
{
	char	filename [100];
	char 	*sptr = getenv ("PROG_PATH");

	sprintf (filename,"%s/WORK/cr_per%05ld", 
				 (sptr == (char *) 0) ? "/usr/LS10.5" : sptr, args.pid);

	cc = RF_OPEN (filename,sizeof (workFileRec),"w",&workFileNo);
	if (cc) 
		file_err (cc, "cr_per", "WKOPEN");

	abc_dbopen (data);
	open_rec (sudt, sudt_list, SUDT_NO_FIELDS, "sudt_hhsp_hash");
	open_rec (suhd, suhd_list, SUHD_NO_FIELDS, "suhd_id_no");
	open_rec (suin, suin_list, SUIN_NO_FIELDS, "suin_hhsu_hash");
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS,
		SortByNumber () ? "sumr_id_no" : "sumr_id_no2");
}

/*=======================
| Close database Files. |
=======================*/
void
CloseDB (
 void)
{
	cc = RF_CLOSE (workFileNo);
	if (cc != 0 && cc != 9)
		file_err (cc, "cr_percalc", "WKCLOSE");

	abc_fclose (sudt);
	abc_fclose (suhd);
	abc_fclose (suin);
	abc_fclose (sumr);
	abc_dbclose (data);
}

void
ReadCrbk (
 void)
{
	open_rec (crbk, crbk_list, CRBK_NO_FIELDS, "crbk_id_no");

	memset (&crbk_rec, 0, sizeof (crbk_rec));
	strcpy (crbk_rec.co_no, comm_rec.tco_no);
	strcpy (crbk_rec.bank_id, args.bank_id);
	if ((cc = find_rec (crbk, &crbk_rec, EQUAL, "r")))
		file_err (cc, "crbk", "!find_rec()");

	abc_fclose (crbk);
}

/*==================================
 Print files initialization and end
==================================*/
FILE *
InitPrintOut (
 void)
{
	FILE *	tOut = tmpfile ();

	if (!tOut)
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);

	/*---------------------------------
	 Start output to standard print
	---------------------------------*/
	fprintf (tOut, ".START%s<%s>\n", DateToString (comm_rec.tcrd_date), PNAME);
	fprintf (tOut, ".LP%d\n", args.lpno);
	fprintf (tOut, ".SO\n");
	fprintf (tOut, ".17\n");
	fprintf (tOut, ".PI12\n");
	fprintf (tOut, ".L120\n");
	fprintf (tOut, ".EDIRECT CREDIT PAYMENT SCHEDULE\n");
	fprintf (tOut, ".EAS AT : %s\n", SystemTime ());
	fprintf (tOut, ".B1\n");
	fprintf (tOut, ".ECompany %s : %s\n", comm_rec.tco_no, clip (comm_rec.tco_name));
	fprintf (tOut,".B1\n");
	fprintf (tOut,".CDIRECT CREDIT REFERENCE :%40.40s\n",args.refno);
	fprintf (tOut,".CBANK                    :%40.40s\n",crbk_rec.bank_name);
	fprintf (tOut,".CBRANCH                  :%40.40s\n",crbk_rec.branch_name);
	fprintf (tOut,".CBANK NO                 :%40.40s\n",crbk_rec.bank_no);
	fprintf (tOut,".CBANK ACCOUNT NO         :%40.40s\n",crbk_rec.bank_acct_no);

	fprintf (tOut, "========================================================================================\n");
	fprintf (tOut, ".R========================================================================================\n");

	fprintf (tOut, "|                                   SUPPLIER DETAILS                                   |\n");
	fprintf (tOut, "|     BANK NAME      |     BRANCH NAME    |  BANK NUMBER  |BANK ACCOUNT NO|   AMOUNT   |\n");
	fprintf (tOut, "|--------------------|--------------------|---------------|---------------|------------|\n");

	return (tOut);
}

void
EndPrintOut (
 FILE *	pOut,
 double	total)
{
	if (pOut)
	{
		fprintf (pOut, "|--------------------|--------------------|---------------|---------------|------------|\n");
		fprintf (pOut, "|                    |                    |               |               |%12.2f|\n", DOLLARS (total));
		fprintf (pOut, ".EOF\n");
	}
}

void
PrintLine (
 FILE *	pOut)
{
	fprintf (pOut, "| %6.6s - %9.9s - %40.40s                        |\n",
			 sumr_rec.crd_no,
			 sumr_rec.acronym,
			 sumr_rec.crd_name);

	fprintf (pOut, "|%20.20s|%20.20s|%15.15s|%15.15s|%12.2f|\n",
			sumr_rec.bank,
			sumr_rec.bank_branch,
			sumr_rec.bank_code,
			sumr_rec.bank_acct_no,
			DOLLARS (suhd_rec.tot_amt_paid));
}

/*====================================
 Copy stuff from temp file to printer
=====================================*/
void
FlushToPrinter (
 FILE *	tmpFile)
{
	int	savedprt = FALSE;
	int	resp;

	if (!tmpFile)
		return;

	do	{
		char	prtLine [256];
		FILE *	pOut = popen ("pformat", "w");

		rewind (tmpFile);

		while (fgets (prtLine, sizeof (prtLine), tmpFile))
		{
			if (savedprt && !strcmp (prtLine, ".SO\n"))
				continue;
			fputs (prtLine, pOut);
		}

		pclose (pOut);

		resp = prmptmsg (ML(mlCrMess019), "YyNn", 22, 4);
		rv_pr (ML(mlCrMess162), 22, 4, 0);

		savedprt = TRUE;

	}	while (toupper (resp) == 'Y');

	fclose (tmpFile);
}

/*===========================
 Evaluates data for current sumr (supplier)
===========================*/
void
ProcessSumr (
 void)
{
	AddSuhd ();		/* adds a suhd */

	ProcessSuin ();

	/*--------------------
	 Reread sudt records
	 (yeah, yeah, i know it's increased i/o - but i
	 *want* to do it this way to account for possible db errors!)
	----------------------*/
	cc = find_hash (sudt, &sudt_rec, EQUAL, "r", suhd_rec.hhsp_hash);
	while (!cc && sudt_rec.hhsp_hash == suhd_rec.hhsp_hash)
	{
		suhd_rec.tot_amt_paid	+= sudt_rec.amt_paid_inv;
		suhd_rec.loc_amt_paid	+= sudt_rec.loc_paid_inv;
		suhd_rec.exch_variance	+= sudt_rec.exch_variatio;

		cc = find_hash (sudt, &sudt_rec, NEXT, "r", suhd_rec.hhsp_hash);
	}

	if ((cc = abc_update (suhd, &suhd_rec)))
		file_err (cc, "suhd", "!abc_update()");
}

/*============================================================
 Looks thru' all invoices associated with the current sumr rec
===============================================================*/
void
ProcessSuin (
 void)
{
	cc = find_hash (suin, &suin_rec, EQUAL, "u", sumr_rec.hhsu_hash);
	while (!cc && suin_rec.hhsu_hash == sumr_rec.hhsu_hash)
	{
		if (*suin_rec.stat_flag == 'S')
		{
			double	pay_amt = no_dec (suin_rec.amt - suin_rec.amt_paid);

			/*---------------------------------------
			 Process invoice payment
			---------------------------------------*/
			if (*suin_rec.type == '1' && suin_rec.pay_amt <= pay_amt)
				pay_amt = no_dec (suin_rec.pay_amt);

			AddSudt (pay_amt);	/* adds sudt stuff */

			/*---------------------------------------
			 Update invoice record with payment
			---------------------------------------*/
			suin_rec.amt_paid += no_dec (pay_amt);
			suin_rec.pay_amt = 0;
			strcpy (suin_rec.stat_flag, "0");
			if ((cc = abc_update (suin, &suin_rec)))
				file_err (cc, "suin", "!abc_update()");
		}

		abc_unlock (suin);
		cc = find_hash (suin, &suin_rec, NEXT, "u", sumr_rec.hhsu_hash);
	}
	abc_unlock (suin);
}

/*================
 suhd, sudt stuff
=================*/
void
AddSuhd (
 void)
{
	/*------------------
	 Fill in the blanks
	--------------------*/
	memset (&suhd_rec, 0, sizeof (suhd_rec));	/* flush */
	suhd_rec.pid			= args.pid;
	suhd_rec.hhsu_hash		= sumr_rec.hhsu_hash;
	suhd_rec.date_post 		= TodaysDate ();
	suhd_rec.date_payment	= args.xferdate;
	*suhd_rec.pay_type		= PAYMETHOD_XFER;
	*suhd_rec.rem_prt		= 'R';
	*suhd_rec.stat_flag		= 'C';
	strcpy (suhd_rec.cheq_no, args.refno);
	strcpy (suhd_rec.bank_id, args.bank_id);

	if ((cc = abc_add (suhd, &suhd_rec)))
		file_err (cc, "suhd", "!abc_add()");

	/*--------------------------
	 Reread to obtain hhsp_hash and to set the current record
	----------------------------*/
	if ((cc = find_rec (suhd,  &suhd_rec, EQUAL, "r")))
		file_err (cc, "suhd", "!find_rec(urk)");
}

void
AddSudt (
 double pay_amt)	/* Amount paid off inv/cn this time  */
{
	double	orig_exch_rate;		/* Invoice/cn exchange rate.         */

	memset (&sudt_rec, 0, sizeof (sudt_rec));	/* flush */
	sudt_rec.hhsp_hash = suhd_rec.hhsp_hash;
	sudt_rec.hhsi_hash = suin_rec.hhsi_hash;
	sudt_rec.amt_paid_inv = pay_amt;
	strcpy (sudt_rec.stat_flag, "0");

	if (env.multcurr)
	{
		/*---------------------------------------
		 Convert payment to local currency
		---------------------------------------*/
		sudt_rec.exch_rate = *suin_rec.er_fixed == 'Y' ?
			suin_rec.exch_rate :
			args.rate;

		if (!sudt_rec.exch_rate )
			sudt_rec.exch_rate = 1;

		sudt_rec.loc_paid_inv = no_dec (pay_amt / sudt_rec.exch_rate);

		if (!(orig_exch_rate = suin_rec.exch_rate))
			orig_exch_rate = 1;

		sudt_rec.exch_variatio = no_dec (
			(pay_amt / orig_exch_rate) -
			sudt_rec.loc_paid_inv);
	}
	else
	{
		sudt_rec.exch_rate  = 1;
		sudt_rec.loc_paid_inv = pay_amt;
		sudt_rec.exch_variatio = 0;
	}

	cc = abc_add ("sudt", &sudt_rec);
	if (cc)
		file_err (cc, "sudt", "DBADD");
}

/*================
 Support utilities
==================*/
int
SortByNumber (
 void)
{
	return (*args.order_by == 'N');
}

int
ValidSupp (
 void)
{
	/*=====================================
	 Validate current sumr_rec for payment
	=====================================*/
	return (
		/* supplier selected for payment */
		*sumr_rec.stat_flag == 'S'
		&&
		/* supplier not on stop payment */
		*sumr_rec.hold_payment != 'Y'
		&&
		/* transfer pay methods only */
		*sumr_rec.pay_method == PAYMETHOD_XFER
		&&
		/* validate currency codes */
		(!env.multcurr ||
			!strcmp (sumr_rec.curr_code, crbk_rec.curr_code))
		&&
		/* something worth paying */
		(sumr_rec.bo_curr || sumr_rec.bo_per1 || sumr_rec.bo_per2 || sumr_rec.bo_per3));
}

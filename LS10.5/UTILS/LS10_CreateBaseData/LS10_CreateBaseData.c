/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: LS10_CreateBaseData.c,v 5.2 2001/08/09 09:26:30 scott Exp $
| $Source: /usr/LS10/REPOSITORY/LS10.5/UTILS/LS10_CreateBaseData/LS10_CreateBaseData.c,v $
|  Program Desc  : (Generate base system data required.           )   |
-----------------------------------------------------------------------
| $Log: LS10_CreateBaseData.c,v $
| Revision 5.2  2001/08/09 09:26:30  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:58:26  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:22:17  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:43:21  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 1.4  2001/02/07 02:42:15  scott
| Updated as appschema changes pocr_rec.operator to pocr_rec.pocr_operator
| due to conflict with C++.
|
| Revision 1.3  2001/01/16 00:57:26  scott
| Updated to show messages.
|
| Revision 1.2  2001/01/15 10:49:30  ramon
| Updated to create environment as blank on comr
|
| Revision 1.1  2001/01/15 02:32:00  scott
| New Program to create base data for database
|
|
*/
#define	CCMAIN
char	*PNAME = "$RCSfile: LS10_CreateBaseData.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/UTILS/LS10_CreateBaseData/LS10_CreateBaseData.c,v 5.2 2001/08/09 09:26:30 scott Exp $";

#include <pslscr.h>
#include <CountryInfo.h>
#include <CurrencyInfo.h>
#include <BG_ProcessInfo.h>
#include <OptsInfo.h>
#include <gl_jnlpage.h>
#include <UomInfo.h>

#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct esmrRecord	esmr_rec;
struct ccmrRecord	ccmr_rec;
struct cudpRecord	cudp_rec;
struct pocfRecord	pocf_rec;
struct pocrRecord	pocr_rec;
struct glctRecord	glct2_rec;
struct bproRecord	bpro_rec;
struct optsRecord	opts_rec;
struct gljcRecord	gljc_rec;
struct glmrRecord	glmr2_rec;
struct inumRecord	inum_rec;

	char	*data	= "data";

/*============================================================================
| Local Function Prototypes.
============================================================================*/
void 	shutdown_prog 		(void);	/*	shutdown program function			*/
void 	OpenDB 				(void);	/* 	Open Datbase function				*/
void 	CloseDB 			(void);	/* 	Close Datbase function				*/
int		CreateComr 			(void);	/*	Create company master file info		*/
void	CreateComm 			(void);	/*	Create commom file info				*/
void	CreateEsmr 			(void);	/*	Create branch master file info		*/
void	CreateCcmr 			(void);	/*	Create warehouse master file info	*/
void	CreateCudp 			(void);	/*	Create department master file info	*/
void	CreateGlct 			(void);	/*	Create GL control master file info	*/
void	CreatePocf 			(void);	/*	Create country master file info		*/
void	CreatePocr 			(void);	/*	Create currency master file info	*/
void	CreateBpro 			(void);	/*	Create background process file info	*/
void	CreateOpts 			(void);	/*	Create Ring menu data for displays	*/
void	CreateGljc 			(void);	/*	Create GL page control file			*/
void	CreateGlmr 			(void);	/*	Create Base GL master file info		*/
void	CreateInum 			(void);	/*	Create UOM master file info			*/
int		heading 			(int);	/*	Normal heading function.			*/
									/*--------------------------------------*/

long	datesCreate [37];

char	*setupString	=	"SETUP SETUP SETUP SETUP SETUP SETUP     ";

int		passedFiscal	=	12;
char	passedGlMask [32];
char	passedCurrency [4];

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
	int    argc,
	char*  argv[])
{
	int		i;
	int		currencyFound	=	FALSE;

	/*---------------------------
	| Setup required parameters |
	---------------------------*/
	init_scr ();

	if (argc < 4)
	{
		print_at (0,0, "Usage %s [Fiscal Period 1-12] [Base Currency] [G/L Account Mask]",argv[0]);
		sleep (10);
    	return (EXIT_FAILURE);
	}

	passedFiscal	=	atoi (argv [1]);
	sprintf (passedCurrency, "%-3.3s", argv [2]);
	sprintf (passedGlMask, "%-31.31s", argv [3]);

	for (i = 0;strlen (currencyInfo [i].currencyCode);i++)
	{
		if (!strncmp (passedCurrency,currencyInfo [i].currencyCode,strlen (currencyInfo [i].currencyCode)))
		{
			currencyFound	=	TRUE;
			break;
		}
	}
	if (currencyFound == FALSE)
	{
		print_at (0,0, "Invalid Currency Found, See INCLUDE/CurrencyInfo.h");
		sleep (10);
    	return (EXIT_FAILURE);
	}
	if (passedFiscal < 0 || passedFiscal > 13)
	{
		print_at (0,0, "Invalid Period Passed");
		sleep (10);
    	return (EXIT_FAILURE);
	}
	OpenDB ();

	printf ("Creating base company\n\r");
	if (CreateComr ())
    	return (EXIT_FAILURE);

	printf ("Creating base branch\n\r");
	CreateEsmr ();
	printf ("Creating base warehouse\n\r");
	CreateCcmr ();
	printf ("Creating base department\n\r");
	CreateCudp ();
	printf ("Creating base comm files\n\r");
	CreateComm ();
	printf ("Creating base G/L control\n\r");
	CreateGlct ();
	printf ("Creating base countries\n\r");
	CreatePocf ();
	printf ("Creating base currencies\n\r");
	CreatePocr ();
	printf ("Creating background processes\n\r");
	CreateBpro ();
	printf ("Creating ring options\n\r");
	CreateOpts ();
	printf ("Creating G/L journal controls\n\r");
	CreateGljc ();
	printf ("Creating G/L Company Control account\n\r");
	CreateGlmr ();
	printf ("Creating base unit of measures\n\r");
	CreateInum ();

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*========================
| Program exit sequence. |
========================*/
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (void)
{
	abc_dbopen(data);

	open_rec (comm, comm_list, COMM_NO_FIELDS, "comm_term");
	open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");
	open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	open_rec (cudp, cudp_list, CUDP_NO_FIELDS, "cudp_id_no");
	open_rec (glct, glct_list, GLCT_NO_FIELDS, "glct_mod_date");
	open_rec (pocf, pocf_list, POCF_NO_FIELDS, "pocf_id_no");
	open_rec (pocr, pocr_list, POCR_NO_FIELDS, "pocr_id_no");
	open_rec (bpro, bpro_list, BPRO_NO_FIELDS, "bpro_id_no");
	open_rec (opts, opts_list, OPTS_NO_FIELDS, "opts_id_no");
	open_rec (gljc, gljc_list, GLJC_NO_FIELDS, "gljc_id_no");
	open_rec (glmr, glmr_list, GLMR_NO_FIELDS, "glmr_id_no");
	open_rec (inum, inum_list, INUM_NO_FIELDS, "inum_id_no2");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (comm);
	abc_fclose (comr);
	abc_fclose (esmr);
	abc_fclose (ccmr);
	abc_fclose (cudp);
	abc_fclose (glct);
	abc_fclose (pocf);
	abc_fclose (pocr);
	abc_fclose (bpro);
	abc_fclose (opts);
	abc_fclose (gljc);
	abc_fclose (glmr);
	abc_fclose (inum);
	abc_dbclose(data);
}

int
CreateComr (void)
{
	strcpy (comr_rec.co_no, " ");
	cc = find_rec (comr, &comr_rec, GTEQ, "r");
	if (!cc)
		return (EXIT_FAILURE);

	memset (&comr_rec, 0, sizeof (comr_rec));

	strcpy (comr_rec.co_no, 			" 1");
	sprintf (comr_rec.co_name, 			"%-40.40s", setupString);
	sprintf (comr_rec.co_short_name, 	"%-15.15s", setupString);
	sprintf (comr_rec.co_adr1, 			"%-40.40s", setupString);
	sprintf (comr_rec.co_adr2, 			"%-40.40s", setupString);
	sprintf (comr_rec.co_adr3, 			"%-40.40s", setupString);
	strcpy (comr_rec.master_br, 		" 1");
	strcpy (comr_rec.module_inst,		"NNNNNNNNNNNNNNNNNNNN");
	strcpy (comr_rec.status_flags,		"11111");
	strcpy (comr_rec.prospect_no,		"      ");
	strcpy (comr_rec.prospect_no,		"      ");
	sprintf (comr_rec.gst_ird_no, 		"%-15.15s", setupString);
	sprintf (comr_rec.price1_desc,		"%-15.15s", setupString);
	sprintf (comr_rec.price2_desc,		"%-15.15s", setupString);
	sprintf (comr_rec.price3_desc,		"%-15.15s", setupString);
	sprintf (comr_rec.price4_desc,		"%-15.15s", setupString);
	sprintf (comr_rec.price5_desc,		"%-15.15s", setupString);
	sprintf (comr_rec.price6_desc,		"%-15.15s", setupString);
	sprintf (comr_rec.price7_desc,		"%-15.15s", setupString);
	sprintf (comr_rec.price8_desc,		"%-15.15s", setupString);
	sprintf (comr_rec.price9_desc,		"%-15.15s", setupString);
	sprintf (comr_rec.env_name,			" ");
	sprintf (comr_rec.base_curr,		"%3.3s",  passedCurrency);
	strcpy (comr_rec.stat_flag,			"0");

	comr_rec.fiscal			=	passedFiscal;
	comr_rec.dbt_date		=	TodaysDate();
	comr_rec.crd_date		=	TodaysDate();
	comr_rec.inv_date		=	TodaysDate();
	comr_rec.payroll_date	=	TodaysDate();
	comr_rec.gl_date		=	TodaysDate();
	comr_rec.stmt_date		=	MonthStart (TodaysDate()) - 1L;
	comr_rec.date_stmt_prn	=	MonthStart (TodaysDate()) - 1L;
	comr_rec.yend_date		=	FinYearStart (TodaysDate(), passedFiscal);

	cc = abc_add (comr, &comr_rec);
	if (cc)
		file_err (cc, comr, "DBADD");

	return (EXIT_SUCCESS);
}
void
CreateEsmr (void)
{
	memset (&esmr_rec, 0, sizeof (esmr_rec));

	strcpy (esmr_rec.co_no,  		" 1");
	strcpy (esmr_rec.est_no, 		" 1");
	sprintf (esmr_rec.est_name,		"%-40.40s",	setupString);
	sprintf (esmr_rec.short_name,	"%-15.15s",	setupString);
	sprintf (esmr_rec.adr1,			"%-40.40s",	setupString);
	sprintf (esmr_rec.adr2,			"%-40.40s",	setupString);
	sprintf (esmr_rec.adr3,			"%-40.40s",	setupString);
	strcpy (esmr_rec.area_code,		" 1");
	esmr_rec.dbt_date				=	TodaysDate ();
	esmr_rec.crd_date				=	TodaysDate ();
	esmr_rec.inv_date				=	TodaysDate ();
	esmr_rec.pay_date				=	TodaysDate ();
	esmr_rec.gl_date				=	TodaysDate ();
	esmr_rec.stmt_date				=	MonthStart (TodaysDate()) - 1L;
	esmr_rec.date_stmt_prn			=	MonthStart (TodaysDate()) - 1L;
	strcpy (esmr_rec.status_flags, 	"11111");
	strcpy (esmr_rec.dflt_bank, 	"     ");
	strcpy (esmr_rec.chg_pref, 		"  ");
	strcpy (esmr_rec.csh_pref, 		"  ");
	strcpy (esmr_rec.crd_pref, 		"  ");
	strcpy (esmr_rec.man_pref, 		"  ");
	strcpy (esmr_rec.sales_acc, 	"      ");

	cc = abc_add (esmr, &esmr_rec);
	if (cc)
		file_err (cc, esmr, "DBADD");

	return;
}
void
CreateCcmr (void)
{
	memset (&ccmr_rec, 0, sizeof (ccmr_rec));

	strcpy (ccmr_rec.co_no, 	" 1");
	strcpy (ccmr_rec.est_no, 	" 1");
	strcpy (ccmr_rec.cc_no, 	" 1");
	strcpy (ccmr_rec.master_wh,	"Y");
	strcpy (ccmr_rec.only_loc,	"          ");
	strcpy (ccmr_rec.sman_no,	"  ");
	sprintf (ccmr_rec.name,		"%-40.40s",	setupString);
	sprintf (ccmr_rec.acronym,	"%-40.40s",	setupString);
	sprintf (ccmr_rec.whse_add1,"%-9.9s",	setupString);
	sprintf (ccmr_rec.whse_add2,"%-40.40s",	setupString);
	sprintf (ccmr_rec.whse_add3,"%-40.40s",	setupString);
	sprintf (ccmr_rec.whse_add4,"%-40.40s",	setupString);
	strcpy (ccmr_rec.type,		"MR");
	strcpy (ccmr_rec.sal_ok,	"Y");
	strcpy (ccmr_rec.pur_ok,	"Y");
	strcpy (ccmr_rec.issues_ok,	"Y");
	strcpy (ccmr_rec.receipts,	"Y");
	strcpy (ccmr_rec.reports_ok,"Y");
	strcpy (ccmr_rec.lrp_ok,	"Y");
	ccmr_rec.lpno			=	1;
	strcpy (ccmr_rec.stat_flag, "0");

	cc = abc_add (ccmr, &ccmr_rec);
	if (cc)
		file_err (cc, ccmr, "DBADD");

	return;
}

void
CreateComm (void)
{
	int		i;

	memset (&comm_rec, 0, sizeof (comm_rec));

	for (i = 0; i < 100; i++)
	{
		comm_rec.term	=	i;
		strcpy (comm_rec.co_no, 	" 1");
		sprintf (comm_rec.co_name,	"%-40.40s",		setupString);
		sprintf (comm_rec.co_short,	"%-15.15s",		setupString);
		sprintf (comm_rec.est_no,	" 1");
		sprintf (comm_rec.est_name,	"%-40.40s",		setupString);
		sprintf (comm_rec.est_short,"%-15.15s",		setupString);
		strcpy (comm_rec.cc_no,		" 1");
		sprintf (comm_rec.cc_name,	"%-40.40s",		setupString);
		sprintf (comm_rec.cc_short,	"%-9.9s",		setupString);
		strcpy (comm_rec.dp_no,		" 1");
		sprintf (comm_rec.dp_name,	"%-40.40s",		setupString);
		sprintf (comm_rec.dp_short,	"%-15.15s",		setupString);
		comm_rec.dbt_date		=	TodaysDate ();
		comm_rec.crd_date		=	TodaysDate ();
		comm_rec.inv_date		=	TodaysDate ();
		comm_rec.payroll_date	=	TodaysDate ();
		comm_rec.gl_date		=	TodaysDate ();
		comm_rec.closed_period	=	0;
		comm_rec.fiscal			=	passedFiscal;
		comm_rec.gst_rate		=	0.00;
		sprintf (comm_rec.price1_desc,		"%-15.15s", setupString);
		sprintf (comm_rec.price2_desc,		"%-15.15s", setupString);
		sprintf (comm_rec.price3_desc,		"%-15.15s", setupString);
		sprintf (comm_rec.price4_desc,		"%-15.15s", setupString);
		sprintf (comm_rec.price5_desc,		"%-15.15s", setupString);
		sprintf (comm_rec.price6_desc,		"%-15.15s", setupString);
		sprintf (comm_rec.price7_desc,		"%-15.15s", setupString);
		sprintf (comm_rec.price8_desc,		"%-15.15s", setupString);
		sprintf (comm_rec.price9_desc,		"%-15.15s", setupString);
		sprintf (comm_rec.env_name,		"%s/BIN/LOGISTIC",getenv ("PROG_PATH"));
		strcpy (comm_rec.stat_flag, "0");

		cc = abc_add (comm, &comm_rec);
		if (cc)
			file_err (cc, comm, "DBADD");
	}
}
void
CreateCudp (void)
{
	memset (&cudp_rec, 0, sizeof (cudp_rec));

	strcpy (cudp_rec.co_no,		" 1");
	strcpy (cudp_rec.br_no,		" 1");
	strcpy (cudp_rec.dp_no,		" 1");
	sprintf (cudp_rec.dp_name,	"%-40.40s", 	setupString);
	sprintf (cudp_rec.dp_short,	"%-15.15s", 	setupString);
	sprintf (cudp_rec.location,	"%-40.40s",		setupString);
	strcpy (cudp_rec.csh_pref,  "  ");
	strcpy (cudp_rec.chg_pref,  "  ");
	strcpy (cudp_rec.crd_pref,  "  ");
	strcpy (cudp_rec.man_pref,  "  ");
	strcpy (cudp_rec.stat_flag, "0");

	cc = abc_add (cudp, &cudp_rec);
	if (cc)
		file_err (cc, cudp, "DBADD");
}
void
CreateGlct (void)
{
	memset (&glct2_rec, 0, sizeof (glct2_rec));

	sprintf (glct2_rec.format, "%-31.31s", passedGlMask);
	glct2_rec.fiscal		=	0;
	glct2_rec.history	=	5;
	glct2_rec.link_max	=	10;
	strcpy (glct2_rec.ptype,		"C");
	glct2_rec.max_budg	=	0;
	glct2_rec.nxt_budg	=	0;
	glct2_rec.mod_date	=	TodaysDate ();
	strcpy (glct2_rec.stat_flag, "0");

	cc = abc_add (glct, &glct2_rec);
	if (cc)
		file_err (cc, glct, "DBADD");
}

void
CreatePocf (void)
{
	int		i;

	for (i = 0;strlen (countryInfo [i].countryCode);i++)
	{
		strcpy (pocf_rec.co_no, 	   " 1");
		sprintf (pocf_rec.code, 	   "%-3.3s", countryInfo [i].countryCode);
		sprintf (pocf_rec.description, "%-20.20s", countryInfo [i].countryDesc);
		sprintf (pocf_rec.ldesc, 	   "%-40.40s",countryInfo [i].countryDesc);
		strcpy (pocf_rec.load_type,    "P");
		pocf_rec.freight_load	=	0.00;
		pocf_rec.lead_time		=	0;
		pocf_rec.last_update	=	0;

		cc = abc_add (pocf, &pocf_rec);
		if (cc)
			file_err (cc, pocf, "DBADD");
	}
}
void
CreatePocr (void)
{
	int		i;

	for (i = 0;strlen (currencyInfo [i].currencyCode);i++)
	{
		strcpy (pocr_rec.co_no, 	   " 1");
		sprintf (pocr_rec.code, 	   "%-3.3s", currencyInfo [i].currencyCode);
		sprintf (pocr_rec.description,"%-40.40s",currencyInfo [i].currencyDesc);
		sprintf (pocr_rec.prime_unit, "%-15.15s",currencyInfo [i].currencyUnit);
		sprintf (pocr_rec.sub_unit,	  "%-15.15s"," ");
		pocr_rec.ex1_factor		=	0.00;
		pocr_rec.ex2_factor		=	0.00;
		pocr_rec.ex3_factor		=	0.00;
		pocr_rec.ex4_factor		=	0.00;
		pocr_rec.ex5_factor		=	0.00;
		pocr_rec.ex6_factor		=	0.00;
		pocr_rec.ex7_factor		=	0.00;
		pocr_rec.ldate_up		=	TodaysDate ();
		strcpy (pocr_rec.gl_ctrl_acct, "0000000000000000");
		strcpy (pocr_rec.gl_exch_var,  "0000000000000000");
		strcpy (pocr_rec.stat_flag, "0");
		strcpy (pocr_rec.pocr_operator, " ");

		cc = abc_add (pocr, &pocr_rec);
		if (cc)
			file_err (cc, pocr, "DBADD");
	}
}
void
CreateBpro (void)
{
	int		i;

	for (i = 0;strlen (bproInfo [i].bproProgram);i++)
	{
		strcpy (bpro_rec.co_no, 	"  ");
		strcpy (bpro_rec.br_no, 	"  ");
		strcpy (bpro_rec.up_time,	"00:00");
		strcpy (bpro_rec.status, 	"S");
		sprintf (bpro_rec.program, 	"%-14.14s", bproInfo [i].bproProgram);
		bpro_rec.hash		=	0L;
		bpro_rec.up_date	=	0L;
		bpro_rec.pid		=	0L;
		bpro_rec.lpno		=	atoi (bproInfo [i].bproPrinter);
		sprintf (bpro_rec.parameters, "%-30.30s", bproInfo [i].bproArguments);
		sprintf (bpro_rec.stat_flag, "%-1.1s", bproInfo [i].bproStatus);

		cc = abc_add (bpro, &bpro_rec);
		if (cc)
			file_err (cc, bpro, "DBADD");
	}
}
void
CreateOpts (void)
{
	int		i;

	for (i = 0;strlen (optsInfo [i].optsProgram);i++)
	{
		strcpy (opts_rec.access_code, "*       ");
		sprintf (opts_rec.prog_name, 	"%-14.14s", optsInfo [i].optsProgram);
		opts_rec.option_no	=	atoi (optsInfo [i].optsOptionNo);
		sprintf (opts_rec.key, 	"%-30.30s", optsInfo [i].optsKey);
		strcpy (opts_rec.allowed, 	"Y");
		strcpy (opts_rec.key_desc, 	" ");
		cc = abc_add (opts, &opts_rec);
		if (cc)
			file_err (cc, opts, "DBADD");

		strcpy (opts_rec.access_code, "a       ");
		sprintf (opts_rec.prog_name, 	"%-14.14s", optsInfo [i].optsProgram);
		opts_rec.option_no	=	atoi (optsInfo [i].optsOptionNo);
		sprintf (opts_rec.key, 	"%-30.30s", optsInfo [i].optsKey);
		strcpy (opts_rec.allowed, 	"Y");
		strcpy (opts_rec.key_desc, 	" ");
		cc = abc_add (opts, &opts_rec);
		if (cc)
			file_err (cc, opts, "DBADD");
	}
}
void
CreateGljc (void)
{
	int		i;

	for (i = 0 ; strlen (jrnl [i * 3]); i++)
	{
		strcpy (gljc_rec.co_no, 		" 1");
		sprintf (gljc_rec.journ_type,	"%2d", i + 1);
		sprintf (gljc_rec.jnl_desc,		"%-15.15s",	jrnl [i * 3]);
		sprintf (gljc_rec.rep_prog1, 	"%-40.40s", jrnl [(i * 3) + 1]);
		sprintf (gljc_rec.rep_prog2, 	"%-40.40s", jrnl [(i * 3) + 2]);
		gljc_rec.run_no 	= 0L;
		gljc_rec.tot_1 		= 0.00;
		gljc_rec.tot_2 		= 0.00;
		gljc_rec.tot_3 		= 0.00;
		gljc_rec.tot_4 		= 0.00;
		gljc_rec.tot_5 		= 0.00;
		gljc_rec.tot_6 		= 0.00;
		strcpy (gljc_rec.stat_flag,"N");

		cc = abc_add (gljc, &gljc_rec);
		if (cc)
			file_err (cc, gljc, "DBADD");
	}
}
void
CreateGlmr (void)
{
	strcpy (glmr2_rec.co_no, " 1");
	strcpy (glmr2_rec.acc_no, "0000000000000000");
	sprintf (glmr2_rec.curr_code, "%-3.3s", passedCurrency);
	strcpy (glmr2_rec.desc, "Company Control Account");
	strcpy (glmr2_rec.class1, "C");
	strcpy (glmr2_rec.class2, "C");
	strcpy (glmr2_rec.class3, " ");
	glmr2_rec.hhca_hash		=	0L;
	glmr2_rec.hhmr_hash		=	0L;
	glmr2_rec.parent_cnt		=	0;
	glmr2_rec.child_cnt		=	0;
	glmr2_rec.mod_date		=	TodaysDate ();
	strcpy (glmr2_rec.stat_flag, "0");

	cc = abc_add (glmr, &glmr2_rec);
	if (cc)
		file_err (cc, glmr, "DBADD");
}
void
CreateInum (void)
{
	int		i;

	memset (&inum_rec, 0, sizeof (inum_rec));

	for (i = 0;strlen (uomInfo [i].UomGroup);i++)
	{
		sprintf (inum_rec.uom_group, "%-20.20s", 	uomInfo [i].UomGroup);
		sprintf (inum_rec.uom, 		"%-4.4s", 		uomInfo [i].Uom);
		sprintf (inum_rec.desc,		"%-40.40s", 	uomInfo [i].UomDesc);
		inum_rec.cnv_fct			=				uomInfo [i].UomCnv;
		inum_rec.hhum_hash			=				0L;

		cc = abc_add (inum, &inum_rec);
		if (cc)
			file_err (cc, inum, "DBADD");
	}
}

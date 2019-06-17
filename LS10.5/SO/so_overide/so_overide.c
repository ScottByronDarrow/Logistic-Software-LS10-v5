/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: so_overide.c,v 5.3 2002/11/28 04:09:51 scott Exp $
|  Program Name  : (so_overide.c)    
|  Program Desc  : (Invoicing Overide Report)
|---------------------------------------------------------------------|
|  Author        : Fui Choo YAP.   | Date Written  : 12/08/86         |
|---------------------------------------------------------------------|
| $Log: so_overide.c,v $
| Revision 5.3  2002/11/28 04:09:51  scott
| SC0053 - Platinum Logistics - LS10.5.2 2002-11-28
| Updated for changes in pricing - See S/C for Details
|
| Revision 5.2  2001/08/09 09:21:38  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:51:34  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:20:11  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:41:27  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2001/03/06 09:26:09  scott
| Updated to allow -ve discounts to be processed.
| Problem with this was test on inmr_disc_pc should have been applied
| only when inmr_disc_pc had a value.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: so_overide.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_overide/so_overide.c,v 5.3 2002/11/28 04:09:51 scott Exp $";

#define		FGN_CURR    (envVarDbMcurr && strcmp (cumr_rec.curr_code,currencyCode))
#include 	<pslscr.h>
#include 	<twodec.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<ml_std_mess.h>
#include 	<ml_so_mess.h>

	/*===============================================================
	| Special fields and flags  ##################################. |
	===============================================================*/
	char	workBranch [3],
			currencyCode [4],
			findStatus [2];
 
	int		firstTime 			= TRUE,
			newInvoice    		= TRUE,
			envVarSoDisIndent 	= TRUE,
			envVarDbMcurr		= 0,
			printerNumber 		= 1,
			printed 			= 0,
			useSystemDate		= 0;

	float	std_disc_pc = 0.00;

	FILE	*fout,
			*fin;

	
#include	"schema"

struct commRecord	comm_rec;
struct ccmrRecord	ccmr_rec;
struct cnchRecord	cnch_rec;
struct cohrRecord	cohr_rec;
struct colnRecord	coln_rec;
struct pocrRecord	pocr_rec;
struct cumrRecord	cumr_rec;
struct inmrRecord	inmr_rec;

	char	*data = "data";
	Money	*cumr_balance	=	&cumr_rec.bo_current;

	int	con_price = FALSE;

#include <cus_price.h>
#include <cus_disc.h>
#include <pr_format3.h>

/*=======================
| Function Declarations |
=======================*/
void 	shutdown_prog 			(void);
void 	OpenDB 					(void);
void 	CloseDB 				(void);
void 	ReadMisc 				(void);
void 	ProcessFile 			(void);
static 	int CheckCustomerCrd 	(long);
void 	ProcessColn 			(long);
float 	DiscProcess 			(double, float);
void 	OpenAudit 				(void);
void 	PrintLine 				(char *, char *, float, float);
void 	EndReport 				(void);
int  	check_page 				(void);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char	*argv [])
{
	char	*sptr = chk_env ("DB_MCURR");

	if (sptr)
		envVarDbMcurr = atoi (sptr);

	sprintf (currencyCode, "%-3.3s", get_env ("CURR_CODE"));

	if (argc != 3 && argc != 4)
	{
		print_at (0,0,mlSoMess763,argv [0]);
		return (EXIT_FAILURE);
	}

	/*--------------------------------------
	| Check for discounts on Indent items. |
	--------------------------------------*/
	sptr = chk_env ("SO_DIS_INDENT");
	envVarSoDisIndent = (sptr == (char *)0) ? TRUE : atoi ( sptr);

	/*--------------------------------
    | Check and Get Order Date Type. |
    ---------------------------------*/
	sptr = chk_env ("SO_DOI");
	useSystemDate = (sptr == (char *)0 || sptr [0] == 'S') ? TRUE : FALSE;
	printerNumber = atoi (argv [1]);

	sprintf (findStatus,"%-1.1s",argv [2]);

	OpenDB 		();
	OpenPrice 	();
	OpenDisc 	();

	if (argc == 4)
		sprintf (workBranch, "%2.2s", argv [3]);
	else
		sprintf (workBranch, "%2.2s", comm_rec.est_no);

	set_tty ();

	dsp_screen (" Printing Invoicing Overide Report.",
					comm_rec.co_no, comm_rec.co_name);

	ProcessFile ();
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*========================
| Program exit sequence. |
========================*/
void
shutdown_prog (
 void)
{
	if (firstTime == FALSE)
		EndReport ();

	CloseDB (); 
	FinishProgram ();
}

/*======================
| Open Database Files. |
======================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);
	ReadMisc ();

	open_rec (cnch, cnch_list, CNCH_NO_FIELDS, "cnch_id_no");
	open_rec (cohr, cohr_list, COHR_NO_FIELDS, "cohr_up_id");
	open_rec (coln, coln_list, COLN_NO_FIELDS, "coln_id_no");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (pocr, pocr_list, POCR_NO_FIELDS, "pocr_id_no");
}

/*=======================
| Close Database files. |
=======================*/
void
CloseDB (
 void)
{
	abc_fclose (cnch);
	abc_fclose (cohr);
	abc_fclose (coln);
	abc_fclose (cumr);
	abc_fclose (inmr);
	abc_fclose (pocr);

	ClosePrice ();
	CloseDisc ();

	abc_dbclose (data);
}

/*============================================
| Get common info from commom database file. |
============================================*/
void
ReadMisc (
 void)
{
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	strcpy (ccmr_rec.co_no,  comm_rec.co_no);
	strcpy (ccmr_rec.est_no, comm_rec.est_no);
	strcpy (ccmr_rec.cc_no,  comm_rec.cc_no);
	cc = find_rec (ccmr,&ccmr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");

	abc_fclose (ccmr);

}

/*======================================================================
| Process whole cohr file looking for stat = findStatus                |
| and updating inmr record appropriately.                              |
| Returns: 0 if ok, non-zero if not ok.                                |
======================================================================*/
void
ProcessFile (
 void)
{
	strcpy (cohr_rec.co_no,     comm_rec.co_no);
	strcpy (cohr_rec.br_no,     workBranch);
	strcpy (cohr_rec.type,      "I");
	strcpy (cohr_rec.stat_flag, findStatus);

	cc = find_rec (cohr, &cohr_rec, GTEQ, "r");
	/*-----------------------
	| Read whole cohr file. |
	-----------------------*/
	while (!cc && 
		   !strcmp (cohr_rec.co_no, comm_rec.co_no) && 
		   !strcmp (cohr_rec.br_no, workBranch) && 
		   cohr_rec.type [0] == 'I' && 
		   cohr_rec.stat_flag [0] == findStatus [0])
	{
		newInvoice = TRUE;
		if (!firstTime && printed)
			pr_format (fin, fout, "LINE1", 0,0);

		dsp_process (" Invoice : ",cohr_rec.inv_no);
		
		ProcessColn (cohr_rec.hhco_hash);

		cc = find_rec (cohr, &cohr_rec, NEXT, "r");
	}
}


static	int
CheckCustomerCrd (
 long	hhcu_hash)
{
	double	curr_bal = 0.00;

	cumr_rec.hhcu_hash = hhcu_hash;
	cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
	if (cc)
		return (cc);

	if (envVarDbMcurr)
	{
		strcpy (pocr_rec.co_no, comm_rec.co_no);
		strcpy (pocr_rec.code , cumr_rec.curr_code);
		cc = find_rec (pocr, &pocr_rec, COMPARISON, "r");
		if (cc)
			return (cc);
	}
	
	curr_bal = cumr_balance [0]  +
			   cumr_balance [1]  +
			   cumr_balance [2]  +
			   cumr_balance [3]  +
			   cumr_balance [4]  +
			   cumr_balance [5]  +
			   cohr_rec.gross  	 +
			   cohr_rec.freight  +
			   cohr_rec.gst      +
			   cohr_rec.tax      - 
			   cohr_rec.disc; 

	if (curr_bal >= cumr_rec.credit_limit && cumr_rec.credit_limit != 0.00)
	{
		PrintLine ("C", " ", DOLLARS (cumr_rec.credit_limit), DOLLARS (curr_bal));
		printed = 1;
	}

	return (EXIT_SUCCESS);
}

void
ProcessColn (
 long	shash)
{
	int		pType;
	float	regPc;
	double	grossPrice = 0.00;
	double	oldPrice = 0.00;

	printed = 0;
	cc = CheckCustomerCrd (cohr_rec.hhcu_hash);
	if (cc)
		return;

	coln_rec.hhco_hash = shash;
	coln_rec.line_no = 0;

	cc = find_rec (coln, &coln_rec, GTEQ, "r");
	while (!cc && coln_rec.hhco_hash == shash)
	{
		inmr_rec.hhbr_hash = coln_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (coln, &coln_rec, NEXT, "r");
			continue;
		}

		/*-------------------------
		| Lookup contract header. |
		-------------------------*/
		strcpy (cnch_rec.co_no, comm_rec.co_no);
		strcpy (cnch_rec.cont_no, cohr_rec.cont_no);
		cc = find_rec (cnch, &cnch_rec, COMPARISON, "r");
		if (cc)
		{
			cnch_rec.hhch_hash = 0L;
			strcpy (cnch_rec.exch_type, " ");
		}

		pType = atoi (cumr_rec.price_type);
		grossPrice = GetCusPrice (comm_rec.co_no,
								 comm_rec.est_no,
								 comm_rec.cc_no,
								 cohr_rec.area_code,
								 cumr_rec.class_type,
								 inmr_rec.sellgrp,
								 cumr_rec.curr_code,
								 pType,
						 		 cumr_rec.disc_code,
								 cnch_rec.exch_type,
								 cumr_rec.hhcu_hash,
								 coln_rec.incc_hash,
								 coln_rec.hhbr_hash,
						 		 inmr_rec.category,
								 cnch_rec.hhch_hash,
							  	 (useSystemDate) ? TodaysDate () : comm_rec.dbt_date,
							 	 (coln_rec.q_order + coln_rec.q_backorder),
								 cohr_rec.exch_rate,
								 FGN_CURR,
								 &regPc);

		con_price = (_CON_PRICE) ? TRUE : FALSE;
		oldPrice = GetCusGprice (grossPrice, regPc);

		if (no_dec (coln_rec.sale_price) != no_dec (oldPrice))
		{
			PrintLine ("S", inmr_rec.item_no, 
					 DOLLARS (oldPrice),
				     DOLLARS (coln_rec.sale_price));
			printed = 1;
		}

		/*-------------------------------
		| Check for change in disc_pc	|
		-------------------------------*/
		std_disc_pc = DiscProcess (oldPrice, regPc);

		if (coln_rec.disc_pc != std_disc_pc)
		{
			PrintLine ("D", inmr_rec.item_no, std_disc_pc, coln_rec.disc_pc);
			printed = 1;
		}
		cc = find_rec (coln, &coln_rec, NEXT, "r");
	}
}

/*-----------------------------
| Calculate current discount. |
-----------------------------*/
float 
DiscProcess (
 double	grossPrice,
 float	regPc)
{
	int		pType;
	int		cumDisc;
	float	discArray [3];
	float	oneDisc;

	/*--------------------------
	| Discount does not apply. |
	--------------------------*/
	if (_cont_status == 2 || _CON_PRICE)
		return ((float)0.00);

	if (!strncmp (inmr_rec.item_no, "INDENT", 6) && !envVarSoDisIndent)
		return ((float)0.00);

	/*---------------
	| Get discount. |
	---------------*/
	pType   = 	atoi (cumr_rec.price_type);
	cumDisc	=	GetCusDisc 
				(
					comm_rec.co_no,
					comm_rec.est_no,
					coln_rec.incc_hash,
					cumr_rec.hhcu_hash,
					cumr_rec.class_type,
					cumr_rec.disc_code,
					alt_hash (inmr_rec.hhbr_hash, inmr_rec.hhsi_hash),
					inmr_rec.category,
					inmr_rec.sellgrp,
					pType,
					grossPrice,
					regPc,
					(coln_rec.q_order + coln_rec.q_backorder),
					discArray
				);
							
	/*----------------------------------
	| Calculate one combined discount. |
	----------------------------------*/
	oneDisc	= CalcOneDisc (cumDisc, 
						  discArray [0],
						  discArray [1],
						  discArray [2]);

	/*-----------------------------------
	| Item discount overides calculated |
	| discount if greater.              |
	-----------------------------------*/
	if (inmr_rec.disc_pc > oneDisc && inmr_rec.disc_pc != 0.0)
		return (inmr_rec.disc_pc);

	return (oneDisc);
}

/*=========================================================================
| Routine to open output pipe to standard print to provide an audit trail |
| of events. This also sends the output straight to the spooler.          |
=========================================================================*/
void
OpenAudit (
 void)
{
	if ((fout = popen ("pformat","w")) == 0) 
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);

	if ((fin = pr_open ("so_overide.p")) == 0) 
		sys_err ("Error in so_overide.p During (FOPEN)", errno, PNAME);

	fprintf (fout, ".START%s<%s>\n", DateToString (comm_rec.dbt_date), PNAME);
	fprintf (fout, ".LP%d\n", printerNumber);
	fprintf (fout, ".SO\n");
	fprintf (fout, ".12\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L158\n");
	fprintf (fout, ".EINVOICING OVERIDE REPORT\n");
	fprintf (fout, ".B1\n");

	fprintf (fout, ".E%s\n",clip (comm_rec.co_name));
	fprintf (fout, ".EAs At %s\n",SystemTime ());
	fprintf (fout, ".B1\n");
	fprintf (fout, ".EBranch %s\n", clip (comm_rec.est_name));

	pr_format (fin, fout, "RULEOFF", 0,0);
	pr_format (fin, fout, "RULER", 0,0);
	pr_format (fin, fout, "HEAD1", 0,0);
	pr_format (fin, fout, "LINE1", 0,0);
}

void
PrintLine (
 char	*warn_type,
 char	*part_no,
 float	val_1,
 float	val_2)
{
	char	warn_msg [31];
	char	narr_str [81];

	if (firstTime == TRUE)
		OpenAudit ();

	firstTime = FALSE;

	if (newInvoice)
	{
		pr_format (fin, fout, "DETAIL", 1,cohr_rec.inv_no);
		pr_format (fin, fout, "DETAIL", 2,cumr_rec.dbt_no);
		pr_format (fin, fout, "DETAIL", 3,cumr_rec.dbt_name);
		if (envVarDbMcurr)
		{
			pr_format (fin, fout,"DETAIL", 4," ");
			pr_format (fin, fout,"DETAIL",5,pocr_rec.description);
			pr_format (fin, fout,"DETAIL", 1," ");
			pr_format (fin, fout,"DETAIL", 2," ");
			pr_format (fin, fout,"DETAIL", 3," ");
		}

		newInvoice = FALSE;
	}
	else
	{
		pr_format (fin, fout, "DETAIL", 1," ");
		pr_format (fin, fout, "DETAIL", 2," ");
		pr_format (fin, fout, "DETAIL", 3," ");
	}

	switch (warn_type [0])
	{
	case	'C':
		sprintf (warn_msg, "%-30.30s", "Customer Exceeds Credit Limit.");
		sprintf (narr_str,
				"Credit Limit %11.2f / Current Balance %13.2f   ",
				val_1,
				val_2);
		break;

	case	'S':
		sprintf (warn_msg, "%-30.30s", "Price Overide On Invoicing.   ");
		sprintf (narr_str,
				"Item %16.16s     Old   %8.2f Vs New   %8.2f",
				part_no,
				val_1,
				val_2);
		break;

	case	'D':
		sprintf (warn_msg, "%-30.30s", "Discount Overide On Invoicing.");
		sprintf (narr_str,
				"Item %16.16s     Old    %%%6.2f Vs New     %6.2f",
				part_no,
				val_1,
				val_2);
		break;
	}
	pr_format (fin, fout, "DETAIL", 4,warn_msg);
	pr_format (fin, fout, "DETAIL", 5,narr_str);
}

void
EndReport (
 void)
{
	fprintf (fout, ".EOF\n");
	pclose (fout);
	fclose (fin);
}

int
check_page (
 void)
{
	return (EXIT_SUCCESS);
}

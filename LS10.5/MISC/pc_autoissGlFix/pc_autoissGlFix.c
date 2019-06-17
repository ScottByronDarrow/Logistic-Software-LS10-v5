/*=====================================================================
|  Copyright (C) 1999 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: pc_autoissGlFix.c,v 5.0 2002/05/08 01:22:27 scott Exp $
|  Program Name  : (pc_autoissGlFix.h) 
|  Program Desc  : (Automatic Production Control Issues GlFix)
|---------------------------------------------------------------------|
|  Date Written  : 29th March 2001 | Author       : Scott B Darrow    |
|---------------------------------------------------------------------|
| $Log: pc_autoissGlFix.c,v $
| Revision 5.0  2002/05/08 01:22:27  scott
| CVS administration
|
| Revision 1.10  2001/08/09 09:49:48  scott
| Updated to add FinishProgram () function
|
| Revision 1.9  2001/08/06 23:31:24  scott
| RELEASE 5.0
|
| Revision 1.8  2001/07/01 01:32:41  scott
| Updated to ensure category is used for all interfaces
|
| Revision 1.7  2001/06/23 01:11:50  scott
| Updated to produce an error log
| Update to post directly to gl work file
|
| Revision 1.6  2001/06/21 12:29:45  cha
| dito
|
| Revision 1.5  2001/06/21 12:28:59  cha
| dito
|
| Revision 1.4  2001/06/21 12:10:14  scott
| Updated for cost
|
| Revision 1.3  2001/06/21 10:17:48  scott
| Updated to use standard rather than actual.
|
| Revision 1.2  2001/06/21 07:10:59  cha
| Updated line number to start at 1
|
| Revision 1.1  2001/06/21 05:23:02  scott
| First release - program re-creates gl transactions for automatic production issue
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: pc_autoissGlFix.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MISC/pc_autoissGlFix/pc_autoissGlFix.c,v 5.0 2002/05/08 01:22:27 scott Exp $";

#define		ERR_WOI_POSTED		1
#define		ERR_WOI_NO_ORDER	2
#define		ERR_WOI_NO_SKU		3
#define		ERR_WOI_NO_CCMR		4
#define		ERR_WOI_NO_INEI		5
#define		ERR_WOI_OK			6

#include	<pslscr.h>
#include	<GlUtils.h>
#include	<ml_std_mess.h>
#include	<ml_pc_mess.h>
#include	<GlUtils.h>
#include	<twodec.h>
#include	<number.h>
#include	<XML_Error.h>

#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct ccmrRecord	ccmr_rec;
struct inmrRecord	inmr_rec;
struct pcglRecord	pcgl_rec;
struct pcmsRecord	pcms_rec;
struct pcwoRecord	pcwo_rec;
struct esmrRecord	esmr_rec;
struct ineiRecord	inei_rec;
struct inumRecord	inum_rec;

	char	*data	= "data";

	float	stdCnvFct	=	0.00,
			altCnvFct	=	0.00,
			cnvFct		=	0.00;

	long	stdHhumHash	=	0L,
			altHhumHash	=	0L;

	char 	debitAccount 	 [MAXLEVEL + 1],	/* WIP D MATL	*/
			creditAccount 	 [MAXLEVEL + 1],	/* COSTSALE M	*/
			mfgAccount 		 [MAXLEVEL + 1],	/* MAN D MATL	*/
			writeOffAcc 	 [MAXLEVEL + 1],	/* Stock write-off account.		 */
			localCurrency 	 [4],
			altUomGroup 	 [21],
			stdUomGroup 	 [21],
			stdUom  		 [5],
			altUom  		 [5];

	long 	debitHash	=	0L,
			creditHash	=	0L,
			mfgHash		=	0L,
			writeOff	=   0L,
			currentHhcc = 	0L;

/*=====================
| function prototypes |
=====================*/
void 	GetAccounts 		 (char *);
int 	Update 				 (void);
void	ReadDefault 		 (char *);
void 	AddPcgl 			 (void);
void 	AddPcglScrapped		 (void);
void 	CloseDB 			 (void);
int		PRError 			 (int);
int		StartProgram 		 (void);
void 	OpenDB 				 (void);
void 	PrintDetails 		 (float, double, char *, char *, char *,float);
void 	shutdown_prog 		 (void);
void	ProcessInum 		 (void);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char	*argv [])
{
	if (argc < 2)
		return (EXIT_FAILURE);

	pcwo_rec.hhwo_hash	=	atol (argv [1]);

	cc = StartProgram ();
	if (cc)
		return (cc);

	/*-------------------
	| Find Works order. |
	-------------------*/
	cc = find_rec (pcwo, &pcwo_rec, EQUAL, "u");
	if (cc)
	{
		abc_unlock (pcwo);
		return (PRError (ERR_WOI_NO_ORDER));
	}

	if (pcwo_rec.stat_flag [0] == 'P')
	{
		abc_unlock (pcwo);
		return (PRError (ERR_WOI_POSTED));
	}

	/*----------------------------------------
	| Find warehouse record for works order. |
	----------------------------------------*/
	strcpy (ccmr_rec.co_no,  pcwo_rec.co_no);
	strcpy (ccmr_rec.est_no, pcwo_rec.br_no);
	strcpy (ccmr_rec.cc_no,  pcwo_rec.wh_no);
	cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
	if (cc)
		return (PRError (ERR_WOI_NO_CCMR));
	
	/*-------------------
	| Find pcms records |
	-------------------*/
	pcms_rec.hhwo_hash 	= pcwo_rec.hhwo_hash;
	pcms_rec.uniq_id 	= 0;
	cc = find_rec (pcms, &pcms_rec, GTEQ, "r");
	while (!cc && pcms_rec.hhwo_hash == pcwo_rec.hhwo_hash)
	{
		/*--------------------------------
		| Find SKU line for works order. |
		--------------------------------*/
		inmr_rec.hhbr_hash	=	pcms_rec.mabr_hash;
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		if (cc)
			return (PRError (ERR_WOI_NO_SKU));

		/*-----------------------------------
		| Process conversion stuff for UOM. |
		-----------------------------------*/
		ProcessInum ();
		/*----------------------------
		| Get accounts for category. |
		----------------------------*/
		ReadDefault (inmr_rec.category);
		GetAccounts (inmr_rec.category);
		
		/*---------------------------------
		| Add General Ledger Transaction. |
		---------------------------------*/
		AddPcgl ();

		AddPcglScrapped ();

		cc = find_rec (pcms, &pcms_rec, NEXT, "r");
	}
	strcpy (pcwo_rec.stat_flag, "P");
	cc = abc_update (pcwo, &pcwo_rec);
	if (cc)
		file_err (cc, pcwo, "DBUPDATE");

	shutdown_prog ();
	return (EXIT_SUCCESS);
}


/*===========================================================================
| Start up program, Open database files, read default accounts, open audit. |
===========================================================================*/
int
StartProgram (void)
{
	/*----------------------
	| Open database files. |
	----------------------*/
	OpenDB ();


	return (EXIT_SUCCESS);
}
/*=======================
| Program exit sequence	|
=======================*/
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*========================
| Read Default accounts. |
========================*/
void
ReadDefault (
	char	*category)
{
	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"WIP D MATL",
		" ",
		category
	);
	debitHash = glmrRec.hhmr_hash;
	strcpy (debitAccount, glmrRec.acc_no);

	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"MAN D MATL",
		" ",
		category
	);
	mfgHash = glmrRec.hhmr_hash;
	strcpy (mfgAccount, glmrRec.acc_no);

	return;
}

/*=======================
| Open data base files	|
=======================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (comr,  comr_list, COMR_NO_FIELDS, "comr_co_no");
	strcpy (comr_rec.co_no, comm_rec.co_no);
	cc = find_rec (comr, &comr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, comr, "DBFIND");

	if (!strcmp (comr_rec.base_curr, "   "))
		sprintf (localCurrency, "%-3.3s", get_env ("CURR_CODE"));
	else
		sprintf (localCurrency, "%-3.3s", comr_rec.base_curr);

	abc_fclose (comr);

	open_rec (ccmr,  ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	open_rec (inum,  inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	open_rec (inei,  inei_list, INEI_NO_FIELDS, "inei_id_no");
	open_rec (inmr,  inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (pcgl,  pcgl_list, PCGL_NO_FIELDS, "pcgl_id_no");
	open_rec (pcms,  pcms_list, PCMS_NO_FIELDS, "pcms_id_no");
	open_rec (pcwo,  pcwo_list, PCWO_NO_FIELDS, "pcwo_hhwo_hash");
	open_rec (esmr,  esmr_list, ESMR_NO_FIELDS, "esmr_id_no");

	strcpy (ccmr_rec.co_no,  comm_rec.co_no);
	strcpy (ccmr_rec.est_no, comm_rec.est_no);
	strcpy (ccmr_rec.cc_no,  comm_rec.cc_no);
	cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");

	OpenGlmr ();
	currentHhcc = ccmr_rec.hhcc_hash;
	GL_OpenBatch (comm_rec.co_no, comm_rec.est_no);
}

/*=======================
| Close data base files	|
=======================*/
void
CloseDB (
 void)
{
	abc_fclose (ccmr);
	abc_fclose (inei);
	abc_fclose (inmr);
	abc_fclose (pcgl);
	abc_fclose (pcms);
	abc_fclose (pcwo);
	abc_fclose (esmr);
	abc_fclose (inum);
	abc_fclose (glmr);
	GL_CloseBatch (0);

	GL_Close ();

	abc_dbclose (data);
}

/*===========================
| Process control Accounts. |
===========================*/
void
GetAccounts (
	char	*category)
{
	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"COSTSALE M",
		" ",
		category
	);
	strcpy (creditAccount, glmrRec.acc_no);
	creditHash = glmrRec.hhmr_hash;
}

/*================================
| Add transactions to pcgl file. |
================================*/
void
AddPcgl (void)
{
	int		periodMonth	=	0;
	double	workCost	=	0.00;
	
	strcpy (inei_rec.est_no, pcwo_rec.br_no);
	inei_rec.hhbr_hash	=	pcms_rec.mabr_hash;
	cc = find_rec (inei, &inei_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, inei, "DBFIND");

	workCost	=	CENTS ((pcms_rec.qty_issued * inei_rec.std_cost) / (double) fourdec (cnvFct));
	workCost	= 	out_cost (workCost, inmr_rec.outer_size);
	workCost	=	no_dec (workCost);

	if (workCost == 0.00)
		return;

	strcpy 	 (pcgl_rec.co_no, ccmr_rec.co_no);
	strcpy 	 (pcgl_rec.tran_type, "19");
	sprintf (pcgl_rec.sys_ref, "%5.1d", 1);
	pcgl_rec.tran_date 	= 	TodaysDate ();
	pcgl_rec.post_date 	= 	TodaysDate ();
	DateToDMY (pcgl_rec.tran_date, NULL, 	&periodMonth, NULL);
	sprintf (pcgl_rec.period_no, "%02d", 	periodMonth);
	sprintf (pcgl_rec.narrative, "%-7.7s/%03d", pcwo_rec.order_no,pcms_rec.uniq_id + 1);
	sprintf (pcgl_rec.user_ref, "%8.8s", 	 "AUTO ISS");
	strcpy 	 (pcgl_rec.stat_flag, "2");
	/*
	 * Post the std cost to the WIP Direct Material Account.
	 */
	strcpy (pcgl_rec.acc_no, debitAccount);
	pcgl_rec.hhgl_hash 	= debitHash;
	pcgl_rec.amount		= workCost;
	pcgl_rec.loc_amount	= workCost;
	strcpy (pcgl_rec.jnl_type, "1");
	pcgl_rec.exch_rate	=	1.00;
	strcpy (pcgl_rec.currency, localCurrency);

	if (pcgl_rec.amount != 0.00)
	{
		pin_bcopy ((char *) &glwkRec, (char *) &pcgl_rec, sizeof (glwkRec));
		GL_AddBatch ();
		/*
		cc = abc_add (pcgl, &pcgl_rec);
		if (cc)
			file_err (cc, "pcgl", "DBADD");
		*/
	}

	/*
	 * Post the act cost to the components stock account.   
	 */
	strcpy (pcgl_rec.acc_no, creditAccount);
	pcgl_rec.hhgl_hash 	=	creditHash;
	pcgl_rec.amount		= workCost;
	pcgl_rec.loc_amount	= workCost;
	strcpy (pcgl_rec.jnl_type, "2");
	pcgl_rec.exch_rate	=	1.00;
	strcpy (pcgl_rec.currency, localCurrency);

	if (pcgl_rec.amount != 0.00)
	{
		pin_bcopy ((char *) &glwkRec, (char *) &pcgl_rec, sizeof (glwkRec));
		GL_AddBatch ();
		/*
		cc = abc_add (pcgl, &pcgl_rec);
		if (cc)
			file_err (cc, "pcgl", "DBADD");
		*/
	}
}
/*================================
| Add transactions to pcgl file. |
================================*/
void
AddPcglScrapped (void)
{
	double	workCost	=	0.00;
	int		periodMonth;

	if (pcms_rec.amt_scrap == 0.00)
		return;

	workCost	= 	out_cost (pcms_rec.amt_scrap, inmr_rec.outer_size);
	workCost	=	no_dec (workCost);

	strcpy 	 (pcgl_rec.co_no, ccmr_rec.co_no);
	strcpy 	 (pcgl_rec.tran_type, "19");
	sprintf (pcgl_rec.sys_ref, "%5.1d", 1);
	pcgl_rec.tran_date 	= 	TodaysDate ();
	pcgl_rec.post_date 	= 	TodaysDate ();
	DateToDMY (pcgl_rec.tran_date, NULL, 	&periodMonth, NULL);
	sprintf (pcgl_rec.period_no, "%02d", 	periodMonth);
	sprintf (pcgl_rec.narrative, "%-7.7s/%03d", pcwo_rec.order_no,pcms_rec.uniq_id + 1);
	sprintf (pcgl_rec.user_ref, "%8.8s", 	 "AUTO ISS");
	strcpy 	 (pcgl_rec.stat_flag, "2");

	/*
	 * Post the act cost to the MAN D MATL
	 */
	strcpy (pcgl_rec.acc_no, mfgAccount);
	pcgl_rec.hhgl_hash 	= 	mfgHash;
	pcgl_rec.amount		=	workCost;
	pcgl_rec.loc_amount	=	workCost;
	strcpy (pcgl_rec.jnl_type, "1");
	pcgl_rec.exch_rate	=	1.00;
	strcpy (pcgl_rec.currency, localCurrency);

	if (pcgl_rec.amount != 0.00)
	{
		pin_bcopy ((char *) &glwkRec, (char *) &pcgl_rec, sizeof (glwkRec));
		GL_AddBatch ();
		/*
		cc = abc_add (pcgl, &pcgl_rec);
		if (cc)
			file_err (cc, "pcgl", "DBADD");
		*/
	}
	strcpy (creditAccount, glmrRec.acc_no);
	creditHash = glmrRec.hhmr_hash;

	/*
	 * Post the act cost to the components stock account.   
	 */
	strcpy (pcgl_rec.acc_no, creditAccount);
	pcgl_rec.hhgl_hash 	= 	creditHash;
	pcgl_rec.amount		=	workCost;
	pcgl_rec.loc_amount	=	workCost;
	strcpy (pcgl_rec.jnl_type, "2");
	pcgl_rec.exch_rate	=	1.00;
	strcpy (pcgl_rec.currency, localCurrency);

	if (pcgl_rec.amount != 0.00)
	{
		pin_bcopy ((char *) &glwkRec, (char *) &pcgl_rec, sizeof (glwkRec));
		GL_AddBatch ();
		/*
		cc = abc_add (pcgl, &pcgl_rec);
		if (cc)
			file_err (cc, "pcgl", "DBADD");
		*/
	}
}

/*================
| Process Errors |
================*/
int
PRError (
	int	errCode)
{
	FILE 	*errFileOutput;
	char	errMess [3][81];

	if ((errFileOutput = fopen ("pc_AutoIssGlFix.LOG","a")) == NULL)
		return (EXIT_FAILURE);

	if (ERR_WOI_POSTED == errCode)
	{
		sprintf (errMess [0], "Works order hash (pcwo_hhwo_hash) has already been posted");
		sprintf (errMess [1], "pcwo_stat_flag = [%s]", pcwo_rec.stat_flag);
		fprintf (errFileOutput, "%s\n", errMess [0]);
		fprintf (errFileOutput, "%s\n", errMess [1]);
		return (errCode);
	}
	if (ERR_WOI_NO_ORDER == errCode)
	{
		sprintf (errMess [0], "Works order hash (pcwo_hhwo_hash) not valid");
		sprintf (errMess [1], "pcwo_hhwo_hash = [%ld]", pcwo_rec.hhwo_hash);
		fprintf (errFileOutput, "%s\n", errMess [0]);
		fprintf (errFileOutput, "%s\n", errMess [1]);
		return (errCode);
	}
	if (ERR_WOI_NO_SKU == errCode)
	{
		sprintf (errMess [0], "SKU could not be found for works order");
		sprintf (errMess [1],  "inmr_hhbr_hash = [%ld]", inmr_rec.hhbr_hash);
		sprintf (errMess [2],  "pcms_hhwo_hash = [%ld]", pcwo_rec.hhwo_hash);
		fprintf (errFileOutput, "%s\n", errMess [0]);
		fprintf (errFileOutput, "%s\n", errMess [1]);
		fprintf (errFileOutput, "%s\n", errMess [2]);
		return (errCode);
	}
	if (ERR_WOI_NO_CCMR == errCode)
	{
		sprintf (errMess [0], "Warehouse record could not be found (ccmr)");
		sprintf (errMess [0], "ccmr_co_no [%s] ccmr_est_no [%s] ccmr_cc_no [%s]",
							ccmr_rec.co_no, ccmr_rec.est_no, ccmr_rec.cc_no);
		fprintf (errFileOutput, "%s\n", errMess [0]);
		fprintf (errFileOutput, "%s\n", errMess [1]);
		return (errCode);
	}
	fclose (errFileOutput);
	return (errCode);
}

/*==========================
| Process UOM conversions. |
==========================*/
void
ProcessInum (void)
{
	number	_stdCnvFct;
	number	_altCnvFct;
	number	_cnvFct;
	number	_result;
	number	_uomCfactor;

	/*-------------------------------
	| Get the UOM conversion factor	|
	-------------------------------*/
	inum_rec.hhum_hash	=	inmr_rec.alt_uom;
	cc = find_rec (inum, &inum_rec, EQUAL, "r");
	if (cc)
		file_err (cc, inum, "DBFIND");

	sprintf (altUomGroup, "%-20.20s", inum_rec.uom_group);
	strcpy (altUom, inum_rec.uom);
	altCnvFct 	= inum_rec.cnv_fct;
	altHhumHash	= inum_rec.hhum_hash;

	inum_rec.hhum_hash	=	inmr_rec.std_uom;
	cc = find_rec (inum, &inum_rec, EQUAL, "r");
	if (cc)
		file_err (cc, inum, "DBFIND");

	strcpy (stdUom, inum_rec.uom);
	strcpy (stdUomGroup, inum_rec.uom_group);
	stdCnvFct	= inum_rec.cnv_fct;
	stdHhumHash	= inum_rec.hhum_hash;


	/*------------------------------------------------
	| converts a float to arbitrary precision number |
	| defined as number.                             |
	------------------------------------------------*/
	NumFlt (&_stdCnvFct, stdCnvFct);
	NumFlt (&_altCnvFct, altCnvFct);
	NumFlt (&_cnvFct, 	 stdCnvFct);

	/*----------------------------------------------------------
	| a function that divides one number by another and places |
	| the result in another number defined variable            |
	| Conversion factor = std uom cnvFct / iss uom cnvFct    |
	|      OR                                                  |
	| Conversion factor = (std uom cnvFct / iss uom cnvFct)  |
	|                     * item's conversion factor           |
	| Same calculations as in pc_recprt.                       |
	----------------------------------------------------------*/
	if (strcmp (altUomGroup, stdUomGroup))
		NumDiv (&_stdCnvFct, &_cnvFct, &_result);
	else
	{
		NumFlt (&_uomCfactor,inmr_rec.uom_cfactor);
		NumDiv (&_altCnvFct, &_cnvFct, 		&_result);
		NumMul (&_result, 	 &_uomCfactor, 	&_result);
	}
	/*---------------------------------------
	| converts a arbitrary precision number |
	| to a float                            |
	---------------------------------------*/
	cnvFct = NumToFlt (&_result);
}

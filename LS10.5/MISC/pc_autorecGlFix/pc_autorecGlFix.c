/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: pc_autorecGlFix.c,v 5.0 2002/05/08 01:22:59 scott Exp $
|  Program Name  : (pc_autorecGlFix.c)
|  Program Desc  : (Automatic Production Control Receipts - GlFix)
|---------------------------------------------------------------------|
|  Author        : Scott B Darrow  | Date Written : 30th March 2001   |
|---------------------------------------------------------------------|
| $Log: pc_autorecGlFix.c,v $
| Revision 5.0  2002/05/08 01:22:59  scott
| CVS administration
|
| Revision 1.8  2001/08/09 09:49:49  scott
| Updated to add FinishProgram () function
|
| Revision 1.7  2001/08/06 23:31:25  scott
| RELEASE 5.0
|
| Revision 1.6  2001/07/01 01:32:44  scott
| Updated to ensure category is used for all interfaces
|
| Revision 1.5  2001/06/23 14:56:49  cha
| ..
|
| Revision 1.4  2001/06/23 14:44:36  cha
| ..
|
| Revision 1.3  2001/06/23 14:03:57  scott
| //
|
| Revision 1.2  2001/06/23 13:43:33  cha
| ..
|
| Revision 1.1  2001/06/23 11:53:48  scott
| New program
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: pc_autorecGlFix.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/MISC/pc_autorecGlFix/pc_autorecGlFix.c,v 5.0 2002/05/08 01:22:59 scott Exp $";

#include	<pslscr.h>
#include	<GlUtils.h>
#include	<twodec.h>
#include	<proc_sobg.h>
#include	<number.h>
#include	<ml_std_mess.h>
#include	<ml_pc_mess.h>
#define		NDEC (x)		n_dec (x, inmr_rec.dec_pt)

#include	"schema"

#define		ERR_WOR_NO_ORDER	1
#define		ERR_WOR_NO_MKU		2
#define		ERR_WOR_NO_PCHS		3
#define		ERR_WOR_GL_GLI			4
#define		ERR_WOR_TIME		5
#define		ERR_WOR_WOFFCODE	6

struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct ccmrRecord	ccmr_rec;
struct inmrRecord	inmr_rec;
struct inmrRecord	inmr2_rec;
struct inumRecord	inum_rec;
struct pcglRecord	pcgl_rec;
struct pcmsRecord	pcms_rec;
struct esmrRecord	esmr_rec;
struct pchsRecord	pchs_rec;
struct pcwoRecord	pcwo_rec;

	char	*data	= 	"data",
			*inmr2	= 	"inmr2",
			*DBFIND	=	"DBFIND";

	char 	altUomGroup 	 [21],
			stdUomGroup 	 [21],
			stdUom  		 [5],
			altUom  		 [5];

	float	stdCnvFct	=	0.00,
			altCnvFct	=	0.00,
			cnvFct		=	0.00;

	long	stdHhumHash	=	0L,
			altHhumHash	=	0L;

	char	localCurrency [4];

	double	batchTotal 		= 0.00,
			quantityTotal 	= 0.00;

	int		printerNumber 		= 1,
			periodMonth			= 0;

	/*--------------------------------------------
	| General Ledger hashes and account numbers. |
	--------------------------------------------*/
	long	matHash 	 [3];				/* 0 - direct	1 - mfg variance */
	char	matAcc 		 [2][MAXLEVEL + 1];	/* 0 - direct	1 - mfg variance */

	long	expiryDate = 0L;

struct {
	double	matCost;			/* Material Costs		 */
} stdRec, actRec, recRec, thisRec;

Date	receiptDate;
double	stdCost;
double	actCost;
double	actCostEach;
float	outerSize;
char	creditAccount [MAXLEVEL + 1];
long	creditHash;

/*=====================
| function prototypes |
=====================*/
double 	CalcPcms 			 (void);
float 	GetUom 				 (long, long);
void 	GetAccount 			 (char *);
int		PRError 			 (int);
int 	ReadDefault 		 (char *);
int 	Update 				 (void);
void 	AddPcgl 			 (long, char *, char *, double, char *, char *);
void 	CalcCost 			 (void);
void 	CloseDB 			 (void);
void 	OpenDB 				 (void);
void	ProcessSalesOrder 	 (long, char *, float);
void 	shutdown_prog 		 (void);
int		StartProgram 		 (void);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int	argc,
 char	*argv [])
{
	double	origQty 	= 0.00,
			workQty 	= 0.00,
			origCost 	= 0.00,
			workCost	= 0.00,
			totalCost	= 0.00;

	if (argc < 2)
		return (EXIT_FAILURE);


	cc = StartProgram ();
	if (cc)
		return (cc);

	/*-------------------
	| Find Works order. |
	-------------------*/
	pcwo_rec.hhwo_hash	=	atol (argv [1]);
	cc = find_rec (pcwo, &pcwo_rec, EQUAL, "u");
	if (cc)
	{
		abc_unlock (pcwo);
		return (PRError (ERR_WOR_NO_ORDER));
	}

	receiptDate	=	TodaysDate ();
	
	pchs_rec.hhwo_hash	=	pcwo_rec.hhwo_hash;
	cc = find_rec (pchs, &pchs_rec, EQUAL, "r");
	if (cc)
	{
		abc_unlock (pchs);
		return (PRError (ERR_WOR_NO_PCHS));
	}

	/*--------------------------------
	| Find MKU line for works order. |
	--------------------------------*/
	inmr_rec.hhbr_hash	=	pcwo_rec.hhbr_hash;
	cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
	if (cc)
		return (PRError (ERR_WOR_NO_MKU));

	if (inmr_rec.hhsi_hash)
	{
		inmr_rec.hhbr_hash	=	inmr_rec.hhsi_hash;
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		if (cc)
		{
			inmr_rec.hhbr_hash	=	pcwo_rec.hhbr_hash;
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		}
	}
	if (cc)
		return (PRError (ERR_WOR_NO_MKU));

	/*---------------------------------------------------
	| Get all the general ledger stuff, and lots of it. |
	---------------------------------------------------*/
	cc = ReadDefault (inmr_rec.category);
	if (cc)
		return (PRError (ERR_WOR_GL_GLI));
 
	/*---------------------------
	| Process control Accounts. |
	---------------------------*/
	GetAccount (inmr_rec.category);

	/*------------------
	| Calculate costs. |
	------------------*/
	CalcCost ();

	if (pchs_rec.outer_size == 0.00)
		pchs_rec.outer_size = 1.00;

	stdCost 		/= pchs_rec.outer_size;
	stdCost 		*= pchs_rec.act_prod_qty;
	stdRec.matCost	=	stdCost;

	actCost 		/= pchs_rec.outer_size;
	actCostEach 	=	actCost;
	actCost 		*= pchs_rec.act_prod_qty;
	actRec.matCost	=	actCost;

	
	memcpy ((char *)&inmr2_rec, (char *)&inmr_rec, sizeof (struct inmrRecord));

	/*--------------------------------------------------
	| Update all inventory and general ledger records. |
	--------------------------------------------------*/
	origQty 	= 0.0;
	workQty 	= 0.0;
	origCost 	= 0.0;

	totalCost 	=	CalcPcms ();
	totalCost	=	out_cost (totalCost, pchs_rec.outer_size);
	totalCost	*=	pchs_rec.act_prod_qty;

	/*--------------------------------------------
	| Add General Ledger inventory transactions. |
	--------------------------------------------*/
	memset (&thisRec, 0, sizeof (thisRec));
    
	/* post this receipt cost to Mfg item stock account */
	AddPcgl 
	 (
		creditHash,
		creditAccount,
		"RECPT ",
		 (totalCost > 0.00) ? totalCost : -totalCost,
		 (totalCost > 0.00) ? "1" : "2",
		"19"
	);

	/* post std cost of materials to the material account */
	thisRec.matCost = stdRec.matCost;
	totalCost -= stdRec.matCost;

	AddPcgl 
	 (
		matHash [0],
		matAcc [0],
		"WIP CRD",
		 (stdRec.matCost > 0.00) ? stdRec.matCost : -stdRec.matCost,
		 (stdRec.matCost > 0.00) ? "2" : "1",
		"19"
	);

	/*------------------------------
	| Write-Off rejected quantity. |
	------------------------------*/
	if (pcwo_rec.act_rej_qty > 0.00)
	{
		double	value;

		value	=	out_cost (actCostEach, pchs_rec.outer_size);
		value	*=	pchs_rec.act_rej_qty;

		AddPcgl 
		 (
			matHash [2],
			matAcc [2],
			"SCRAP",
			value,
			"1",
			"12"
		);
		AddPcgl 
		 (
			creditHash,
			creditAccount,
			"SCRAP",
			value,
		 	"2",
			"12"
		);
	}
	workCost = thisRec.matCost + recRec.matCost;
	workCost = actRec.matCost - workCost;
	totalCost -= workCost;
	AddPcgl 
	 (
		matHash [1],
		matAcc [1],
		"MAT VAR",
		 (workCost > 0) ? workCost : -workCost,
		 (workCost > 0) ? "2" : "1",
		"19"
	);
	/* post total act cost */
	AddPcgl
	 (
		creditHash,
		creditAccount,
		"REC ACT",
		 actCost,
		"1",
		"19"
	);
	/* post total std cost */
	AddPcgl
	 (
		creditHash,
		creditAccount,
		"REC STD",
		 stdCost,
		"2",
		"19"
	);
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

	/*----------------------------
	| Setup General Ledger Mask. |
	----------------------------*/
	GL_SetMask ("XXXXXXXXXXXXXXXX");

	DateToDMY (comm_rec.inv_date, NULL, &periodMonth, NULL);

	return (EXIT_SUCCESS);
}

/*========================
| Program exit sequence. |
========================*/
void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*========================
| Open data base files . |
========================*/
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

	abc_alias (inmr2, inmr);

	open_rec (inum,  inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	open_rec (ccmr,  ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	open_rec (inmr,  inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (inmr2, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (pcms,  pcms_list, PCMS_NO_FIELDS, "pcms_id_no");
	open_rec (pcwo,  pcwo_list, PCWO_NO_FIELDS, "pcwo_hhwo_hash");
	open_rec (esmr,  esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (pcgl,  pcgl_list, PCGL_NO_FIELDS, "pcgl_id_no");
	open_rec (pchs,  pchs_list, PCHS_NO_FIELDS, "pchs_hhwo_hash");

	OpenGlmr ();

	/*----------------------------------
	| Read ccmr for current warehouse. |
	----------------------------------*/
	strcpy (ccmr_rec.co_no,  comm_rec.co_no);
	strcpy (ccmr_rec.est_no, comm_rec.est_no);
	strcpy (ccmr_rec.cc_no,  comm_rec.cc_no);
	cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, (char *)ccmr, DBFIND);

	GL_OpenBatch (comm_rec.co_no, comm_rec.est_no);

}

/*========================
| Close data base files. |
========================*/
void
CloseDB (
 void)
{
	abc_fclose (inum);
	abc_fclose (ccmr);
	abc_fclose (glmr);
	abc_fclose (inmr);
	abc_fclose (pcms);
	abc_fclose (pcwo);
	abc_fclose (esmr);
	abc_fclose (pcgl);
	abc_fclose (pchs);
	GL_CloseBatch (0);
	GL_Close ();
	abc_dbclose (data);
}

/*========================================
| Read all nessasary files for defaults. |
========================================*/
int
ReadDefault (
char	*category)
{
	abc_selfield (glmr, "glmr_id_no");

	strcpy (ccmr_rec.co_no,  comm_rec.co_no);
	strcpy (ccmr_rec.est_no, pcwo_rec.rec_br_no);
	strcpy (ccmr_rec.cc_no,  pcwo_rec.rec_wh_no);
	cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
	if (cc)
		return (cc);
		
	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"WIP D MATL",
		" ",
		category
	);
	matHash [0] = glmrRec.hhmr_hash;
	strcpy (matAcc [0], glmrRec.acc_no);

	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"MAN D MATL",
		" ",
		category
	);
	matHash [1] = glmrRec.hhmr_hash;
	strcpy (matAcc [1], glmrRec.acc_no);

	abc_selfield (glmr, "glmr_hhmr_hash");

	return (EXIT_SUCCESS);
}

/*------------------------------
| Calculate cost of production |
| based on materials and cost  |
| of time through routing.     |
------------------------------*/
void
CalcCost (
 void)
{
	stdCost	=	pchs_rec.std_cost;
	actCost	=	pchs_rec.act_cost;
}
/*---------------------------------------
| Update $amt recptd on pcms records	|
---------------------------------------*/
double	
CalcPcms (void)
{
	return (pchs_rec.std_cost);
}

/*===========================
| Process control Accounts	|
===========================*/
void
GetAccount (
 	char 	*category)
{
	abc_selfield (glmr, "glmr_id_no");

	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"COSTSALE C",
		" ",
		category
	);
	strcpy (creditAccount, glmrRec.acc_no);
	creditHash = glmrRec.hhmr_hash;

	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"COSTSALE M",
		" ",
		category
	);
	matHash [2] = glmrRec.hhmr_hash;
	strcpy (matAcc [2], glmrRec.acc_no); 

	abc_selfield (glmr, "glmr_hhmr_hash");
}
/*===============================================================
| Add a trans to the pcgl file.	NB: amount should be in cents	|
===============================================================*/
void
AddPcgl 
 (
	long	hhmrHash,
	char	*accountNumber,
	char	*glReference,
	double	amount,
	char	*type,
	char	*tranType
)
{
	if (amount == 0)
		return;

	strcpy (pcgl_rec.co_no, comm_rec.co_no);
	strcpy (pcgl_rec.tran_type, tranType);
	pcgl_rec.post_date = comm_rec.inv_date;
	pcgl_rec.tran_date = comm_rec.inv_date;

	DateToDMY (comm_rec.inv_date, NULL, &periodMonth, NULL);
	sprintf (pcgl_rec.period_no,"%02d", periodMonth);
	sprintf (pcgl_rec.sys_ref, "%5.1d", comm_rec.term);
	sprintf (pcgl_rec.user_ref, "%8.8s", glReference);
	strcpy (pcgl_rec.stat_flag, "2");
	sprintf (pcgl_rec.narrative, "%-20.20s", pcwo_rec.order_no);
	pcgl_rec.amount 		= CENTS (amount);
	pcgl_rec.loc_amount 	= CENTS (amount);
	pcgl_rec.exch_rate		=	1.00;
	strcpy (pcgl_rec.currency, localCurrency);
	strcpy (pcgl_rec.acc_no, accountNumber);
	pcgl_rec.hhgl_hash 		= hhmrHash;

	strcpy (pcgl_rec.jnl_type, type);

	pin_bcopy ((char *) &glwkRec, (char *) &pcgl_rec, sizeof (glwkRec));
	GL_AddBatch ();
}
/*=======================================================================
| Process Errors, did not forget ML calls, just that these messages are |
| used for debug use and it's a bit hard when converted to non english  |
=======================================================================*/
int
PRError (
	int	errCode)
{
	char	errMess [3][61];

	if (ERR_WOR_NO_ORDER == errCode)
	{
		strcpy (errMess [0], "Works order hash (pcwo_hhwo_hash) not valid");
		sprintf (errMess [1], "pcwo_hhwo_hash = [%ld]", pcwo_rec.hhwo_hash);
		fprintf (stderr, "%s\n", errMess [0]);
		fprintf (stderr, "%s\n", errMess [1]);
		return (errCode);
	}
	if (ERR_WOR_NO_PCHS == errCode)
	{
		strcpy (errMess [0], "Works order history hash (pchs_hhwo_hash) not valid");
		sprintf (errMess [1], "pchs_hhwo_hash = [%ld]", pcwo_rec.hhwo_hash);
		fprintf (stderr, "%s\n", errMess [0]);
		fprintf (stderr, "%s\n", errMess [1]);
		return (errCode);
	}
	if (ERR_WOR_NO_MKU == errCode)
	{
		strcpy (errMess [0], "MKU could not be found for works order");
		sprintf (errMess [1], "inmr_hhbr_hash = [%ld]", inmr_rec.hhbr_hash);
		sprintf (errMess [2], "pcms_hhwo_hash = [%ld]", pcwo_rec.hhwo_hash);

		fprintf (stderr, "%s\n", errMess [0]);
		fprintf (stderr, "%s\n", errMess [1]);
		fprintf (stderr, "%s\n", errMess [2]);

		return (errCode);
	}
	if (ERR_WOR_GL_GLI == errCode)
	{
		strcpy (errMess [0], "General Ledger interface not defined.");
		fprintf (stderr, "%s\n", errMess [0]);
		return (errCode);
	}
	return (errCode);
}

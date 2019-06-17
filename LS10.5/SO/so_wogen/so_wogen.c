/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: so_wogen.c,v 5.3 2001/12/12 10:03:35 robert Exp $
|  Program Name  : (so_wogen.c)                          
|  Program Desc  : (Sales order to works order generation)
|---------------------------------------------------------------------|
|  Date Written  : (28th Feb 2001) | Author      : Scott B Darrow     |
|---------------------------------------------------------------------|
| $Log: so_wogen.c,v $
| Revision 5.3  2001/12/12 10:03:35  robert
| LS10.5-GUI update
|
| Revision 5.2  2001/08/09 09:22:11  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:52:13  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:20:51  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.1  2001/03/27 07:07:43  scott
| Updated as program should have created (tried) pcms records from bmms.
| Did a ASSUME that records existed and yes made a ASS U ME.
|
| Revision 4.0  2001/03/09 02:42:08  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 1.3  2001/03/06 05:08:51  scott
| Updated to update pcwo_hhsl_hash for automatic release.
|
| Revision 1.2  2001/02/28 07:28:20  scott
| Updated to add additional error logging
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: so_wogen.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_wogen/so_wogen.c,v 5.3 2001/12/12 10:03:35 robert Exp $";

#include 	<pslscr.h>	

#ifdef GVISION
#include	<RemoteFile.h>
#include	<OpenSpecial.h>
#endif

#include 	<proc_sobg.h>
#include 	<twodec.h>
#include	"schema"

	struct	comrRecord	comr_rec;
	struct	ccmrRecord	ccmr_rec;
	struct	solnRecord	soln_rec;
	struct	inmrRecord	inmr_rec;
	struct	inumRecord	inum_rec;
	struct	inccRecord	incc_rec;
	struct 	bmmsRecord	bmms_rec;
	struct 	bmmsRecord	bmms2_rec;
	struct 	ineiRecord	inei_rec;
	struct 	inmrRecord	inmr2_rec;
	struct 	pcbpRecord	pcbp_rec;
	struct 	pclnRecord	pcln_rec;
	struct 	pcmsRecord	pcms_rec;
	struct 	pcwoRecord	pcwo_rec;
	struct 	pcwoRecord	pcwo2_rec;
	struct 	rgbpRecord	rgbp_rec;
	struct 	rghrRecord	rghr_rec;
	struct 	rglnRecord	rgln_rec;

	char	*bmms2	= "bmms2",
			*inmr2	= "inmr2",
			*pcwo2	= "pcwo2",
			*pcwo3	= "pcwo3";

	void 	CheckIncc 				(long, long);

	long	hhwoHash		= 0L,
			lsystemDate		= 0L;

	int		bomAlternate		= 0,
			rtgAlternate		= 0,
			printerNumber 		= 1,
			printerOutputOpen	= FALSE;

	float	qtyReqCalc 			= 0.00,
			qtyPrdCalc 			= 0.00,
			quantityRequired	= 0.00;

	FILE	*fin,
			*fout;

	char	*currentUser,
			envSupOrdRound [2],
			envSoWoLevel [2];

/*===========================
| Local Function Prototype. |
===========================*/
static 	float RoundMultiple (float, char *, float, float);
int 	CheckPcwo 			(long);
void	shutdown_prog 		(void);
void	StartProgram 		(void);
void 	AddWorksOrder		(void);
void 	CloseDB 			(void);
void 	HeadingPrint 		(void);
void 	LogError 			(char *);
void 	OpenDB 				(void);
void 	Process 			(long);
int		WO_Create 			(long);
void 	WO_UpdatePcwo 		(long);
void 	WO_UpdatePcms 		(void);
void 	WO_UpdatePcln 		(void);
void 	WO_UpdatePcbp 		(void);

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int    argc,
 char*  argv [])
{
	long	hhslHash	=	0L;

	StartProgram ();

	if (scanf ("%d", &printerNumber) == EOF)
		return (EXIT_FAILURE);

	while (scanf ("%ld", &hhslHash) != EOF)
	{
		if (hhslHash)
			Process (hhslHash);
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*============================
| Start up the normal stuff. |
============================*/
void
StartProgram (void)
{
	char	*sptr;

	currentUser = getenv ("LOGNAME");
	lsystemDate = TodaysDate ();

	/*----------------------------------------------
	| Process Supplier order rounting environment. |
	----------------------------------------------*/
	sptr = get_env ("SUP_ORD_ROUND");
	if (sptr == (char *) 0)
		sprintf (envSupOrdRound, "B");
	else
	{
		switch (*sptr)
		{
		case	'U':
		case	'u':
			sprintf (envSupOrdRound, "U");
			break;

		case	'D':
		case	'd':
			sprintf (envSupOrdRound, "D");
			break;

		default:
			sprintf (envSupOrdRound, "B");
			break;
		}
	}

	/*-----------------------------------------------------------------------
	| Process Sales order to Works Order processing level for BOM Defaults. |
	-----------------------------------------------------------------------*/
	sptr = get_env ("SO_WO_LEVEL");
	if (sptr == (char *) 0)
		sprintf (envSoWoLevel, "C");
	else
	{
		switch (*sptr)
		{
		case	'C':
		case	'c':
			sprintf (envSoWoLevel, "C");
			break;

		case	'B':
		case	'b':
			sprintf (envSoWoLevel, "B");
			break;

		case	'W':
		case	'w':
			sprintf (envSoWoLevel, "W");
			break;

		default:
			sprintf (envSoWoLevel, "C");
			break;
		}
	}
	OpenDB ();
}

/*========================
| Program exit sequence. |
========================*/
void
shutdown_prog (void)
{
	if (printerOutputOpen == TRUE)
	{
		fprintf (fout,".EOF\n");
		pclose (fout);
	}
	CloseDB (); 
	FinishProgram ();
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (void)
{
	abc_dbopen ("data");

	open_rec (soln, soln_list, SOLN_NO_FIELDS, "soln_hhsl_hash");
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_hhcc_hash");
	open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");
	open_rec (inum, inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	open_rec (inei, inei_list, INEI_NO_FIELDS, "inei_id_no");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");

	abc_alias (bmms2, bmms);
	abc_alias (inmr2, inmr);
	abc_alias (pcwo2, pcwo);
	abc_alias (pcwo3, pcwo);

	open_rec (bmms,  bmms_list, BMMS_NO_FIELDS, "bmms_id_no");
	open_rec (bmms2, bmms_list, BMMS_NO_FIELDS, "bmms_id_no_2");
	open_rec (inmr2, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (pcbp,  pcbp_list, PCBP_NO_FIELDS, "pcbp_id_no");
	open_rec (pcln,  pcln_list, PCLN_NO_FIELDS, "pcln_id_no");
	open_rec (pcms,  pcms_list, PCMS_NO_FIELDS, "pcms_id_no");
	open_rec (pcwo,  pcwo_list, PCWO_NO_FIELDS, "pcwo_id_no");
	open_rec (pcwo2, pcwo_list, PCWO_NO_FIELDS, "pcwo_id_no3");
	open_rec (rgbp,  rgbp_list, RGBP_NO_FIELDS, "rgbp_id_no");
	open_rec (rghr,  rghr_list, RGHR_NO_FIELDS, "rghr_id_no");
	open_rec (rgln,  rgln_list, RGLN_NO_FIELDS, "rgln_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (comr);
	abc_fclose (soln);
	abc_fclose (inum);
	abc_fclose (inei);
	abc_fclose (incc);
	abc_fclose (inmr);
	abc_fclose (ccmr);

	abc_fclose (bmms);
	abc_fclose (bmms2);
	abc_fclose (inmr2);
	abc_fclose (pcbp);
	abc_fclose (pcln);
	abc_fclose (pcms);
	abc_fclose (pcwo);
	abc_fclose (pcwo2);
	abc_fclose (rgbp);
	abc_fclose (rghr);
	abc_fclose (rgln);
	abc_dbclose ("data");
}

/*===========================
| Report heading for Audit. |
===========================*/
void
HeadingPrint (void)
{
	if (printerOutputOpen == TRUE)
		return;

	printerOutputOpen	=	TRUE;

	if ((fout = popen ("pformat","w")) == NULL)
		file_err (errno, "pformat", PNAME);

	fprintf (fout, ".START%s\n",DateToString (lsystemDate));
	fprintf (fout, ".LP%d\n",printerNumber);
	fprintf (fout, ".SO\n");

	fprintf (fout, ".12\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L150\n");
	fprintf (fout, ".B1\n");
	fprintf (fout, ".EWORK ORDER GENERATION REPORT\n");
	fprintf (fout, ".B1\n");
	fprintf (fout, ".E%s - %s\n", comr_rec.co_no, comr_rec.co_name);
	fprintf (fout, ".B1\n");
	fprintf (fout, ".EAS AT %-24.24s\n",SystemTime ());

	fprintf (fout,".R===================");
	fprintf (fout,"===========================================");
	fprintf (fout,"===================");
	fprintf (fout,"=============");
	fprintf (fout,"======");
	fprintf (fout,"===============");
	fprintf (fout,"============\n");

	fprintf (fout,"===================");
	fprintf (fout,"===========================================");
	fprintf (fout,"===================");
	fprintf (fout,"=============");
	fprintf (fout,"======");
	fprintf (fout,"===============");
	fprintf (fout,"============\n");

	fprintf (fout,"|   PART NUMBER.   ");
	fprintf (fout,"| WORKS ORDER NO   ");
	fprintf (fout,"|   PART DESCRIPTION                       ");
	fprintf (fout,"| ORDER QTY. ");
	fprintf (fout,"| UOM ");
	fprintf (fout,"| STD UNIT COST");
	fprintf (fout,"| DUE DATE |\n");

	fprintf (fout,"|------------------");
	fprintf (fout,"|------------------");
	fprintf (fout,"|------------------------------------------");
	fprintf (fout,"|------------");
	fprintf (fout,"|-----");
	fprintf (fout,"|--------------");
	fprintf (fout,"|----------|\n");

	fflush (fout);
}
/*==========================
| Main processing routine. |
==========================*/
void
Process (
	long	hhslHash)
{
	/*------------------------------------
	| Find Sales order line and process. |
	------------------------------------*/
	soln_rec.hhsl_hash	=	hhslHash;
	cc = find_rec (soln, &soln_rec, COMPARISON, "r");
	if (cc)
	{
		sprintf 
		(
			err_str, 
			"Failed find_rec on soln. (soln_hhsl_hash = %ld) (cc = %d)",
			hhslHash, 
			cc
		);
		LogError (err_str);
		return;
	}

	/*---------------------------------------------------------------
	| Only process lines that have a status of 'W' for Works Order. |
	---------------------------------------------------------------*/
	if (soln_rec.status [0] != 'W')
	{
		sprintf 
		(
			err_str, 
			"Failed processing as soln_status = [%s] and should be [W]",
			soln_rec.status
		);
		LogError (err_str);
		return;
	}

	/*------------------------------------------------
	| Only process lines that have a quantity > 0.00 |
	------------------------------------------------*/
	quantityRequired	= soln_rec.qty_order + soln_rec.qty_bord;
	if (quantityRequired <= 0.00)
	{
		sprintf 
		(
			err_str, 
			"Failed processing as quantity %.2f is <= 0.00",
			quantityRequired
		);
		LogError (err_str);
		return;
	}

	/*------------------------------------------------------------------------
	| Find warehouse master record. Will give company/branch/warehouse info. |
	------------------------------------------------------------------------*/
	ccmr_rec.hhcc_hash	=	soln_rec.hhcc_hash;
	cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
	if (cc)
	{
		sprintf 
		(
			err_str, 
			"Failed find_rec on ccmr. (ccmr_hhcc_hash = %ld) (cc = %d)",
			soln_rec.hhcc_hash,
			cc
		);
		LogError (err_str);
		return;
	}

	/*----------------------------------
	| Find Company Master file record. |
	----------------------------------*/
	strcpy (comr_rec.co_no, ccmr_rec.co_no);
	cc = find_rec (comr, &comr_rec, COMPARISON, "r");
	if (cc)
	{
		sprintf 
		(
			err_str, 
			"Failed find_rec on comr. (comr_co_no = %s) (cc = %d)",
			ccmr_rec.co_no,
			cc
		);
		LogError (err_str);
		return;
	}

	/*------------------------
	| Read item Master file. |
	------------------------*/
	inmr_rec.hhbr_hash	=	soln_rec.hhbr_hash;
	cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
	if (cc)
	{
		sprintf 
		(
			err_str, 
			"Failed find_rec on inmr. (inmr_hhbr_hash = %ld) (cc = %d)",
			soln_rec.hhbr_hash,
			cc
		);
		LogError (err_str);
		return;
	}

	/*-----------------------------------
	| Ensure product source = MP or MC. |
	-----------------------------------*/
	if (inmr_rec.source [0] != 'M')
	{
		sprintf 
		(
			err_str, 
			"Failed process as item source = [%s] and should be [M(C,P)]",
			inmr_rec.source
		);
		LogError (err_str);
		return;
	}

	/*-------------------------------
	| Read inventory branch Record. |
	-------------------------------*/
	inei_rec.hhbr_hash	=	soln_rec.hhbr_hash;
	strcpy (inei_rec.est_no, ccmr_rec.est_no);
	cc = find_rec (inei, &inei_rec, COMPARISON,"r");
	if (cc) 
	{
		sprintf 
		(
			err_str, 
			"Failed find_rec on inei. (inei_id_no = [%ld][%s]) (cc = %d)",
			soln_rec.hhbr_hash,
			ccmr_rec.est_no,
			cc
		);
		LogError (err_str);
		return;
	}

	/*----------------------------------
	| Read inventory warehouse Record. |
	----------------------------------*/
	incc_rec.hhbr_hash = soln_rec.hhbr_hash;
	incc_rec.hhcc_hash = soln_rec.hhcc_hash;
	cc = find_rec (incc, &incc_rec, COMPARISON, "r");
	if (cc)
	{
		sprintf 
		(
			err_str, 
			"Failed find_rec on incc. (incc_id_no = [%ld][%ld]) (cc = %d)",
			soln_rec.hhbr_hash,
			soln_rec.hhcc_hash,
			cc
		);
		LogError (err_str);
		return;
	}

	/*-----------------------------
	| Set default bom and routes. |
	-----------------------------*/
	bomAlternate = inmr_rec.dflt_bom;
	rtgAlternate = inmr_rec.dflt_rtg;

	if (envSoWoLevel [0] == 'B' && inei_rec.dflt_bom)
		bomAlternate = inei_rec.dflt_bom;

	if (envSoWoLevel [0] == 'B' && inei_rec.dflt_rtg)
		rtgAlternate = inei_rec.dflt_rtg;

	if (envSoWoLevel [0] == 'W' && incc_rec.dflt_bom)
		bomAlternate = incc_rec.dflt_bom;

	if (envSoWoLevel [0] == 'W' && incc_rec.dflt_rtg)
		rtgAlternate = incc_rec.dflt_bom;

	/*------------------------------------
	| Fit quantity to batch constraints. |
	------------------------------------*/
	quantityRequired	=	RoundMultiple 
						 	(
								quantityRequired, 
								envSupOrdRound, 
								inei_rec.prd_multiple, 
								inei_rec.min_batch
							);

	/*------------------------
	| Find item standard UOM |
	------------------------*/
	inum_rec.hhum_hash	= inmr_rec.std_uom;
	cc = find_rec (inum, &inum_rec, COMPARISON, "r");
	if (cc)
		strcpy (inum_rec.uom, inmr_rec.sale_unit);

	/*--------------------------------------
	| Open printer output, only done once. |
	--------------------------------------*/
	HeadingPrint ();

	/*---------------------
	| Create Works order. |
	---------------------*/
	if (WO_Create (ccmr_rec.hhcc_hash))
		return;

	/*-------------------------------------------------
	| Write a printed log of what is being processed. |
	-------------------------------------------------*/
	fprintf (fout,"| %s ",		inmr_rec.item_no);
	fprintf (fout,"| %-16.16s ",pcwo_rec.order_no);
	fprintf (fout,"| %s ",		inmr_rec.description);
	fprintf (fout,"| %10.2f ",	quantityRequired);
	fprintf (fout,"|%-4.4s ",	inum_rec.uom);
	fprintf (fout,"| %12.2f ", 	inei_rec.std_cost);
	fprintf (fout,"|%10.10s|\n",DateToString (pcwo_rec.reqd_date));
	return;
}

/*================================================
| Log errors found why record would not process. |
================================================*/
void
LogError (
	char	*failedMessage)
{
	char	*sptr;
	FILE 	*errFileOutput;

	sptr	=	OpenSpecial ("LOG", "so_wogen.log");
	if (!sptr)
		return;

#ifndef GVISION
	if ((errFileOutput = fopen (sptr, "a")) == NULL)
		return;

	fprintf (errFileOutput, "==============================================\n");
	fprintf (errFileOutput, "Version  : %s\n", PROG_VERSION);
	fprintf (errFileOutput, "Date     : %s\n", SystemTime ());
	fprintf (errFileOutput, "PROG_PATH: %s\n", getenv ("PROG_PATH"));
	fprintf (errFileOutput, "DBPATH   : %s\n", getenv ("DBPATH"));
	fprintf (errFileOutput, "Comments : %s\n", failedMessage);
	fprintf (errFileOutput, "==============================================\n");
	fclose (errFileOutput);
#else
	if ((errFileOutput = Remote_fopen (sptr, "a")) == NULL)
		return;

	Remote_fprintf (errFileOutput, "==============================================\n");
	Remote_fprintf (errFileOutput, "Version  : %s\n", PROG_VERSION);
	Remote_fprintf (errFileOutput, "Date     : %s\n", SystemTime ());
	Remote_fprintf (errFileOutput, "PROG_PATH: %s\n", getenv ("PROG_PATH"));
	Remote_fprintf (errFileOutput, "DBPATH   : %s\n", getenv ("DBPATH"));
	Remote_fprintf (errFileOutput, "Comments : %s\n", failedMessage);
	Remote_fprintf (errFileOutput, "==============================================\n");
	Remote_fclose (errFileOutput);
#endif
}

static 
float
RoundMultiple (
	float  orderQty,
	char*  roundType,
	float  orderMultiple,
	float  minimumQty)
{
	double	wrk_qty;
	double	up_qty;
	double	down_qty;

	if (orderQty == 0.00)
		return (0.00);

	if (orderMultiple == 0.00)
		return ((orderQty < minimumQty) ? minimumQty : orderQty);

	orderQty -= minimumQty;
	if (orderQty < 0.00)
		orderQty = 0.00;

	/*---------------------------
	| Already An Exact Multiple |
	---------------------------*/
	wrk_qty = (double) (orderQty / orderMultiple);
	if (ceil (wrk_qty) == wrk_qty)
		return (orderQty + minimumQty);

	/*------------------
	| Perform Rounding |
	------------------*/
	switch (roundType [0])
	{
	case 'U':
		/*------------------------------
		| Round Up To Nearest Multiple |
		------------------------------*/
		wrk_qty = (double) (orderQty / orderMultiple);
		wrk_qty = ceil (wrk_qty);
		orderQty = (float) (wrk_qty * orderMultiple);
		break;

	case 'D':
		/*--------------------------------
		| Round Down To Nearest Multiple |
		--------------------------------*/
		wrk_qty = (double) (orderQty / orderMultiple);
		wrk_qty = floor (wrk_qty);
		orderQty = (float) (wrk_qty * orderMultiple);
		break;

	case 'B':
		/*--------------------------
		| Find Value If Rounded Up |
		--------------------------*/
		up_qty = (double) orderQty;
		wrk_qty = (up_qty / (double)orderMultiple);
		wrk_qty = ceil (wrk_qty);
		up_qty = (float) (wrk_qty * orderMultiple);

		/*----------------------------
		| Find Value If Rounded Down |
		----------------------------*/
		down_qty = (double) orderQty;
		wrk_qty = (down_qty / (double) orderMultiple);
		wrk_qty = floor (wrk_qty);
		down_qty = (float) (wrk_qty * orderMultiple);

		/*-----------------------------------
		| Round Up/Down To Nearest Multiple |
		-----------------------------------*/
		if ((up_qty - (double) orderQty) <= ((double) orderQty - down_qty))
			orderQty = (float) up_qty;
		else
			orderQty = (float) down_qty;

		break;

	default:
		break;
	}

	return (minimumQty + orderQty);
}

/*=======================
| Create a Works Order. |
=======================*/
int
WO_Create (
	long	hhccHash)
{
	WO_UpdatePcwo (hhccHash);

	strcpy (rghr_rec.co_no, ccmr_rec.co_no);
	strcpy (rghr_rec.br_no, ccmr_rec.est_no);
	rghr_rec.hhbr_hash 	= pcwo_rec.hhbr_hash;
	rghr_rec.alt_no 	= pcwo_rec.rtg_alt;
	cc = find_rec (rghr, &rghr_rec, COMPARISON, "r");
	if (cc)
	{
		sprintf 
		(
			err_str, 
			"Failed find_rec on rghr. (rghr_id_no = [%s][%s][%ld][%d]) (cc %d)",
			rghr_rec.co_no, 
			rghr_rec.br_no, 
			rghr_rec.hhbr_hash, 
			rghr_rec.alt_no,
			cc
		);
		LogError (err_str);
		return (EXIT_FAILURE);
	}

	if (qtyReqCalc == qtyPrdCalc)
		return (EXIT_SUCCESS);

	/*-------------------------------
	| Step 1: Update pcms as needed	|
	-------------------------------*/
	WO_UpdatePcms ();

	/*-------------------------------
	| Step 2: Update pcln as needed	|
	-------------------------------*/
	WO_UpdatePcln ();

	/*-------------------------------
	| Step 3: Update pcbp if needed	|
	-------------------------------*/
	WO_UpdatePcbp ();

	abc_unlock (pcwo);
	recalc_sobg ();
	return (EXIT_SUCCESS);
}

/*======================================
| Production Control Works Order file. |
======================================*/
void
WO_UpdatePcwo (
	long	hhccHash)
{
	qtyPrdCalc = (float) n_dec (inei_rec.std_batch, inmr_rec.dec_pt);

	/*----------------------------------------------------
	| get the next works order number from the ccmr file |
	----------------------------------------------------*/
	ccmr_rec.hhcc_hash	=	hhccHash;
	cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "u");
	if (cc)
		file_err (cc, ccmr, "DBFIND");

	open_rec (pcwo3, pcwo_list, PCWO_NO_FIELDS, "pcwo_id_no");
	while (TRUE)
	{
		if (CheckPcwo (ccmr_rec.nx_wo_num))
			break;
		ccmr_rec.nx_wo_num ++;
	}
	abc_fclose (pcwo3);

	strcpy (pcwo_rec.co_no, ccmr_rec.co_no);
	strcpy (pcwo_rec.br_no, ccmr_rec.est_no);
	strcpy (pcwo_rec.wh_no, ccmr_rec.cc_no);

	sprintf (pcwo_rec.order_no, "%07ld", ccmr_rec.nx_wo_num);

	ccmr_rec.nx_wo_num ++;
	cc = abc_update (ccmr, &ccmr_rec);
	if (cc)
		file_err (cc, ccmr, "DBUPDATE");

	strcpy (pcwo_rec.req_br_no, ccmr_rec.est_no);
	strcpy (pcwo_rec.req_wh_no, ccmr_rec.cc_no);
	strcpy (pcwo_rec.rec_br_no, ccmr_rec.est_no);
	strcpy (pcwo_rec.rec_wh_no, ccmr_rec.cc_no);
	strcpy (pcwo_rec.batch_no,  "          ");
	pcwo_rec.reqd_date 		= lsystemDate + incc_rec.acc_mlt;
	pcwo_rec.rtg_seq 		= 0;
	pcwo_rec.priority 		= 5;	/* SBD TO CHECK */
	sprintf (pcwo_rec.op_id, "%-14.14s", currentUser);
	strcpy (pcwo_rec.create_time, 	TimeHHMM());
	pcwo_rec.create_date 	= lsystemDate;
	pcwo_rec.mfg_date 		= 0L;
	pcwo_rec.hhbr_hash 		= inmr_rec.hhbr_hash;
	pcwo_rec.bom_alt 		= bomAlternate;
	pcwo_rec.rtg_alt 		= rtgAlternate;
	pcwo_rec.hhcc_hash 		= ccmr_rec.hhcc_hash;
	pcwo_rec.prod_qty 		= quantityRequired;
	pcwo_rec.act_prod_qty 	= (float) 0.00;
	pcwo_rec.act_rej_qty 	= (float) 0.00;
	strcpy (pcwo_rec.order_status, "P");
	pcwo_rec.hhsl_hash		= soln_rec.hhsl_hash;
	strcpy (pcwo_rec.stat_flag, "0");
	cc = abc_add (pcwo, &pcwo_rec);
	if (cc)
		file_err (cc, pcwo, "DBADD");

	cc = find_rec (pcwo, &pcwo_rec, EQUAL, "u");
	if (cc)
		file_err (cc, pcwo, "DBFIND");

	hhwoHash = pcwo_rec.hhwo_hash;
	qtyReqCalc = quantityRequired;

	add_hash 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		"RC",
		0,
		pcwo_rec.hhbr_hash,
		ccmr_rec.hhcc_hash,
		0L,
		(double) 0.00
	);
}

/*========================================
| Check BOM Material Specification file. |
========================================*/
void
WO_UpdatePcms (void)
{
	/*-------------------------------
	| Copy new records from bmms	|
	-------------------------------*/
	pcwo_rec.bom_alt 		= bomAlternate;
	pcwo_rec.rtg_alt 		= rtgAlternate;

	strcpy (pcms_rec.co_no, pcwo_rec.co_no);
	pcms_rec.hhbr_hash 	= pcwo_rec.hhbr_hash;
	pcms_rec.alt_no 	= pcwo_rec.bom_alt;
	pcms_rec.line_no 	= 0;
	cc = find_rec (bmms, &pcms_rec, GTEQ, "r");
	while (!cc && !strcmp (pcms_rec.co_no, pcwo_rec.co_no) &&
					pcms_rec.hhbr_hash == pcwo_rec.hhbr_hash &&
					pcms_rec.alt_no == pcwo_rec.bom_alt)
	{
		strcpy (pcms_rec.act_qty_in, "N");
		pcms_rec.hhwo_hash = pcwo_rec.hhwo_hash;
		pcms_rec.matl_qty *= qtyReqCalc;
		pcms_rec.matl_qty /= qtyPrdCalc;
		pcms_rec.uniq_id   = pcms_rec.line_no;
		cc = abc_add (pcms, &pcms_rec);
		if (cc)
			file_err (cc, pcms, "DBADD");

		/*-----------------------------
		| Add sobg record to re-calc. |
		| the material committed qty. |
		-----------------------------*/
		add_hash 
		(
			pcwo_rec.co_no,
			pcwo_rec.br_no,
			"RC",
			0,
			pcms_rec.mabr_hash,
			ccmr_rec.hhcc_hash,
			0L,
			(double) 0
		);
		cc = find_rec (bmms, &pcms_rec, NEXT, "r");
	}
}

/*================================
| Check routing By-Product file. |
================================*/
void
WO_UpdatePcln (
 void)
{
	pcln_rec.hhwo_hash 	= pcwo_rec.hhwo_hash;
	pcln_rec.seq_no 	= 0;
	pcln_rec.line_no 	= 0;
	cc = find_rec (pcln, &pcln_rec, GTEQ, "u");
	while (!cc && pcln_rec.hhwo_hash == pcwo_rec.hhwo_hash)
	{
		pcln_rec.run *= (long)	qtyReqCalc;
		pcln_rec.run /= (long)	qtyPrdCalc;
		cc = abc_update (pcln, &pcln_rec);
		if (cc)
			file_err (cc, pcln, "DBUPDATE");

		cc = find_rec (pcln, &pcln_rec, NEXT, "u");
	}
	abc_unlock (pcln);
}

/*================================
| Check routing By-Product file. |
================================*/
void
WO_UpdatePcbp (void)
{
	pcbp_rec.hhwo_hash 	= pcwo_rec.hhwo_hash;
	pcbp_rec.seq_no 	= 0;
	pcbp_rec.hhbr_hash 	= 0L;
	cc = find_rec (pcbp, &pcbp_rec, GTEQ, "u");
	while (!cc && pcbp_rec.hhwo_hash == pcwo_rec.hhwo_hash)
	{
		pcbp_rec.qty *= qtyReqCalc;
		pcbp_rec.qty /= qtyPrdCalc;
		cc = abc_update (pcbp, &pcbp_rec);
		if (cc)
			file_err (cc, pcbp, "DBUPDATE");

		add_hash 
		(
			pcwo_rec.co_no,
			pcwo_rec.br_no,
			"RC",
			0,
			pcbp_rec.hhbr_hash,
			pcwo_rec.hhcc_hash,
			0L,
			(double) 0.00
		);

		cc = find_rec (pcbp, &pcbp_rec, NEXT, "u");
	}
	abc_unlock (pcbp);
}

/*========================================================
| Checks if the item has an incc record                  |
| if the record is not found the record is then created  |
========================================================*/
void
CheckIncc (
	long	hhccHash, 
	long	hhbrHash)
{
	incc_rec.hhcc_hash = hhccHash;
	incc_rec.hhbr_hash = hhbrHash;
	cc = find_rec (incc, &incc_rec, COMPARISON, "r");
	if (cc)
	{
		incc_rec.hhcc_hash = hhccHash;
		incc_rec.hhbr_hash = hhbrHash;
		strcpy (incc_rec.ff_option,  "A");
		strcpy (incc_rec.allow_repl, "E");
		strcpy (incc_rec.abc_code,   "A");
		strcpy (incc_rec.abc_update, "Y");

		cc = abc_add (incc, &incc_rec);
		if (cc)
			file_err (cc, incc, "DBADD");
	}
	return;
}
/*========================================================
| Check it existing works order exists with same number. |
========================================================*/
int
CheckPcwo (
	long	orderNo)
{
	strcpy (pcwo2_rec.co_no, ccmr_rec.co_no);
	strcpy (pcwo2_rec.br_no, ccmr_rec.est_no);
	strcpy (pcwo2_rec.wh_no, ccmr_rec.cc_no);
	sprintf (pcwo2_rec.order_no, "%07ld", orderNo);
	return (find_rec (pcwo3, &pcwo2_rec, COMPARISON, "r"));
}

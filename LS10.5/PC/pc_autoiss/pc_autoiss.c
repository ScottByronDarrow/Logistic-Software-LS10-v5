/*=====================================================================
|  Copyright (C) 1999 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: pc_autoiss.c,v 5.8 2001/08/09 09:14:29 scott Exp $
|  Program Name  : (pc_autoiss.h) 
|  Program Desc  : (Automatic Production Control Issue raw-materials)
|---------------------------------------------------------------------|
|  Date Written  : 29th March 2001 | Author       : Scott B Darrow    |
|---------------------------------------------------------------------|
| $Log: pc_autoiss.c,v $
| Revision 5.8  2001/08/09 09:14:29  scott
| Updated to add FinishProgram () function
|
| Revision 5.7  2001/08/06 23:34:55  scott
| RELEASE 5.0
|
| Revision 5.6  2001/07/25 02:18:20  scott
| Update - LS10.5
|
| Revision 4.9  2001/05/14 05:51:59  scott
| Updated to add new XML error logging
| Updated to change scrap postings
| Updated to add posting to pcms_qty_scrap and pcms_amt_scrap
|
| Revision 4.8  2001/05/03 10:17:50  scott
| Updated as scrapped journal around the wrong way
|
| Revision 4.7  2001/05/01 12:26:54  scott
| Updated to ensure fileds are initilised and correct indexes selected
|
| Revision 4.6  2001/04/30 09:44:53  scott
| Updated to change scrapped as per Martin L email.
|
| Revision 4.5  2001/04/30 04:46:10  cha
| Updated for message
|
| Revision 4.4  2001/04/30 04:42:53  scott
| Updated to not rely on sequence number.
| Updated to add posting of scrapped to GL
|
| Revision 4.3  2001/04/28 08:31:53  scott
| Updated to make general ledger entries the same as pc_issue.
|
| Revision 4.2  2001/04/08 08:27:55  cha
| Updated as should have performed an update not add.
|
| Revision 4.1  2001/04/06 06:44:17  scott
| Updated to use standard functions for FindFifo and FifoValue.
|
| Revision 4.0  2001/04/02 00:01:07  scott
| Updated new program to release 4.(x)
|
| Revision 1.2  2001/03/29 09:24:46  scott
| Small changes from testing.
|
| Revision 1.1  2001/03/29 06:12:29  scott
| New Program - Automatic stock issues to Production.
| Driven by XML files.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: pc_autoiss.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PC/pc_autoiss/pc_autoiss.c,v 5.8 2001/08/09 09:14:29 scott Exp $";


#include	<pslscr.h>
#include	<ml_std_mess.h>
#include	<ml_pc_mess.h>
#include	<proc_sobg.h>
#include	<GlUtils.h>
#include	<twodec.h>
#include	<number.h>
#include	<pc_autoiss.h>
#include	<XML_Error.h>
#include	<Costing.h>

#define		NONSTOCK	(inmr_rec.inmr_class [0] == 'N' || \
						 inmr_rec.inmr_class [0] == 'Z')
#define		SERIAL		(inmr_rec.serial_item [0] == 'Y')

FILE	*pp;

#include	"schema"

struct comrRecord	comr_rec;
struct ccmrRecord	ccmr_rec;
struct commRecord	comm_rec;
struct ffdmRecord	ffdm_rec;
struct inccRecord	incc_rec;
struct inwuRecord	inwu_rec;
struct inmrRecord	inmr_rec;
struct inumRecord	inum_rec;
struct pcglRecord	pcgl_rec;
struct pcmsRecord	pcms_rec;
struct pcwoRecord	pcwo_rec;
struct pcltRecord	pclt_rec;
struct esmrRecord	esmr_rec;
struct exwoRecord	exwo_rec;

#include	<MoveRec.h>

	char	*data	= "data";

	int		envQcApply 		= FALSE,
			envPcGenNum		= FALSE,
			envSkQcAvl		= FALSE,
			printerNumber	= 1;

	char 	debitAccount 	[MAXLEVEL + 1],	/* WIP D MATL	*/
			creditAccount 	[MAXLEVEL + 1],	/* COSTSALE M	*/
			mfgAccount 		[MAXLEVEL + 1],	/* MAN D MATL	*/
			writeOffAcc 	[MAXLEVEL + 1],	/* Stock write-off account.		 */
			localCurrency 	[4],
			altUomGroup 	[21],
			stdUomGroup 	[21],
			stdUom  		[5],
			altUom  		[5];

	float	stdCnvFct	=	0.00,
			altCnvFct	=	0.00,
			cnvFct		=	0.00;

	long	stdHhumHash	=	0L,
			altHhumHash	=	0L,
			debitHash	=	0L,
			creditHash	=	0L,
			mfgHash		=	0L,
			writeOff	=   0L,
			currentHhcc = 	0L;

	double	batchTotal	= 	0.00,
			qtyScrapped	=	0.00,
			skuCost		= 	0.00,
			stdCost		= 	0.00;


#include	<MoveAdd.h>

/*=====================
| function prototypes |
=====================*/
int		LotSelectFlag;
void 	GetAccounts 		(char *);
int 	Update 				(void);
void	ProcessInum 		(void);
void 	AddPcgl 			(double);
void 	AddPcglScrapped		(double);
void 	CloseAudit 			(void);
void 	CloseDB 			(void);
int		PRError 			(int);
int 	DoTransaction 		(float);
int		ProdMatlIssue		(long, int, long, long, float, char *);
int		StartProgram 		(void);
void 	OpenAudit 			(void);
void 	OpenDB 				(void);
void 	PrintDetails 		(float, double, char *, char *, char *,float);
void 	shutdown_prog 		(void);

#include	<LocHeader.h>

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int  argc, 
 char *argv [])
{
	long	hhwoHash	=	0L,
			mkuHash		=	0L,
			skuHash		=	0L;
	float	qtyIssued	=	0.0;
	int		lineID		=	0;
	char	lineComplete 	[2];

	if (scanf ("%d", &printerNumber) == EOF)
		return (EXIT_FAILURE);

	cc = StartProgram ();
	if (cc)
		return (cc);

	
	while (
		scanf ("%ld", &hhwoHash) 	!= EOF &&
		scanf ("%d",  &lineID) 		!= EOF &&
		scanf ("%ld", &mkuHash) 	!= EOF &&
		scanf ("%ld", &skuHash) 	!= EOF &&
		scanf ("%f",  &qtyIssued) 	!= EOF &&
		scanf ("%s",  lineComplete) != EOF )
	{
		ProdMatlIssue 
		(
			hhwoHash,
			lineID,
			mkuHash,
			skuHash,
			qtyIssued,
			lineComplete
		);
	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}


/*===========================================================================
| Start up program, Open database files, read default accounts, open audit. |
===========================================================================*/
int
StartProgram (void)
{
	char	*sptr;

	/*----------------------
	| Open database files. |
	----------------------*/
	OpenDB ();

	/*--------------------
	| Open Report Audit. |
	--------------------*/
	OpenAudit ();

	/*-------------------------------------------------------
	| Works order number is M(anually or S(ystem generated. |
	-------------------------------------------------------*/
	sptr = chk_env ("PC_GEN_NUM");
	if (sptr)
		envPcGenNum = (*sptr == 'M' || *sptr == 'm') ? FALSE : TRUE;
	else
		envPcGenNum = TRUE;

	/*-------------------------------------------------------
	| Include QC in available stock.                        |
	-------------------------------------------------------*/
	envSkQcAvl = (sptr = chk_env ("SK_QC_AVL")) ? atoi (sptr) : 0;
	envQcApply = (sptr = chk_env ("QC_APPLY"))  ? atoi (sptr) : 0;


	abc_selfield (glmr, "glmr_hhmr_hash");

	/*-------------------------------------
	| Production Receipts Write off code. |
	-------------------------------------*/
	sptr	=	chk_env ("PC_WOFF_CODE");
	if (sptr == (char *)0)
	{
		PRError (ERR_WOI_WOFFCODE);
		return (EXIT_FAILURE);
	}
	else
	{
		strcpy (exwo_rec.co_no, comm_rec.co_no);
		sprintf (exwo_rec.code, "%-2.2s", sptr);
		cc = find_rec (exwo, &exwo_rec, EQUAL, "r");
		if (cc)
		{
			PRError (ERR_WOI_WOFFCODE);
			return (EXIT_FAILURE);
		}
	
		glmrRec.hhmr_hash = exwo_rec.hhmr_hash;
		cc = find_rec (glmr, &glmrRec, EQUAL, "r");
		if (cc)
		{
			PRError (ERR_WOI_WOFFCODE);
			return (EXIT_FAILURE);
		}
		writeOff	=	glmrRec.hhmr_hash;
		strcpy (writeOffAcc, glmrRec.acc_no);
	}
	abc_selfield (glmr, "glmr_id_no");
	return (EXIT_SUCCESS);
}
/*=======================
| Program exit sequence	|
=======================*/
void
shutdown_prog (void)
{
	recalc_sobg ();
	CloseAudit ();
	CloseDB (); 
	FinishProgram ();
}

/*=======================
| Open data base files	|
=======================*/
void
OpenDB (void)
{
	MoveOpen	=	TRUE;

	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (comr,  comr_list, COMR_NO_FIELDS, "comr_co_no");
	open_rec (ffdm,  ffdm_list, FFDM_NO_FIELDS, "ffdm_id_no2");
	open_rec (ccmr,  ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	open_rec (incc,  incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (inmr,  inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (inum,  inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	open_rec (pcgl,  pcgl_list, PCGL_NO_FIELDS, "pcgl_id_no");
	open_rec (pcms,  pcms_list, PCMS_NO_FIELDS, "pcms_id_no");
	open_rec (pcwo,  pcwo_list, PCWO_NO_FIELDS, "pcwo_hhwo_hash");
	open_rec (pclt,  pclt_list, PCLT_NO_FIELDS, "pclt_hhwo_hash");
	open_rec (esmr,  esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (inwu,  inwu_list, INWU_NO_FIELDS, "inwu_id_no");
	open_rec (exwo,  exwo_list, EXWO_NO_FIELDS, "exwo_id_no");
	open_rec ("move",move_list, MOVE_NO_FIELDS, "move_move_hash");

	strcpy (ccmr_rec.co_no,  comm_rec.co_no);
	strcpy (ccmr_rec.est_no, comm_rec.est_no);
	strcpy (ccmr_rec.cc_no,  comm_rec.cc_no);
	cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "ccmr", "DBFIND");

	strcpy (comr_rec.co_no, comm_rec.co_no);
	cc = find_rec (comr, &comr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, comr, "DBFIND");

	if (!strcmp (comr_rec.base_curr, "   "))
		sprintf (localCurrency, "%-3.3s", get_env ("CURR_CODE"));
	else
		sprintf (localCurrency, "%-3.3s", comr_rec.base_curr);

	currentHhcc = ccmr_rec.hhcc_hash;

	OpenGlmr ();
	OpenLocation (ccmr_rec.hhcc_hash);
	strcpy (llctAutoAll, "Y");

	LotSelectFlag	=	INP_AUTO;
}

/*=======================
| Close data base files	|
=======================*/
void
CloseDB (
 void)
{
	abc_fclose (ccmr);
    abc_fclose (ffdm);
	abc_fclose (incc);
	abc_fclose (inmr);
	abc_fclose (inum);
	abc_fclose (pcgl);
	abc_fclose (pcms);
	abc_fclose (pcwo);
	abc_fclose (pclt);
	abc_fclose (esmr);
	abc_fclose (inwu);
	abc_fclose (exwo);
	abc_fclose ("move");
	CloseLocation ();
	CloseCosting ();
	GL_Close ();

	abc_dbclose (data);
}


/*=====================
| Update pcms records |
=====================*/
int
ProdMatlIssue (
	long	hhwoHash,
	int		lineID,
	long	mkuHash,
	long	skuHash,
	float	qtyIssued,
	char	*lineComplete)
{
	int		lineFoundHardWay	= FALSE;

	qtyScrapped	=	0.00;
	skuCost		= 	0.00,
	stdCost		= 	0.00;

	if (lineComplete [0] != 'N' && lineComplete [0] != 'Y')
		return (PRError (ERR_WOI_BAD_FLAG));

	/*-------------------
	| Find Works order. |
	-------------------*/
	pcwo_rec.hhwo_hash	=	hhwoHash;
	cc = find_rec (pcwo, &pcwo_rec, EQUAL, "u");
	if (cc)
	{
		abc_unlock (pcwo);
		return (PRError (ERR_WOI_NO_ORDER));
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

	/*--------------------------------
	| Find SKU line for works order. |
	--------------------------------*/
	inmr_rec.hhbr_hash	=	skuHash;
	cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
	if (cc)
		return (PRError (ERR_WOI_NO_SKU));

	if (inmr_rec.hhsi_hash)
	{
		inmr_rec.hhbr_hash	=	inmr_rec.hhsi_hash;
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		if (cc)
		{
			inmr_rec.hhbr_hash	=	skuHash;
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		}
	}
	if (cc)
		return (PRError (ERR_WOI_NO_SKU));

	/*--------------------------------
	| Find SKU line for works order. |
	--------------------------------*/
	inmr_rec.hhbr_hash	=	skuHash;
	cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
	if (cc)
		return (PRError (ERR_WOI_NO_SKU));

	/*----------------------------
	| Get accounts for category. |
	----------------------------*/
	GetAccounts (inmr_rec.category);
	
	/*-------------------
	| Find pcms records |
	-------------------*/
	lineFoundHardWay	=	FALSE;
	pcms_rec.hhwo_hash 	= pcwo_rec.hhwo_hash;
	pcms_rec.uniq_id 	= 0;
	cc = find_rec (pcms, &pcms_rec, GTEQ, "r");
	while (!cc && pcms_rec.hhwo_hash == pcwo_rec.hhwo_hash)
	{
		if (mkuHash == pcms_rec.hhbr_hash && skuHash == pcms_rec.mabr_hash)
		{
			lineFoundHardWay	=	TRUE;
			break;
		}
		cc = find_rec (pcms, &pcms_rec, NEXT, "r");
	}
	/*---------------------------------------------
	| No SKU found so it's all over for this line |
	---------------------------------------------*/
	if (lineFoundHardWay == FALSE)
		return (PRError (ERR_WOI_NO_LINE));
		
	cc = find_rec (pcms, &pcms_rec, CURRENT, "u");
	if (cc)
	{
		abc_unlock (pcms);
		return (PRError (ERR_WOI_NO_LINE));
	}
	
	/*-------------------------------------
	| Process bulk of stock transactions. |
	-------------------------------------*/
	cc = DoTransaction (qtyIssued);
	if (cc)
		return (PRError (cc));

	/*---------------------------------------
	| Update production issue quantity etc. |
	---------------------------------------*/
	if (qtyIssued > pcms_rec.matl_qty)
	{
		qtyScrapped	=	qtyIssued - pcms_rec.matl_qty;
		qtyIssued	=	pcms_rec.matl_qty;
	}
	pcms_rec.qty_issued		= 	qtyIssued;
	pcms_rec.amt_issued		= 	CENTS (qtyIssued * skuCost / (double) fourdec (cnvFct));
	pcms_rec.amt_issued		= 	out_cost 
								(
									pcms_rec.amt_issued,
									inmr_rec.outer_size
								);

	strcpy (pcms_rec.act_qty_in, lineComplete);

	pcms_rec.qty_scrap	= 	qtyScrapped;
	pcms_rec.amt_scrap	= 	CENTS (qtyScrapped * skuCost / (double) fourdec (cnvFct));

	cc = abc_update (pcms, &pcms_rec);
	if (cc)
		file_err (cc, "pcms", "DBUPDATE");

	/*----------------------------------------------
	| Add RC record to re-calculate committed etc. |
	----------------------------------------------*/
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

	/*---------------------------------
	| Add General Ledger Transaction. |
	---------------------------------*/
	if (qtyIssued != 0.00)
		AddPcgl ((double) qtyIssued);

	/*---------------------------------
	| Add General Ledger Transaction. |
	---------------------------------*/
	if (qtyScrapped != 0.00)
		AddPcglScrapped ((double) qtyScrapped);
	
	fprintf (pp, "|----------------|");
	fprintf (pp, "---------------------|");
	fprintf (pp, "-------------------------|");
	fprintf (pp, "-------|");
	fprintf (pp, "----------|");
	fprintf (pp, "-------------------|");
	fprintf (pp, "---------------|");
	fprintf (pp, "-------|");
	fprintf (pp, "----------------|");
	fprintf (pp, "-----------|\n");
	return (EXIT_SUCCESS);
}

/*=====================================
| Processess bulk of inventory stuff. |
=====================================*/
int
DoTransaction (
	float	qtyIssued)
{
	int		i;
	int		noLots		=	TRUE;

	/*-----------------------------------
	| Process conversion stuff for UOM. |
	-----------------------------------*/
	ProcessInum ();

	/*--------------------------------------------------
	| Find item/warehouse record for works order line. |
	--------------------------------------------------*/
	incc_rec.hhbr_hash	=	(inmr_rec.hhsi_hash) ? inmr_rec.hhsi_hash
												 : inmr_rec.hhbr_hash;
	incc_rec.hhcc_hash	=	ccmr_rec.hhcc_hash;
	cc = find_rec (incc, &incc_rec, COMPARISON, "r");
	if (cc)
		return (PRError (ERR_WOI_NO_INCC));

	if (incc_rec.closing_stock < (qtyIssued / cnvFct))
	{
		return (PRError (ERR_WOI_SOH));
	}
	/*-----------------------------------------------
	| Get costing information for works order line. |
	-----------------------------------------------*/
	switch (inmr_rec.costing_flag [0])
	{
	    case 'A':
	    case 'P':
	    case 'T':
	    case 'L':
			skuCost	=	FindIneiCosts 
						(
							inmr_rec.costing_flag,
							ccmr_rec.est_no,
							incc_rec.hhbr_hash
						);
		break;

	    case 'F':
			skuCost 	= 	FindIncfValue 
				 		  	(
								incc_rec.hhwh_hash, 
								incc_rec.closing_stock, 
								qtyIssued / fourdec (cnvFct),
								TRUE,
								inmr_rec.dec_pt
							);
		break;

	    case 'I':
			skuCost 	= 	FindIncfValue 
				 		  	(
								incc_rec.hhwh_hash, 
								incc_rec.closing_stock, 
								qtyIssued / fourdec (cnvFct),
								FALSE,
								inmr_rec.dec_pt
							);
		break;
	}
	if (skuCost < 0.00)
	{
		skuCost	=	FindIneiCosts 
					(
						"L",
						ccmr_rec.est_no,
						incc_rec.hhbr_hash
					);
	}
	stdCost	=	FindIneiCosts 
				(
					"T",
					ccmr_rec.est_no,
					incc_rec.hhbr_hash
				);

	/*----------------------------
	| Setup default lot loading. |
	----------------------------*/
	if (SK_BATCH_CONT || MULT_LOC)
	{
		cc =	DisplayLL
				(										
					pcms_rec.uniq_id,
					0,
					0,
					0,									
					incc_rec.hhwh_hash,
					stdHhumHash,
					incc_rec.hhcc_hash,
					stdUom, 
					qtyIssued,
					fourdec (cnvFct),
					TodaysDate (),
					LotSelectFlag,
					TRUE,
					inmr_rec.lot_ctrl
				);
		/*-----------------------------------------------------
		| Lot allocation error, most likely not enough stock. |
		-----------------------------------------------------*/
		if (cc)
			return (PRError (ERR_WOI_LOTALLOC));
	}

	/*----------------------------
	| Process lots and locations |
	----------------------------*/
	noLots = TRUE;
	if (!SERIAL && !NONSTOCK)
	{
		noLots	=	TRUE;
		for (i = 0; i < MAX_LOTS; i++)
		{
			if (!LL_Valid (pcms_rec.uniq_id, i))
				break;

			if (GetBaseQty (pcms_rec.uniq_id, i) == 0.00)
				continue;

			noLots	=	FALSE;
			/*--------------------------
			| Log inventory movements. |
			--------------------------*/
			MoveAdd
			(
				ccmr_rec.co_no, 
				ccmr_rec.est_no, 
				ccmr_rec.cc_no, 
				incc_rec.hhbr_hash, 
				incc_rec.hhcc_hash,
				stdHhumHash,
				TodaysDate (),
				8, 
				GetLotNo (pcms_rec.uniq_id, i),
				inmr_rec.inmr_class, 
				inmr_rec.category, 
				pcwo_rec.order_no, 
				pcwo_rec.batch_no, 
				GetBaseQty (pcms_rec.uniq_id, i),
				0.00, 
				CENTS (skuCost)
			);
			/*---------------------
			| Print an Audit line |
			---------------------*/
			PrintDetails
		 	(
				GetQty (pcms_rec.uniq_id, i),
				skuCost, 
				GetLotNo (pcms_rec.uniq_id, i),
				" ", 
				GetLoc (pcms_rec.uniq_id, i),
				fourdec (cnvFct)
			);

			strcpy (pclt_rec.co_no, pcwo_rec.co_no);
			strcpy (pclt_rec.br_no, pcwo_rec.br_no);
			pclt_rec.hhwo_hash 	= pcwo_rec.hhwo_hash;
			pclt_rec.hhbr_hash 	= inmr_rec.hhbr_hash;
			pclt_rec.issue_date 	= TodaysDate ();
			pclt_rec.qty_used 	= GetBaseQty (pcms_rec.uniq_id, i);
			pclt_rec.iss_uom 	= stdHhumHash;

			strcpy (pclt_rec.lot_number, 	GetLotNo	 (pcms_rec.uniq_id, i));
			strcpy (pclt_rec.slot_no, 	 	GetSLotNo	 (pcms_rec.uniq_id, i));
			strcpy (pclt_rec.lot_location, 	GetLoc 		 (pcms_rec.uniq_id,i));

			/*----------------------------
			| Add W/O lots trace record. |
			----------------------------*/
			cc = abc_add (pclt, &pclt_rec);
			if (cc)
				file_err (cc, "pclt", "DBADD");
		}
		if (noLots == TRUE)
		{
			/*--------------------------
			| Log inventory movements. |
			--------------------------*/
			MoveAdd
			(
				ccmr_rec.co_no, 
				ccmr_rec.est_no, 
				ccmr_rec.cc_no, 
				incc_rec.hhbr_hash, 
				incc_rec.hhcc_hash,
				stdHhumHash,
				TodaysDate (),
				8, 
			    "      ", 
				inmr_rec.inmr_class, 
				inmr_rec.category, 
				pcwo_rec.order_no, 
			    pcwo_rec.batch_no, 
				 (float) (qtyIssued / fourdec (cnvFct)),
				0.00, 
			    CENTS (skuCost)
			);
			/*---------------------
			| Print an Audit line |
			---------------------*/
			PrintDetails
			(
				(float) qtyIssued, 
				skuCost,
				" ", 
				" ", 
				" ",
				fourdec (cnvFct)
			);
		}
	}
	if (!NONSTOCK)
	{
		incc_rec.issues			+= 	(float) (qtyIssued / fourdec (cnvFct));
		incc_rec.ytd_issues		+= 	(float) (qtyIssued / fourdec (cnvFct));
		incc_rec.closing_stock 	= 	incc_rec.opening_stock	+
									incc_rec.pur				+
									incc_rec.receipts		+
									incc_rec.adj				-
									incc_rec.issues			-
									incc_rec.sales;

		cc = abc_update (incc, &incc_rec);
		if (cc)
			file_err (cc, incc, "DBUPDATE");

		/*--------------------------------------
		| Find Warehouse unit of measure file. |
		--------------------------------------*/
		inwu_rec.hhwh_hash	=	incc_rec.hhwh_hash;
		inwu_rec.hhum_hash	=	inmr_rec.std_uom;
		cc = find_rec (inwu, &inwu_rec, COMPARISON,"u");
		if (cc)
		{
			memset (&inwu_rec, 0, sizeof (inwu_rec));
			inwu_rec.hhwh_hash	=	incc_rec.hhwh_hash;
			inwu_rec.hhum_hash	=	inmr_rec.std_uom;
			cc = abc_add (inwu, &inwu_rec);
			if (cc)
				file_err (cc, inwu, "DBADD");

			inwu_rec.hhwh_hash	=	incc_rec.hhwh_hash;
			inwu_rec.hhum_hash	=	inmr_rec.std_uom;
			cc = find_rec (inwu, &inwu_rec, COMPARISON,"u");
			if (cc)
				file_err (cc, inwu, "DBFIND");
		}
		inwu_rec.issues			+= 	(float) (qtyIssued / fourdec (cnvFct));
		inwu_rec.closing_stock 	= 	inwu_rec.opening_stock 	+
								 	inwu_rec.pur 			+
								 	inwu_rec.receipts 		+
								 	inwu_rec.adj 			-
								 	inwu_rec.issues 			-
								 	inwu_rec.sales;

		cc = abc_update (inwu,&inwu_rec);
		if (cc)
			file_err (cc, inwu, "DBUPDATE");
	}
	else
		abc_unlock (incc);

	if ((MULT_LOC || SK_BATCH_CONT) && !NONSTOCK)
		UpdateLotLocation (pcms_rec.uniq_id, TRUE);
	
	if (!NONSTOCK)
	{
		ffdm_rec.hhbr_hash	=	incc_rec.hhbr_hash;
		ffdm_rec.hhcc_hash	=	incc_rec.hhcc_hash;
		strcpy (ffdm_rec.type, "6");
		ffdm_rec.date		=	TodaysDate ();
		cc = find_rec (ffdm, &ffdm_rec, COMPARISON, "r");
		if (cc)
		{
			ffdm_rec.qty	=	(float) (qtyIssued / fourdec (cnvFct));
			cc = abc_add (ffdm, &ffdm_rec);
			if (cc)
				file_err (cc, ffdm, "DBADD");
		}
		else
		{
			ffdm_rec.qty	+=	(float) (qtyIssued / fourdec (cnvFct));
			cc = abc_update (ffdm, &ffdm_rec);
			if (cc)
				file_err (cc, ffdm, "DBUPDATE");
		}
		inmr_rec.on_hand -= (float) (qtyIssued / fourdec (cnvFct));
		cc = abc_update (inmr, &inmr_rec);
		if (cc)
			file_err (cc, inmr, "DBUPDATE");

	}
	else
		abc_unlock (inmr);

	return (EXIT_SUCCESS);
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

/*===========================
| Process control Accounts. |
===========================*/
void
GetAccounts (
	char	*categoryNumber)
{
	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"COSTSALE M",
		" ",
		categoryNumber
	);
	strcpy (creditAccount, glmrRec.acc_no);
	creditHash = glmrRec.hhmr_hash;

	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"WIP D MATL",
		" ",
		categoryNumber
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
		categoryNumber
	);
	mfgHash = glmrRec.hhmr_hash;
	strcpy (mfgAccount, glmrRec.acc_no);

}

/*================================
| Add transactions to pcgl file. |
================================*/
void
AddPcgl (
	double	qtyIssued)
{
	int		periodMonth;
	double	costDiff;
	char	type [2];

	strcpy 	(pcgl_rec.co_no, ccmr_rec.co_no);
	strcpy 	(pcgl_rec.tran_type, "19");
	sprintf (pcgl_rec.sys_ref, "%5.1d", 1);
	pcgl_rec.tran_date 	= 	TodaysDate ();
	pcgl_rec.post_date 	= 	TodaysDate ();
	DateToDMY (pcgl_rec.tran_date, NULL, 	&periodMonth, NULL);
	sprintf (pcgl_rec.period_no, "%02d", 	periodMonth);
	sprintf (pcgl_rec.narrative, "%-20.20s", pcwo_rec.order_no);
	sprintf (pcgl_rec.user_ref, "%8.8s", 	 "AUTO ISS");
	strcpy 	(pcgl_rec.stat_flag, "2");

	/*-------------------------------------------------------
	| Post the std cost to the WIP Direct Material Account. |
	-------------------------------------------------------*/
	strcpy (pcgl_rec.acc_no, debitAccount);
	pcgl_rec.hhgl_hash 	= debitHash;
	pcgl_rec.amount		=	CENTS (qtyIssued * stdCost / (double) fourdec (cnvFct));
	pcgl_rec.amount 	= 	out_cost (pcgl_rec.amount, inmr_rec.outer_size);
	strcpy (pcgl_rec.jnl_type, "1");
	pcgl_rec.loc_amount	=	pcgl_rec.amount;
	pcgl_rec.exch_rate	=	1.00;
	strcpy (pcgl_rec.currency, localCurrency);

	if (pcgl_rec.amount != 0.00)
	{
		cc = abc_add (pcgl, &pcgl_rec);
		if (cc)
			file_err (cc, "pcgl", "DBADD");
	}

	/*-------------------------------------------------------
	| Post the act cost to the components stock account.    |
	-------------------------------------------------------*/
	strcpy (pcgl_rec.acc_no, creditAccount);
	pcgl_rec.hhgl_hash 	=	creditHash;
	pcgl_rec.amount		=	CENTS (qtyIssued * skuCost / (double) fourdec (cnvFct));
	pcgl_rec.amount 	= 	out_cost (pcgl_rec.amount, inmr_rec.outer_size);
	strcpy (pcgl_rec.jnl_type, "2");
	pcgl_rec.loc_amount	=	pcgl_rec.amount;
	pcgl_rec.exch_rate	=	1.00;
	strcpy (pcgl_rec.currency, localCurrency);

	if (pcgl_rec.amount != 0.00)
	{
		cc = abc_add (pcgl, &pcgl_rec);
		if (cc)
			file_err (cc, "pcgl", "DBADD");
	}

	/*-------------------------------------------------------
	| Post the act cost to the components stock account.    |
	| post the difference between the act cost and the std  |
	| cost to the MFG Variance Direct Material account.     |
	-------------------------------------------------------*/
	strcpy (pcgl_rec.acc_no, mfgAccount);
	pcgl_rec.hhgl_hash 	= 	mfgHash;
	costDiff = skuCost - stdCost;
	if (costDiff == 0.00) /* No record needed if variance is zero. */
		return;

	if (costDiff > 0.00)
		strcpy (type, "1");
	else
	{
		costDiff = 0 - costDiff;
		strcpy (type, "2");
	}

	pcgl_rec.amount = CENTS (pcms_rec.qty_issued * costDiff / (double) fourdec (cnvFct));
	pcgl_rec.amount = out_cost (pcgl_rec.amount, inmr_rec.outer_size);
	strcpy (pcgl_rec.jnl_type, type);

	pcgl_rec.loc_amount	=	pcgl_rec.amount;
	pcgl_rec.exch_rate	=	1.00;
	strcpy (pcgl_rec.currency, localCurrency);

	if (pcgl_rec.amount != 0.00)
	{
		cc = abc_add (pcgl, &pcgl_rec);
		if (cc)
			file_err (cc, "pcgl", "DBADD");
	}
}
/*================================
| Add transactions to pcgl file. |
================================*/
void
AddPcglScrapped(
	double	qtyScrapped)
{
	int		periodMonth;
	char	type [2];
	double	costDiff	=	0.00;

	strcpy 	(pcgl_rec.co_no, ccmr_rec.co_no);
	strcpy 	(pcgl_rec.tran_type, "19");
	sprintf (pcgl_rec.sys_ref, "%5.1d", 1);
	pcgl_rec.tran_date 	= 	TodaysDate ();
	pcgl_rec.post_date 	= 	TodaysDate ();
	DateToDMY (pcgl_rec.tran_date, NULL, 	&periodMonth, NULL);
	sprintf (pcgl_rec.period_no, "%02d", 	periodMonth);
	sprintf (pcgl_rec.narrative, "%-20.20s", pcwo_rec.order_no);
	sprintf (pcgl_rec.user_ref, "%8.8s", 	 "AUTO ISS");
	strcpy 	(pcgl_rec.stat_flag, "2");

	/*----------------------------------------
	| Post Debit to Scrap account at actual. |
	----------------------------------------*/
	strcpy (pcgl_rec.acc_no, writeOffAcc);
	pcgl_rec.hhgl_hash 	= 	writeOff;
	pcgl_rec.amount		=	CENTS (qtyScrapped * skuCost / (double) fourdec (cnvFct));
	pcgl_rec.amount 	= 	out_cost (pcgl_rec.amount, inmr_rec.outer_size);
	strcpy (pcgl_rec.jnl_type, "1");
	pcgl_rec.loc_amount	=	pcgl_rec.amount;
	pcgl_rec.exch_rate	=	1.00;
	strcpy (pcgl_rec.currency, localCurrency);

	if (pcgl_rec.amount != 0.00)
	{
		cc = abc_add (pcgl, &pcgl_rec);
		if (cc)
			file_err (cc, "pcgl", "DBADD");
	}

	/*-----------------------------------
	| Post standard cost to MAN D MATL. |
	-----------------------------------*/
	strcpy (pcgl_rec.acc_no, debitAccount);
	pcgl_rec.hhgl_hash 	= 	debitHash;
	pcgl_rec.amount		=	CENTS (qtyScrapped * stdCost / (double) fourdec (cnvFct));
	pcgl_rec.amount 	= 	out_cost (pcgl_rec.amount, inmr_rec.outer_size);
	strcpy (pcgl_rec.jnl_type, "2");

	pcgl_rec.loc_amount	=	pcgl_rec.amount;
	pcgl_rec.exch_rate	=	1.00;
	strcpy (pcgl_rec.currency, localCurrency);

	if (pcgl_rec.amount != 0.00)
	{
		cc = abc_add (pcgl, &pcgl_rec);
		if (cc)
			file_err (cc, "pcgl", "DBADD");
	}

	/*-------------------------------------------------------
	| Post the act cost to the components stock account.    |
	| post the difference between the act cost and the std  |
	| cost to the MFG Variance Direct Material account.     |
	-------------------------------------------------------*/
	strcpy (pcgl_rec.acc_no, mfgAccount);
	pcgl_rec.hhgl_hash 		= 	mfgHash;
	costDiff 				= skuCost - stdCost;
	if (costDiff == 0.00) /* No record needed if variance is zero. */
		return;

	if (costDiff > 0.00)
		strcpy (type, "2");
	else
	{
		costDiff = 0 - costDiff;
		strcpy (type, "1");
	}

	pcgl_rec.amount = CENTS (qtyScrapped * costDiff / (double) fourdec(cnvFct));
	pcgl_rec.amount = out_cost (pcgl_rec.amount, inmr_rec.outer_size);
	strcpy (pcgl_rec.jnl_type, type);

	pcgl_rec.loc_amount	=	pcgl_rec.amount;
	pcgl_rec.exch_rate	=	1.00;
	strcpy (pcgl_rec.currency, localCurrency);

	if (pcgl_rec.amount != 0.00)
	{
		cc = abc_add (pcgl, &pcgl_rec);
		if (cc)
			file_err (cc, "pcgl", "DBADD");
	}
}

/*=========================================================================
| Routine to open output pipe to standard print to provide an Audit trail |
| of events. This also sends the output straight to the spooler.          |
=========================================================================*/
void
OpenAudit (
 void)
{

	if ((pp = popen ("pformat", "w")) == 0) 
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);

	fprintf (pp, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (pp, ".SO\n");
	fprintf (pp, ".LP%d\n", printerNumber);
	fprintf (pp, ".12\n");
	fprintf (pp, ".L158\n");
	fprintf (pp, ".E%s\n", ML ("AUTOMATIC STOCK ISSUES FOR PRODUCTION"));
	fprintf (pp, ".B1\n");
	fprintf (pp, ".E%s as at %24.24s\n", clip (comm_rec.co_short), SystemTime ());

	fprintf (pp, ".B2\n");
	fprintf (pp, ".EBRANCH: %s WAREHOUSE: %s %s\n", 
										ccmr_rec.est_no, ccmr_rec.cc_no,
										clip (ccmr_rec.name));

	fprintf (pp, ".R==================");
	fprintf (pp, "======================");
	fprintf (pp, "==========================");
	fprintf (pp, "========");
	fprintf (pp, "=================");
	fprintf (pp, "===================");
	fprintf (pp, "===========");
	fprintf (pp, "========");
	fprintf (pp, "=================");
	fprintf (pp, "============\n");

	fprintf (pp, "==================");
	fprintf (pp, "======================");
	fprintf (pp, "==========================");
	fprintf (pp, "========");
	fprintf (pp, "=================");
	fprintf (pp, "===================");
	fprintf (pp, "===========");
	fprintf (pp, "========");
	fprintf (pp, "=================");
	fprintf (pp, "============\n");

	fprintf (pp, "|  ITEM NUMBER   |");
	fprintf (pp, "  ITEM DESCRIPTION   |");
	fprintf (pp, "       SERIAL NUMBER     |");
	fprintf (pp, "  LOT  |");
	fprintf (pp, " LOCATION |");
	fprintf (pp, "      QUANTITY     |");
	fprintf (pp, "     @COST     |");
	fprintf (pp, " OUTER |");
	fprintf (pp, " GENERAL LEDGER |");
	fprintf (pp, "  EXTENDED |\n");

	fprintf (pp, "|                |");
	fprintf (pp, "                     |");
	fprintf (pp, "                         |");
	fprintf (pp, "  NO.  |");
	fprintf (pp, "          |");
	fprintf (pp, "       ISSUED      |");
	fprintf (pp, "               |");
	fprintf (pp, "  SIZE |");
	fprintf (pp, "     ACCOUNT    |");
	fprintf (pp, "  VALUE.   |\n");

	fprintf (pp, "|----------------|");
	fprintf (pp, "---------------------|");
	fprintf (pp, "-------------------------|");
	fprintf (pp, "-------|");
	fprintf (pp, "----------|");
	fprintf (pp, "-------------------|");
	fprintf (pp, "---------------|");
	fprintf (pp, "-------|");
	fprintf (pp, "----------------|");
	fprintf (pp, "-----------|\n");

	fprintf (pp, ".PI12\n");

	batchTotal = 0.00;
}

/*==============================
| Print details of data input. |
==============================*/
void
PrintDetails (
	float	qtyIssued,
	double	cost,
	char	*lotNumber,
	char	*serialNumber,
	char	*location,
	float	cfactor)
{
	double	value = out_cost (cost, inmr_rec.outer_size);

	/*----------------
	| Printe Details |
	----------------*/
	fprintf (pp, "|%16.16s|",	inmr_rec.item_no);
	fprintf (pp, "%-21.21s|",	inmr_rec.description);
	fprintf (pp, "%25.25s|",	" ");
	fprintf (pp, "%7.7s|",  	lotNumber);
	fprintf (pp, "%-10.10s|",	location);
	fprintf (pp, "%14.6f/", 	qtyIssued);
	fprintf (pp, "%-4.4s|", 	stdUom);
	fprintf (pp, "%10.2f/", 	cost);
	fprintf (pp, "%-4.4s|", 	stdUom);
	fprintf (pp, "%7.1f|", 		inmr_rec.outer_size);

	fprintf (pp, "%-16.16s|", creditAccount);
	fprintf (pp, "%11.2f|\n", (qtyIssued * value) / cfactor);
	fflush (pp);

	batchTotal += (double) ((qtyIssued * value) / cfactor);
}

/*===============================================
| Routine to close the audit trail output file. |
===============================================*/
void
CloseAudit (
 void)
{
	fprintf (pp, "| BATCH TOTAL    |");
	fprintf (pp, "                     |");
	fprintf (pp, "                         |");
	fprintf (pp, "       |");
	fprintf (pp, "          |");
	fprintf (pp, "                   |");
	fprintf (pp, "               |");
	fprintf (pp, "       |");
	fprintf (pp, "                |");
	fprintf (pp, "%11.2f|\n", batchTotal);
	fprintf (pp, ".EOF\n");
	pclose (pp);
}

/*================
| Process Errors |
================*/
int
PRError (
	int	errCode)
{
	char	errMess [3][81];

	if (ERR_WOI_BAD_FLAG == errCode)
	{
		strcpy (errMess [0], "Close line flag can only be 'N' or 'Y'");
		fprintf (stderr, "%s\n", errMess [0]);
		XML_Error 
		(
			"pc_autoiss",
			errMess [0],
			" ",
			" ",
			" "
		);
		return (errCode);
	}
	if (ERR_WOI_NO_ORDER == errCode)
	{
		sprintf (errMess [0], "Works order hash (pcwo_hhwo_hash) not valid");
		sprintf (errMess [1], "pcwo_hhwo_hash = [%ld]", pcwo_rec.hhwo_hash);
		fprintf (stderr, "%s\n", errMess [0]);
		fprintf (stderr, "%s\n", errMess [1]);
		XML_Error 
		(
			"pc_autoiss",
			errMess [0],
			errMess [1],
			" ",
			" "
		);
		return (errCode);
	}
	if (ERR_WOI_NO_LINE == errCode)
	{
		sprintf (errMess [0],"Works order line (pcms) could not be found");
		sprintf (errMess [1],"pcms_hhwo_hash = [%ld] pcms_uniq_id [%d]", pcms_rec.hhwo_hash, pcms_rec.uniq_id);
		fprintf (stderr, "%s\n", errMess [0]);
		fprintf (stderr, "%s\n", errMess [1]);
		XML_Error 
		(
			"pc_autoiss",
			errMess [0],
			errMess [1],
			" ",
			" "
		);
		return (errCode);
	}
	if (ERR_WOI_NO_MKU == errCode)
	{
		sprintf (errMess [0], "MKU could not be found for works order");
		sprintf (errMess [1], "inmr_hhbr_hash = [%ld]", inmr_rec.hhbr_hash);
		sprintf (errMess [2], "pcms_hhwo_hash = [%ld]", pcwo_rec.hhwo_hash);
		fprintf (stderr, "%s\n", errMess [0]);
		fprintf (stderr, "%s\n", errMess [1]);
		fprintf (stderr, "%s\n", errMess [2]);
		XML_Error 
		(
			"pc_autoiss",
			errMess [0],
			errMess [1],
			errMess [2],
			" "
		);
		return (errCode);
	}
	if (ERR_WOI_NO_SKU == errCode)
	{
		sprintf (errMess [0], "SKU could not be found for works order");
		sprintf (errMess [1],  "inmr_hhbr_hash = [%ld]", inmr_rec.hhbr_hash);
		sprintf (errMess [2],  "pcms_hhwo_hash = [%ld]", pcwo_rec.hhwo_hash);
		fprintf (stderr, "%s\n", errMess [0]);
		fprintf (stderr, "%s\n", errMess [1]);
		fprintf (stderr, "%s\n", errMess [2]);
		XML_Error 
		(
			"pc_autoiss",
			errMess [0],
			errMess [1],
			errMess [2],
			" "
		);
		return (errCode);
	}
	if (ERR_WOI_NO_CCMR == errCode)
	{
		sprintf(errMess [0], "Warehouse record could not be found (ccmr)");
		sprintf(errMess [0], "ccmr_co_no [%s] ccmr_est_no [%s] ccmr_cc_no [%s]",
							ccmr_rec.co_no, ccmr_rec.est_no, ccmr_rec.cc_no);
		fprintf (stderr, "%s\n", errMess [0]);
		fprintf (stderr, "%s\n", errMess [1]);
		XML_Error 
		(
			"pc_autoiss",
			errMess [0],
			errMess [1],
			" ",
			" "
		);
		return (errCode);
	}
	if (ERR_WOI_NO_INCC == errCode)
	{
		sprintf (errMess [0], "Warehouse Item record not found (incc)");
		sprintf (errMess [1], "incc_hhbr_hash [%ld] incc_hhcc_hash [%ld]",
							incc_rec.hhbr_hash, incc_rec.hhcc_hash);
		fprintf (stderr, "%s\n", errMess [0]);
		fprintf (stderr, "%s\n", errMess [1]);
		XML_Error 
		(
			"pc_autoiss",
			errMess [0],
			errMess [1],
			" ",
			" "
		);
		return (errCode);
	}
	if (ERR_WOI_LOTALLOC == errCode)
	{
		sprintf (errMess [0], "Not enough SOH at location for allocation");
		sprintf (errMess [1], "pcms_hhwo_hash = [%ld] pcms_uniq_id [%d]", pcms_rec.hhwo_hash, pcms_rec.uniq_id);
		sprintf (errMess [2], "incc_hhbr_hash [%ld] incc_hhcc_hash [%ld]",
							incc_rec.hhbr_hash, incc_rec.hhcc_hash);
		fprintf (stderr, "%s\n", errMess [0]);
		fprintf (stderr, "%s\n", errMess [1]);
		fprintf (stderr, "%s\n", errMess [2]);
		XML_Error 
		(
			"pc_autoiss",
			errMess [0],
			errMess [1],
			errMess [2],
			" "
		);
		return (errCode);
	}
	if (ERR_WOI_SOH == errCode)
	{
		sprintf (errMess [0], "Not enough SOH at issue warehouse\n");
		fprintf (stderr, "%s\n", errMess [0]);
		XML_Error 
		(
			"pc_autoiss",
			errMess [0],
			" ",
			" ",
			" "
		);
		return (errCode);
	}
	if (ERR_WOI_WOFFCODE == errCode)
	{
		sprintf (errMess [0], "No write-off code found or defined (exwo)");
		fprintf (stderr, "%s\n", errMess [0]);
		XML_Error 
		(
			"pc_autoiss",
			errMess [0],
			" ",
			" ",
			" "
		);
		return (errCode);
	}
	return (errCode);
}

/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: pc_autorec.c,v 5.10 2001/08/09 09:14:30 scott Exp $
|  Program Name  : (pc_autorec.c)
|  Program Desc  : (Automatic Production Control Receipts)
|---------------------------------------------------------------------|
|  Author        : Scott B Darrow  | Date Written : 30th March 2001   |
|---------------------------------------------------------------------|
| $Log: pc_autorec.c,v $
| Revision 5.10  2001/08/09 09:14:30  scott
| Updated to add FinishProgram () function
|
| Revision 5.9  2001/08/06 23:34:56  scott
| RELEASE 5.0
|
| Revision 5.8  2001/07/25 02:18:21  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: pc_autorec.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PC/pc_autorec/pc_autorec.c,v 5.10 2001/08/09 09:14:30 scott Exp $";

#include	<pslscr.h>
#include	<pc_autorec.h>
#include	<GlUtils.h>
#include	<twodec.h>
#include	<proc_sobg.h>
#include	<number.h>
#include	<ml_std_mess.h>
#include	<ml_pc_mess.h>
#include	<XML_Error.h>
#include	<Costing.h>
#define		NDEC(x)		n_dec (x, inmr_rec.dec_pt)

#include	"schema"

struct comrRecord	comr_rec;
struct ccmrRecord	ccmr_rec;
struct commRecord	comm_rec;
struct exwoRecord	exwo_rec;
struct inccRecord	incc_rec;
struct inwuRecord	inwu_rec;
struct inmrRecord	inmr_rec;
struct inmrRecord	inmr2_rec;
struct inumRecord	inum_rec;
struct pcglRecord	pcgl_rec;
struct pclnRecord	pcln_rec;
struct pcmsRecord	pcms_rec;
struct pcrqRecord	pcrq_rec;
struct pcwcRecord	pcwc_rec;
struct pcwoRecord	pcwo_rec;
struct rgrsRecord	rgrs_rec;
struct pcatRecord	pcat_rec;
struct esmrRecord	esmr_rec;
struct pchsRecord	pchs_rec;
struct qcmrRecord	qcmr_rec;
struct qchrRecord	qchr_rec;
struct sohrRecord	sohr_rec;
struct solnRecord	soln_rec;

INEI_STRUCT	inei2Rec;

	int		*inei_expiry_prd	=	&ineiRec.expiry_prd1;

#include	<MoveRec.h>

	char	*data	= 	"data",
			*inmr2	= 	"inmr2",
			*DBFIND	=	"DBFIND";

#define	QCITEM		 (inmr_rec.qc_reqd [0] == 'Y')
#define	LOT_CTRL	 (inmr_rec.lot_ctrl [0] == 'Y')
#define	JOB_CLOSING	 (pcwo_rec.order_status [0] == 'C')

	/*----------------------------------------------------------------
	| Special fields and flags  ################################## . |
	----------------------------------------------------------------*/
	FILE	*pp;
	int		popeOpen = FALSE;

struct	{
	char	name [10];
	int		noDays;
} mnths [] = {
	{"January",   31},
	{"February",  28},
	{"March",     31},
	{"April",     30},
	{"May",       31},
	{"June",      30},
	{"July",      31},
	{"August",    31},
	{"September", 30},
	{"October",   31},
	{"November",  30},
	{"December",  31},
	{"", 0}
};

	char 	altUomGroup 	[21],
			stdUomGroup 	[21],
			stdUom  		[5],
			altUom  		[5];

	float	stdCnvFct	=	0.00,
			altCnvFct	=	0.00,
			cnvFct		=	0.00;

	long	stdHhumHash	=	0L,
			altHhumHash	=	0L;

	char	localCurrency [4];

	double	batchTotal 		= 0.00,
			quantityTotal 	= 0.00,
			thisReceipt		= 0.00,
			prevReceipt		= 0.00;

	int		printerNumber 		= 1,
			envVarQcApply 		= FALSE,
			envVarPcGenNum  	= 0,
			envVarConOrders 	= 0,
			envVarSoWoAllowed 	= 0,
			periodMonth			= 0;

	char	envVarPcWoffCode	[3];

	/*--------------------------------------------
	| General Ledger hashes and account numbers. |
	--------------------------------------------*/
	long	matHash 	[2],				/* 0 - direct	1 - mfg variance */
			dirHash 	[5],				/* 0 - labour   1 - machine  	 */
			fixHash 	[5],				/* 2 - qc-check 3 - special  	 */
			mfgDHash 	[5],				/* 4 - other    */
			mfgFHash 	[5],
			writeOff;						/* Stock Write-off set to PR	 */
	char	matAcc 		[2] [MAXLEVEL + 1],	/* 0 - direct	1 - mfg variance */
			dirAcc 		[5] [MAXLEVEL + 1],	/* 0 - labour   1 - machine  	 */
			fixAcc 		[5] [MAXLEVEL + 1],	/* 2 - qc-check 3 - special  	 */
			mfgDAcc 	[5] [MAXLEVEL + 1],	/* 4 - other    	      	 	 */
			mfgFAcc 	[5] [MAXLEVEL + 1],
			writeOffAcc [MAXLEVEL + 1];		/* Stock write-off account.		 */

	long	expiryDate = 0L;

struct {
	double	matCost;			/* Material Costs		 */
	double	dLabour;			/* Direct Labour Costs	 */
	double	dMachine;			/* Direct Machine Costs	 */
	double	dQcCheck;			/* Direct QC-Check Costs */
	double	dSpecial;			/* Direct Special Costs	 */
	double	dOther;				/* Direct Other Costs	 */
	double	fLabour;			/* Fixed Labour Costs	 */
	double	fMachine;			/* Fixed Machine Costs	 */
	double	fQcCheck;			/* Fixed QC-Check Costs	 */
	double	fSpecial;			/* Fixed Special Costs	 */
	double	fOther;				/* Fixed Other Costs	 */
} stdRec, actRec, recRec, thisRec;


float	qtyReceipt;
float	qtyReject;
Date	receiptDate;
double	standardCost;
double	costPerUnit;
double	actCost;
float	outerSize;
char	creditAccount [MAXLEVEL + 1];
long	creditHash;


#include	<LocHeader.h>
#include	<MoveAdd.h>

/*=====================
| function prototypes |
=====================*/
double 	UpdatePcln 			 (int);
double 	UpdatePcms 			 (int);
float 	GetUom 				 (long, long);
void 	GetAccount 			 (char *);
void	ProcessInum 		 (void);
int		PRError 			 (int);
int 	ReadDefault 		 (char *);
int 	Update 				 (void);
void 	AddPcgl 			 (long, char *, char *, double, char *, char *);
void 	CalcCost 			 (void);
void 	CloseAudit 			 (void);
void 	CloseDB 			 (void);
void 	OpenAudit 			 (void);
void 	OpenDB 				 (void);
void 	PrintDetails 		 (int);
void	ProcessSalesOrder 	 (long, char *, float);
void 	UpdateFifo 			 (long, double, double);
void 	UpdateQchr 			 (float, long);
void 	shutdown_prog 		 (void);
int		StartProgram 		 (void);
int		ProcReceipt 		 (long, float, char *);

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int  argc, 
 char *argv [])
{
	long	hhwoHash	=	0L;
	char	processSO 	    [2];

	if (scanf ("%d", &printerNumber) == EOF)
		return (EXIT_FAILURE);

	if (StartProgram ())
		return (EXIT_FAILURE);

	while 
	( 
		scanf ("%ld", &hhwoHash) 	!= EOF &&
		scanf ("%f",  &qtyReceipt) 	!= EOF &&
		scanf ("%s",  processSO) 	!= EOF
	)
	{
		ProcReceipt
		 (
			hhwoHash,
			qtyReceipt, 
			processSO
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

	/*--------------------------------
	| Check Consolidation of orders. |
	--------------------------------*/
	envVarSoWoAllowed = (sptr = chk_env ("SO_WO_ALLOWED")) ? atoi (sptr) : 0;

	/*----------------------
	| Open database files. |
	----------------------*/
	OpenDB ();

	/*----------------------------
	| Setup General Ledger Mask. |
	----------------------------*/
	GL_SetMask ("XXXXXXXXXXXXXXXX");

	/*-------------------
	| Check QC applies. |
	-------------------*/
	envVarQcApply = (sptr = chk_env ("QC_APPLY")) ? atoi (sptr) : 0;

	abc_selfield (glmr, "glmr_hhmr_hash");

	/*-------------------------------------
	| Production Receipts Write off code. |
	-------------------------------------*/
	sptr	=	chk_env ("PC_WOFF_CODE");
	if (sptr == (char *)0)
	{
		PRError (ERR_WOR_WOFFCODE);
		return (EXIT_FAILURE);
	}
	else
	{
		strcpy (exwo_rec.co_no, comm_rec.co_no);
		sprintf (exwo_rec.code, "%-2.2s", sptr);
		cc = find_rec (exwo, &exwo_rec, EQUAL, "r");
		if (cc)
		{
			PRError (ERR_WOR_WOFFCODE);
			return (EXIT_FAILURE);
		}
	
		glmrRec.hhmr_hash = exwo_rec.hhmr_hash;
		cc = find_rec (glmr, &glmrRec, EQUAL, "r");
		if (cc)
		{
			PRError (ERR_WOR_WOFFCODE);
			return (EXIT_FAILURE);
		}
		writeOff	=	glmrRec.hhmr_hash;
		strcpy (writeOffAcc, glmrRec.acc_no);
	}
	abc_selfield (glmr, "glmr_id_no");

	/*------------------------------
	| Check if Works Order Allowed |
	------------------------------*/
	envVarConOrders = (sptr = chk_env ("CON_ORDERS")) ? atoi (sptr) : 0;

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
	if (popeOpen)
		CloseAudit ();
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
	MoveOpen	=	TRUE;

	abc_dbopen (data);

	abc_alias (inmr2, inmr);

	open_rec (inum,  inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	open_rec (exwo,  exwo_list, EXWO_NO_FIELDS, "exwo_id_no");
	open_rec (pcat,  pcat_list, PCAT_NO_FIELDS, "pcat_hhwo_hash");
	open_rec (ccmr,  ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	open_rec (glmr,  glmr_list, GLMR_NO_FIELDS, "glmr_hhmr_hash");
	open_rec (incc,  incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (inmr,  inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (inmr2, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (pcln,  pcln_list, PCLN_NO_FIELDS, "pcln_id_no");
	open_rec (pcms,  pcms_list, PCMS_NO_FIELDS, "pcms_id_no");
	open_rec (pcrq,  pcrq_list, PCRQ_NO_FIELDS, "pcrq_id_no2");
	open_rec (pcwc,  pcwc_list, PCWC_NO_FIELDS, "pcwc_hhwc_hash");
	open_rec (pcwo,  pcwo_list, PCWO_NO_FIELDS, "pcwo_hhwo_hash");
	open_rec (rgrs,  rgrs_list, RGRS_NO_FIELDS, "rgrs_hhrs_hash");
	open_rec (esmr,  esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (inwu,  inwu_list, INWU_NO_FIELDS, "inwu_id_no");
	open_rec (pcgl,  pcgl_list, PCGL_NO_FIELDS, "pcgl_id_no");
	open_rec (pchs,  pchs_list, PCHS_NO_FIELDS, "pchs_hhwo_hash");
	open_rec (qchr,  qchr_list, QCHR_NO_FIELDS, "qchr_id_no");
	open_rec (comr,  comr_list, COMR_NO_FIELDS, "comr_co_no");
	open_rec ("move",move_list, MOVE_NO_FIELDS, "move_move_hash");
	if (envVarQcApply)
		open_rec (qcmr, qcmr_list, QCMR_NO_FIELDS, "qcmr_id_no");

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	/*----------------------------------
	| Read ccmr for current warehouse. |
	----------------------------------*/
	strcpy (ccmr_rec.co_no,  comm_rec.co_no);
	strcpy (ccmr_rec.est_no, comm_rec.est_no);
	strcpy (ccmr_rec.cc_no,  comm_rec.cc_no);
	cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, (char *)ccmr, DBFIND);

	OpenLocation (ccmr_rec.hhcc_hash);
	IgnoreAvailChk	=	TRUE;
	
	OpenGlmr ();

	if (envVarSoWoAllowed)
	{
		open_rec (sohr, sohr_list, SOHR_NO_FIELDS, "sohr_hhso_hash");
		open_rec (soln, soln_list, SOLN_NO_FIELDS, "soln_hhsl_hash");
	}
	strcpy (comr_rec.co_no, comm_rec.co_no);
	cc = find_rec (comr, &comr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, comr, "DBFIND");

	if (!strcmp (comr_rec.base_curr, "   "))
		sprintf (localCurrency, "%-3.3s", get_env ("CURR_CODE"));
	else
		sprintf (localCurrency, "%-3.3s", comr_rec.base_curr);

	abc_fclose (comr);
}

/*========================
| Close data base files. |
========================*/
void
CloseDB (
 void)
{
	abc_fclose (exwo);
	abc_fclose (inum);
	abc_fclose (pcat);
	abc_fclose (ccmr);
	abc_fclose (glmr);
	abc_fclose (incc);
	abc_fclose (inmr);
	abc_fclose (pcln);
	abc_fclose (pcms);
	abc_fclose (pcrq);
	abc_fclose (pcwc);
	abc_fclose (pcwo);
	abc_fclose (rgrs);
	abc_fclose (esmr);
	abc_fclose (inwu);
	abc_fclose (pcgl);
	abc_fclose (pchs);
	abc_fclose (qchr);
	abc_fclose ("move");
	CloseLocation ();
	if (envVarQcApply)
		abc_fclose (qcmr);
	GL_Close ();
	
	SearchFindClose ();

	if (envVarSoWoAllowed)
	{
		abc_fclose (sohr);
		abc_fclose (soln);
	}
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
		"WIP D LABR",
		" ",
		category
	);
	dirHash [0] = glmrRec.hhmr_hash;
	strcpy (dirAcc [0], glmrRec.acc_no);

	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"WIP D MACH",
		" ",
		category
	);
	dirHash [1] = glmrRec.hhmr_hash;
	strcpy (dirAcc [1], glmrRec.acc_no);

	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"WIP D QC  ",
		" ",
		category
	);
	dirHash [2] = glmrRec.hhmr_hash;
	strcpy (dirAcc [2], glmrRec.acc_no);

	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"WIP D SPEC",
		" ",
		category
	);
	dirHash [3] = glmrRec.hhmr_hash;
	strcpy (dirAcc [3], glmrRec.acc_no);

	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"WIP D OTH ",
		" ",
		category
	);
	dirHash [4] = glmrRec.hhmr_hash;
	strcpy (dirAcc [4], glmrRec.acc_no);

	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"WIP F LABR",
		" ",
		category
	);
	fixHash [0] = glmrRec.hhmr_hash;
	strcpy (fixAcc [0], glmrRec.acc_no);

	GL_GLI 
	(
		ccmr_rec.co_no, 
		ccmr_rec.est_no, 
		ccmr_rec.cc_no, 
		"WIP F MACH", 
		" ", 
		category
	);
	fixHash [1] = glmrRec.hhmr_hash;
	strcpy (fixAcc [1], glmrRec.acc_no);

	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"WIP F QC  ",
		" ",
		category
	);
	fixHash [2] = glmrRec.hhmr_hash;
	strcpy (fixAcc [2], glmrRec.acc_no);

	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"WIP F SPEC",
		" ",
		category
	);
	fixHash [3] = glmrRec.hhmr_hash;
	strcpy (fixAcc [3], glmrRec.acc_no);

	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"WIP F OTH ",
		" ",
		category
	);
	fixHash [4] = glmrRec.hhmr_hash;
	strcpy (fixAcc [4], glmrRec.acc_no);

	/* Manufacturing Variance Direct Accounts */
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

	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"MAN D LABR",
		" ",
		category
	);
	mfgDHash [0] = glmrRec.hhmr_hash;
	strcpy (mfgDAcc [0], glmrRec.acc_no);

	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"MAN D MACH",
		" ",
		category
	);
	mfgDHash [1] = glmrRec.hhmr_hash;
	strcpy (mfgDAcc [1], glmrRec.acc_no);

	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"MAN D QC  ",
		" ",
		category
	);
	mfgDHash [2] = glmrRec.hhmr_hash;
	strcpy (mfgDAcc [2], glmrRec.acc_no);

	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"MAN D SPEC",
		" ",
		category
	);
	mfgDHash [3] = glmrRec.hhmr_hash;
	strcpy (mfgDAcc [3], glmrRec.acc_no);

	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"MAN D OTH ",
		" ",
		category
	);
	mfgDHash [4] = glmrRec.hhmr_hash;
	strcpy (mfgDAcc [4], glmrRec.acc_no);

	/* Manufacturing Variance Fixed Accounts */
	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"MAN F LABR",
		" ",
		category
	);
	mfgFHash [0] = glmrRec.hhmr_hash;
	strcpy (mfgFAcc [0], glmrRec.acc_no);
	
	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"MAN F MACH",
		" ",
		category
	);
	mfgFHash [1] = glmrRec.hhmr_hash;
	strcpy (mfgFAcc [1], glmrRec.acc_no);

	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"MAN F QC  ",
		" ",
		category
	);
	mfgFHash [2] = glmrRec.hhmr_hash;
	strcpy (mfgFAcc [2], glmrRec.acc_no);
	
	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"MAN F SPEC",
		" ",
		category
	);
	mfgFHash [3] = glmrRec.hhmr_hash;
	strcpy (mfgFAcc [3], glmrRec.acc_no);
	
	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"MAN F OTH ",
		" ",
		category
	);
	mfgFHash [4] = glmrRec.hhmr_hash;
	strcpy (mfgFAcc [4], glmrRec.acc_no);

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
	double	stdDir, stdFix,
			actDir, actFix,
			tmp_val = 0.00;
	long	stdTime, actTime;

	memset (&stdRec, 0, sizeof (stdRec));
	memset (&actRec, 0, sizeof (actRec));
	memset (&recRec, 0, sizeof (recRec));

	/*-----------------------
	| Process pcms records. |
	-----------------------*/
	pcms_rec.hhwo_hash 	= pcwo_rec.hhwo_hash;
	pcms_rec.uniq_id 	= 0;
	cc = find_rec (pcms, &pcms_rec, GTEQ, "r");
	while (!cc && pcms_rec.hhwo_hash == pcwo_rec.hhwo_hash)
	{
		/*-----------------------
		| Find item master file |
		-----------------------*/
		inmr2_rec.hhbr_hash = pcms_rec.mabr_hash;
		cc = find_rec (inmr2, &inmr2_rec, COMPARISON, "r");
		if (cc)
			inmr2_rec.outer_size = 1.00;

		/*----------------------------------------------------
		| Find item branch file at the manufacturing branch. | 
		----------------------------------------------------*/
		inei2Rec.hhbr_hash = pcms_rec.mabr_hash;
		strcpy (inei2Rec.est_no, pcwo_rec.br_no);
		cc = find_rec (inei, &inei2Rec, COMPARISON, "r");
		if (cc)
			inei2Rec.std_cost = 0.00;

		ProcessInum ();

		inei2Rec.std_cost /= (double) fourdec (cnvFct);
		pcms_rec.matl_wst_pc += 100.00;
		pcms_rec.matl_wst_pc /= 100.00;

		/*-------------------------
		| Standard material cost. |
		-------------------------*/
		tmp_val = out_cost (inei2Rec.std_cost, inmr2_rec.outer_size);
		tmp_val *= (pcms_rec.matl_qty * pcms_rec.matl_wst_pc);
		tmp_val	=	twodec (tmp_val);
		stdRec.matCost += CENTS (tmp_val);

		/*-----------------------
		| Actual material cost. |
		-----------------------*/
		actRec.matCost += pcms_rec.amt_issued;

		/*-----------------------------------
		| Receipted material cost - so far. |
		-----------------------------------*/
		recRec.matCost += pcms_rec.amt_recptd;

		cc = find_rec (pcms, &pcms_rec, NEXT, "r");
	}

	/*-----------------------
	| Process pcln records. |
	-----------------------*/
	pcln_rec.hhwo_hash	= pcwo_rec.hhwo_hash;
	pcln_rec.seq_no		= 0;
	pcln_rec.line_no	= 0;
	cc = find_rec (pcln, &pcln_rec, GTEQ, "r");
	while (!cc && pcln_rec.hhwo_hash == pcwo_rec.hhwo_hash)
	{
		stdTime = 0L;
		stdDir = stdFix = 0.00;

		/*----------------
		| Standard time. |
		----------------*/
		stdTime =  pcln_rec.setup;
		stdTime += pcln_rec.run;
		stdTime += pcln_rec.clean;

		if (stdTime)
		{
			/*-----------------------
			| Standard direct cost. |
			-----------------------*/
			stdDir = (double) stdTime * pcln_rec.rate;
			stdDir /= 60.00;
			stdDir *= (double) pcln_rec.qty_rsrc;

			/*-------------------------
			| Standard overhead cost. |
			-------------------------*/
			stdFix = (double) stdTime * pcln_rec.ovhd_var;
			stdFix /= 60.00;
			stdFix += pcln_rec.ovhd_fix;
			stdFix *= (double) pcln_rec.qty_rsrc;
		}

		actTime = 0L;
		pcrq_rec.hhwo_hash = pcln_rec.hhwo_hash;
		pcrq_rec.seq_no = pcln_rec.seq_no;
		pcrq_rec.line_no = pcln_rec.line_no;
		cc = find_rec (pcrq, &pcrq_rec, GTEQ, "r");
		while (!cc &&
			pcrq_rec.hhwo_hash == pcln_rec.hhwo_hash &&
			pcrq_rec.seq_no == pcln_rec.seq_no &&
			pcrq_rec.line_no == pcln_rec.line_no)
		{
			/*--------------
			| Actual time. |
			--------------*/
			actTime += pcrq_rec.act_setup;
			actTime += pcrq_rec.act_run;
			actTime += pcrq_rec.act_clean;

			cc = find_rec (pcrq, &pcrq_rec, NEXT, "r");
		}

		/*------------------
		| Actual line cost |
		------------------*/
		actDir = (double) actTime * pcln_rec.rate;
		actDir /= 60.00;
		actDir *= pcln_rec.qty_rsrc;

		/*------------------------
		| Actual overhead costs. |
		------------------------*/
		actFix = (double) actTime * pcln_rec.ovhd_var;
		actFix /= 60.00;
		actFix += pcln_rec.ovhd_fix;
		actFix *= (double) pcln_rec.qty_rsrc;

		/*---------------------------------------------------------
		| Check resource type, and add to appropriate total cost. |
		---------------------------------------------------------*/
		rgrs_rec.hhrs_hash	=	pcln_rec.hhrs_hash;
		cc = find_rec (rgrs, &rgrs_rec, EQUAL, "r");
		if (!cc)
		{
			switch (rgrs_rec.type [0])
			{
			case	'L': /* Labour */
				stdRec.dLabour += stdDir;
				stdRec.fLabour += stdFix;
				actRec.dLabour += actDir;
				actRec.fLabour += actFix;
				recRec.dLabour += (	pcln_rec.amt_recptd - pcln_rec.ovh_recptd);
				recRec.fLabour += pcln_rec.ovh_recptd;
				break;
			case	'M': /* Machine */
				stdRec.dMachine += stdDir;
				stdRec.fMachine += stdFix;
				actRec.dMachine += actDir;
				actRec.fMachine += actFix;
				recRec.dMachine += (pcln_rec.amt_recptd - pcln_rec.ovh_recptd);
				recRec.fMachine += pcln_rec.ovh_recptd;
				break;
			case	'Q': /* QC-Check */
				stdRec.dQcCheck += stdDir;
				stdRec.fQcCheck += stdFix;
				actRec.dQcCheck += actDir;
				actRec.fQcCheck += actFix;
				recRec.dQcCheck += (pcln_rec.amt_recptd - pcln_rec.ovh_recptd);
				recRec.fQcCheck += pcln_rec.ovh_recptd;
				break;
			case	'S': /* Special */
				stdRec.dSpecial += stdDir;
				stdRec.fSpecial += stdFix;
				actRec.dSpecial += actDir;
				actRec.fSpecial += actFix;
				recRec.dSpecial += (pcln_rec.amt_recptd - pcln_rec.ovh_recptd);
				recRec.fSpecial += pcln_rec.ovh_recptd;
				break;
			case	'O': /* Other */
				stdRec.dOther += stdDir;
				stdRec.fOther += stdFix;
				actRec.dOther += actDir;
				actRec.fOther += actFix;
				recRec.dOther += (pcln_rec.amt_recptd - pcln_rec.ovh_recptd);
				recRec.fOther += pcln_rec.ovh_recptd;
				break;
			}
		}
		
		cc = find_rec (pcln, &pcln_rec, NEXT, "r");
	}

	standardCost  = stdRec.matCost;
	standardCost += (stdRec.dLabour   + stdRec.fLabour);
	standardCost += (stdRec.dMachine  + stdRec.fMachine);
	standardCost += (stdRec.dQcCheck  + stdRec.fQcCheck);
	standardCost += (stdRec.dSpecial  + stdRec.fSpecial);
	standardCost += (stdRec.dOther    + stdRec.fOther);

	actCost  = actRec.matCost;
	actCost += (actRec.dLabour   + actRec.fLabour);
	actCost += (actRec.dMachine  + actRec.fMachine);
	actCost += (actRec.dQcCheck  + actRec.fQcCheck);
	actCost += (actRec.dSpecial  + actRec.fSpecial);
	actCost += (actRec.dOther    + actRec.fOther);

	actCost			=	no_dec (actCost);
	standardCost	=	no_dec (standardCost);
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
	inum_rec.hhum_hash	=	inmr2_rec.alt_uom;
	cc = find_rec (inum, &inum_rec, EQUAL, "r");
	if (cc)
		file_err (cc, inum, DBFIND);

	sprintf (altUomGroup, "%-20.20s", inum_rec.uom_group);
	strcpy (altUom, inum_rec.uom);
	altCnvFct 	= inum_rec.cnv_fct;
	altHhumHash	= inum_rec.hhum_hash;

	inum_rec.hhum_hash	=	inmr2_rec.std_uom;
	cc = find_rec (inum, &inum_rec, EQUAL, "r");
	if (cc)
		file_err (cc, inum, DBFIND);

	strcpy (stdUom, inum_rec.uom);
	strcpy (stdUomGroup, inum_rec.uom_group);
	stdCnvFct	= inum_rec.cnv_fct;
	stdHhumHash	= inum_rec.hhum_hash;

	inum_rec.hhum_hash	=	pcms_rec.uom;
	cc = find_rec (inum, &inum_rec, EQUAL, "r");
	if (cc)
		file_err (cc, inum, DBFIND);

	cnvFct = inum_rec.cnv_fct;

	/*------------------------------------------------
	| converts a float to arbitrary precision number |
	| defined as number.                             |
	------------------------------------------------*/
	NumFlt (&_stdCnvFct, stdCnvFct);
	NumFlt (&_altCnvFct, altCnvFct);
	NumFlt (&_cnvFct, 	 cnvFct);

	/*----------------------------------------------------------
	| a function that divides one number by another and places |
	| the result in another number defined variable            |
	| Conversion factor = std uom cnvFct / iss uom cnvFct      |
	|      OR                                                  |
	| Conversion factor = (std uom cnvFct / iss uom cnvFct)    |
	|                     * item's conversion factor           |
	| Same calculations as in pc_recprt.                       |
	----------------------------------------------------------*/
	if (strcmp (altUomGroup, inum_rec.uom_group))
		NumDiv (&_stdCnvFct, &_cnvFct, &_result);
	else
	{
		NumFlt (&_uomCfactor,inmr2_rec.uom_cfactor);
		NumDiv (&_altCnvFct, &_cnvFct, 		&_result);
		NumMul (&_result, 	 &_uomCfactor, 	&_result);
	}
	/*---------------------------------------
	| converts a arbitrary precision number |
	| to a float                            |
	---------------------------------------*/
	cnvFct = NumToFlt (&_result);
}
/*===============================
| add transaction to incc files |
===============================*/
int
ProcReceipt (
	long	hhwoHash,
	float	qtyReceipt,
    char	*processSO)
{
	int		i,
			j,
			mth_idx,
			NoLots				= TRUE,
			closeJob			= FALSE;

	float	qtyCnvFct 	= 0.00;
	double	origQty 	= 0.00,
			workQty 	= 0.00,
			origCost 	= 0.00,
			workCost	= 0.00,
			totalCost	= 0.00;

	receiptDate	=	TodaysDate ();
	/*-------------------
	| Find Works order. |
	-------------------*/
	pcwo_rec.hhwo_hash	=	hhwoHash;
	cc = find_rec (pcwo, &pcwo_rec, EQUAL, "u");
	if (cc)
	{
		abc_unlock (pcwo);
		return (PRError (ERR_WOR_NO_ORDER));
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

	thisReceipt = qtyReceipt + qtyReject;
	prevReceipt = NDEC (pcwo_rec.act_prod_qty) + NDEC (pcwo_rec.act_rej_qty);

    /* added condition on closing WO when activity type is 'F' */
	closeJob = FALSE;
	if ((thisReceipt + prevReceipt) >= NDEC (pcwo_rec.prod_qty) || processSO [0] == 'F')
		closeJob = TRUE;

	/*---------------------------------------------------
	| Get all the general ledger stuff, and lots of it. |
	---------------------------------------------------*/
	cc = ReadDefault (inmr_rec.category);
	if (cc)
		return (PRError (ERR_WOR_GLI));
 
	/*---------------------------
	| Process control Accounts. |
	---------------------------*/
	GetAccount (inmr_rec.category);

	/*------------------
	| Calculate costs. |
	------------------*/
	outerSize = inmr_rec.outer_size;
	CalcCost ();

	standardCost /= pcwo_rec.prod_qty;
	standardCost *= outerSize;
	costPerUnit = standardCost / outerSize;

	memcpy ((char *)&inmr2_rec, (char *)&inmr_rec, sizeof (struct inmrRecord));

	ProcessInum ();
	/*-----------------------
	| Calculate expiry date |
	-----------------------*/
	expiryDate = comm_rec.inv_date;
	for (j = 0; j < inei_expiry_prd [0]; j++)
	{
		mth_idx = (periodMonth + j) % 12;
		expiryDate += (long) (mnths [mth_idx].noDays);
	}

	/*------------------------------------
	| Check if works order can be closed |
	------------------------------------*/
	if (JOB_CLOSING || closeJob)
	{
		if (closeJob)
		{
			pcat_rec.hhwo_hash = pcwo_rec.hhwo_hash;
			cc = find_rec (pcat, &pcat_rec, GTEQ, "r");
			while (!cc && pcat_rec.hhwo_hash == pcwo_rec.hhwo_hash)
			{
				if (pcat_rec.stat_flag [0] != 'U')
					return (PRError (ERR_WOR_TIME));
					
				cc = find_rec (pcat, &pcat_rec, NEXT, "r");
			}
		}
		/*-----------------------
		| Calculate actual cost |
		-----------------------*/
		if (pcwo_rec.act_prod_qty == 0.00)
			actCost /= qtyReceipt;
		else
			actCost /= (NDEC (pcwo_rec.act_prod_qty) + qtyReceipt);

		actCost *= outerSize;
		actCost	= no_dec (actCost);

		closeJob = TRUE;
	}

	/*--------------------------------------------------
	| Update all inventory and general ledger records. |
	--------------------------------------------------*/
	origQty 	= 0.0;
	workQty 	= 0.0;
	origCost 	= 0.0;

	/*--------------------------------------------------
	| Find item/warehouse record for works order line. |
	--------------------------------------------------*/
	incc_rec.hhcc_hash 	= pcwo_rec.hhcc_hash;
	incc_rec.hhbr_hash 	= pcwo_rec.hhbr_hash;
	cc = find_rec (incc, &incc_rec, COMPARISON, "u");
	if (cc)
	{
		abc_unlock (incc);
		return (PRError (ERR_WOR_NO_INCC));
	}

	cc = DisplayLL
		 (										/*----------------------*/
			0,									/*	Line number.		*/
			0,									/*  Row for window		*/
			0,									/*  Col for window		*/
			0,									/*  length for window	*/
			incc_rec.hhwh_hash, 				/*	Warehouse hash.		*/
			stdHhumHash,						/*	UOM hash			*/
			incc_rec.hhcc_hash,					/*	CC hash.			*/
			stdUom,								/* UOM					*/
			(float) qtyReceipt,					/* Quantity.			*/
			fourdec (cnvFct),					/* Conversion factor.	*/
			expiryDate,							/* Expiry Date.			*/
			TRUE,								/* Silent mode			*/
			TRUE,								/* Input Mode.			*/
			inmr_rec.lot_ctrl					/* Lot controled item. 	*/
												/*----------------------*/
		);
	if (cc)
	{
		abc_unlock (incc);
		return (PRError (ERR_WOR_LOTALLOC));
	}
	/*-------------------------------------------------------
	| update inventory cost centre stock record (file incc)	|
	-------------------------------------------------------*/
	incc_rec.receipts  		+= (float) NDEC (qtyReceipt);
	incc_rec.ytd_receipts 	+= (float) NDEC (qtyReceipt);
	if (envVarQcApply && QCITEM)
		incc_rec.qc_qty	+= (float) NDEC (qtyReceipt);
	origQty = NDEC (incc_rec.closing_stock);
	workQty = NDEC (incc_rec.closing_stock);
	incc_rec.closing_stock = (float) (NDEC (incc_rec.opening_stock) +
				  						NDEC (incc_rec.pur) +
				  						NDEC (incc_rec.receipts) +
				  						NDEC (incc_rec.adj) -
				  						NDEC (incc_rec.issues) -
				  						NDEC (incc_rec.sales));

	/*--------------------------
	| Update warehouse record. |
	--------------------------*/
	cc = abc_update (incc,&incc_rec);
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
			file_err (cc, inwu, DBFIND);
	}
	inwu_rec.receipts  += (float) NDEC (qtyReceipt);
	inwu_rec.closing_stock = (float) (NDEC (inwu_rec.opening_stock) +
							 		  NDEC (inwu_rec.pur) +
							 		  NDEC (inwu_rec.receipts) +
							 		  NDEC (inwu_rec.adj) -
							 		  NDEC (inwu_rec.issues) -
							 		  NDEC (inwu_rec.sales));

	cc = abc_update (inwu,&inwu_rec);
	if (cc)
		file_err (cc, inwu, "DBUPDATE");

	/*---------------------------------
	| Somes details have been printed |
	---------------------------------*/
	PrintDetails (closeJob);

	if (qtyReceipt != 0.00)
	{
		/*-------------------------------------------
		| Find branch record from master item hash. |
		-------------------------------------------*/
		cc = FindInei (inmr_rec.hhbr_hash, pcwo_rec.rec_br_no, "u");
		if (cc)
			file_err (cc, inei, DBFIND);

		origCost = ineiRec.avge_cost;

		if (origQty < 0.00)
			workQty = 0.00;

		if (origQty + qtyReceipt == 0.00)
			ineiRec.avge_cost = twodec (DOLLARS (actCost));
		else
		{
			if (origQty + qtyReceipt < 0.00)
				ineiRec.avge_cost = 0.00;
			else
			{
				ineiRec.avge_cost = twodec (((workQty * origCost) + 
						 (qtyReceipt * DOLLARS (actCost))) / 
						 (workQty + qtyReceipt));
			}
		}
		if (closeJob)
		{
			ineiRec.prev_cost = ineiRec.last_cost;
			ineiRec.last_cost = twodec (DOLLARS (actCost));
			ineiRec.date_lcost = comm_rec.inv_date;
		}

		/*--------------------------
		| Update branch records.   |
		--------------------------*/
		cc = abc_update (inei,&ineiRec);
		if (cc)
			file_err (cc, inei, "DBUPDATE");

		switch (inmr_rec.costing_flag [0])
		{
		case 'F':
		case 'I':

			/*-------------------------------------------
			| Product is in stock take mode so control  |
			| record  needs to be checked.              |
			-------------------------------------------*/
			if (incc_rec.stat_flag [0] >= 'A' && incc_rec.stat_flag [0] <= 'Z')
			{
				while (CheckInsc (incc_rec.hhcc_hash, receiptDate,incc_rec.stat_flag))
					receiptDate += 1L;
			}
			/*-------------------------------------------------------
			| Add FIFO record for the mfg branch and warehouse.     |
			-------------------------------------------------------*/
			cc	=	AddIncf 
					(
						incc_rec.hhwh_hash,
						receiptDate,
						DOLLARS (standardCost),
						DOLLARS (standardCost),
						(float) qtyReceipt,
						pcwo_rec.batch_no,
						DOLLARS (standardCost),
						0.00,
						0.00,
						0.00,
						0.00,
						DOLLARS (standardCost),
						"A"
					);
			if (cc)
				file_err (cc, incf, "DBADD");

			if (closeJob)
			{
				UpdateFifo 
				 (
					incc_rec.hhwh_hash,
					(double) DOLLARS (actCost),
					(double) DOLLARS (standardCost)
				);
			}
			break;

		default:
			break;
		}
	}

	inmr_rec.on_hand += (float) NDEC (qtyReceipt);
	if (envVarQcApply && QCITEM)
		inmr_rec.qc_qty += (float) NDEC (qtyReceipt);

	/*----------------------------------
	| Update inventory master records. |
	----------------------------------*/
	cc = abc_update (inmr, &inmr_rec);
	if (cc)
		file_err (cc, inmr, "DBUPDATE");

	NoLots = TRUE;

	for (i = 0; i < MAX_LOTS; i++)
	{
		if (!LL_Valid (0, i))
			break;

		UpdateLotLocation (0, FALSE);

		NoLots	=	FALSE;

		/*--------------------------
		| Log inventory movements. |
		--------------------------*/
		MoveAdd
		(
			comm_rec.co_no,
			comm_rec.est_no,
			comm_rec.cc_no,
			incc_rec.hhbr_hash,
			incc_rec.hhcc_hash,
			inmr_rec.std_uom,
			receiptDate,
			10,
			pcwo_rec.batch_no,
			inmr_rec.inmr_class,
			inmr_rec.category,
			pcwo_rec.order_no,
			pcwo_rec.batch_no, 
			GetBaseQty (0, i),
			0.00,
			actCost
		);
		if (envVarQcApply && QCITEM)
			UpdateQchr (GetBaseQty (0, i), GetINLO (0, i));
	}
	if (NoLots)
	{
		/*--------------------------
		| Log inventory movements. |
		--------------------------*/
		MoveAdd
		 (
			comm_rec.co_no,
			comm_rec.est_no,
			comm_rec.cc_no,
			incc_rec.hhbr_hash,
			incc_rec.hhcc_hash,
			inmr_rec.std_uom,
			receiptDate,
			10,
			pcwo_rec.order_no,
			inmr_rec.inmr_class,
			inmr_rec.category,
			pcwo_rec.order_no,
			pcwo_rec.order_no,
			 (float)qtyReceipt,
			0.00,
			actCost
		);
		if (envVarQcApply && QCITEM)
			UpdateQchr ((float)qtyReceipt, 0L);
	}

	totalCost 	=	UpdatePcms (closeJob);
	totalCost 	+= 	UpdatePcln (closeJob);

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

	qtyCnvFct = (float) (thisReceipt / NDEC (pcwo_rec.prod_qty));

	/* post std cost of materials to the material account */
	workCost = NDEC (stdRec.matCost * qtyCnvFct);
	thisRec.matCost = workCost;
	totalCost -= workCost;
	AddPcgl 
	 (
		matHash [0],
		matAcc [0],
		pcwc_rec.work_cntr,
		(workCost > 0.00) ? workCost : -workCost,
		(workCost > 0.00) ? "2" : "1",
		"19"
	);

	/* post std cost of direct labour to the direct labour account */
	workCost = no_dec (stdRec.dLabour * qtyCnvFct);
	thisRec.dLabour = workCost;
	totalCost -= workCost;
	AddPcgl 
	 (
		dirHash [0],
		dirAcc [0],
		pcwc_rec.work_cntr,
		 (workCost > 0.00) ? workCost : -workCost,
		 (workCost > 0.00) ? "2" : "1",
		"19"
	);

	/* post std cost of fixed labour to the fixed labour account */
	workCost = no_dec (stdRec.fLabour * qtyCnvFct);
	thisRec.fLabour = workCost;
	totalCost -= workCost;
	AddPcgl 
	 (
		fixHash [0],
		fixAcc [0],
		pcwc_rec.work_cntr,
		 (workCost > 0.00) ? workCost : -workCost,
		 (workCost > 0.00) ? "2" : "1",
		"19"
	);

	/* post std cost of direct machine to the direct machine account */
	workCost = no_dec (stdRec.dMachine * qtyCnvFct);
	thisRec.dMachine = workCost;
	totalCost -= workCost;
	AddPcgl 
	 (
		dirHash [1],
		dirAcc [1],
		pcwc_rec.work_cntr,
		 (workCost > 0.00) ? workCost : -workCost,
		 (workCost > 0.00) ? "2" : "1",
		"19"
	);

	/* post std cost of fixed machine to the fixed machine account */
	workCost = no_dec (stdRec.fMachine * qtyCnvFct);
	thisRec.fMachine = workCost;
	totalCost -= workCost;
	AddPcgl 
	 (
		fixHash [1],
		fixAcc [1],
		pcwc_rec.work_cntr,
		 (workCost > 0.00) ? workCost : -workCost,
		 (workCost > 0.00) ? "2" : "1",
		"19"
	);

	/* post std cost of direct qc_check to the direct qc_check account */
	workCost = no_dec (stdRec.dQcCheck * qtyCnvFct);
	thisRec.dQcCheck = workCost;
	totalCost -= workCost;
	AddPcgl 
	 (
		dirHash [2],
		dirAcc [2],
		pcwc_rec.work_cntr,
		 (workCost > 0.00) ? workCost : -workCost,
		 (workCost > 0.00) ? "2" : "1",
		"19"
	);

	/* post std cost of fixed qc_check to the fixed qc_check account */
	workCost = no_dec (stdRec.fQcCheck * qtyCnvFct);
	thisRec.fQcCheck = workCost;
	totalCost -= workCost;
	AddPcgl
	 (
		fixHash [2],
		fixAcc [2],
		pcwc_rec.work_cntr,
		 (workCost > 0.00) ? workCost : -workCost,
		 (workCost > 0.00) ? "2" : "1",
		"19"
	);

	/* post std cost of direct special to the direct special account */
	workCost = no_dec (stdRec.dSpecial * qtyCnvFct);
	thisRec.dSpecial = workCost;
	totalCost -= workCost;
	AddPcgl 
	 (
		dirHash [3],
		dirAcc [3],
		pcwc_rec.work_cntr,
		 (workCost > 0.00) ? workCost : -workCost,
		 (workCost > 0.00) ? "2" : "1",
		"19"
	);

	/* post std cost of fixed special to the fixed special account */
	workCost = no_dec (stdRec.fSpecial * qtyCnvFct);
	thisRec.fSpecial = workCost;
	totalCost -= workCost;
	AddPcgl 
	 (
		fixHash [3],
		fixAcc [3],
		pcwc_rec.work_cntr,
		 (workCost > 0.00) ? workCost : -workCost,
		 (workCost > 0.00) ? "2" : "1",
		"19"
	);

	/* post std cost of direct other to the direct other account */
	workCost = no_dec (stdRec.dOther * qtyCnvFct);
	thisRec.dOther = workCost;
	totalCost -= workCost;
	AddPcgl 
	 (
		dirHash [4],
		dirAcc [4],
		pcwc_rec.work_cntr,
		 (workCost > 0.00) ? workCost : -workCost,
		 (workCost > 0.00) ? "2" : "1",
		"19"
	);

	/* post std cost of fixed other to the fixed other account */
	workCost = no_dec (stdRec.fOther * qtyCnvFct);
	thisRec.fOther = workCost;
	totalCost -= workCost;
	AddPcgl 
	 (
		fixHash [4],
		fixAcc [4],
		pcwc_rec.work_cntr,
		 (workCost > 0.00) ? workCost : -workCost,
		 (workCost > 0.00) ? "2" : "1",
		"19"
	);

	/*------------------------------
	| Write-Off rejected quantity. |
	------------------------------*/
	if (qtyReject > 0.00)
	{
		double	value = out_cost (standardCost, inmr_rec.outer_size);

		strcpy (glmrRec.co_no,	comm_rec.co_no);
		strcpy (glmrRec.acc_no,	writeOffAcc);
		if ((cc = find_rec (glmr, &glmrRec, EQUAL, "r")))
			file_err (cc, glmr, DBFIND);

		AddPcgl 
		 (
			glmrRec.hhmr_hash,
			glmrRec.acc_no,
			pcwc_rec.work_cntr,
			qtyReject * value,
			"1",
			"12"
		);
		AddPcgl 
		 (
			creditHash,
			creditAccount,
			pcwc_rec.work_cntr,
			qtyReject * value,
			"2",
			"12"
		);
	}
	/*--------------------------------------------------------------
	| If closeJob, post difference between the std and the actual  |
	| cost to the manufacturing variance accounts.                 |
	| The variance is the actual cost less the already receipted   |
	| costs plus this receipted costs.                             |
	--------------------------------------------------------------*/
	if (closeJob) 
	{
		/* post mfg variance materials to the material account */
		workCost = no_dec (thisRec.matCost + recRec.matCost);
		workCost = no_dec (actRec.matCost - workCost);
		totalCost -= workCost;
		AddPcgl 
		 (
			matHash [1],
			matAcc [1],
			pcwc_rec.work_cntr,
			 (workCost > 0) ? workCost : -workCost,
			 (workCost > 0) ? "2" : "1",
			"19"
		);

		/* post mfg variance direct labour to the direct labour account */
		workCost = no_dec (thisRec.dLabour + recRec.dLabour);
		workCost = no_dec (actRec.dLabour - workCost);
		totalCost -= workCost;
		AddPcgl 
		 (
			mfgDHash [0],
			mfgDAcc [0],
			pcwc_rec.work_cntr,
			 (workCost > 0) ? workCost : -workCost,
			 (workCost > 0) ? "2" : "1",
			"19"
		);
		/* post mfg variance fixed labour to the fixed labour account */
		workCost = no_dec (thisRec.fLabour + recRec.fLabour);
		workCost = no_dec (actRec.fLabour - workCost);
		totalCost -= workCost;
		AddPcgl
		 (
			mfgFHash [0],
			mfgFAcc [0],
			pcwc_rec.work_cntr,
			 (workCost > 0) ? workCost : -workCost,
			 (workCost > 0) ? "2" : "1",
			"19"
		);

		/* post mfg variance direct machine to the direct machine account */
		workCost = no_dec (thisRec.dMachine + recRec.dMachine);
		workCost = no_dec (actRec.dMachine - workCost);
		totalCost -= workCost;
		AddPcgl
		 (
			mfgDHash [1],
			mfgDAcc [1],
			pcwc_rec.work_cntr,
			 (workCost > 0) ? workCost : -workCost,
			 (workCost > 0) ? "2" : "1",
			"19"
		);
		/* post mfg variance fixed machine to the fixed machine account */
		workCost = no_dec (thisRec.fMachine + recRec.fMachine);
		workCost = no_dec (actRec.fMachine - workCost);
		totalCost -= workCost;
		AddPcgl
		 (
			mfgFHash [1],
			mfgFAcc [1],
			pcwc_rec.work_cntr,
			 (workCost > 0) ? workCost : -workCost,
			 (workCost > 0) ? "2" : "1",
			"19"
		);

		/* post mfg var direct qc_check to the direct qc_check account */
		workCost = no_dec (thisRec.dQcCheck + recRec.dQcCheck);
		workCost = no_dec (actRec.dQcCheck - workCost);
		totalCost -= workCost;
		AddPcgl
		 (
			mfgDHash [2],
			mfgDAcc [2],
			pcwc_rec.work_cntr,
			 (workCost > 0) ? workCost : -workCost,
			 (workCost > 0) ? "2" : "1",
			"19"
		);
		/* post mfg variance fixed qc_check to the fixed qc_check account */
		workCost = no_dec (thisRec.fQcCheck + recRec.fQcCheck);
		workCost = no_dec (actRec.fQcCheck - workCost);
		totalCost -= workCost;
		AddPcgl
		 (
			mfgFHash [2],
			mfgFAcc [2],
			pcwc_rec.work_cntr,
			 (workCost > 0) ? workCost : -workCost,
			 (workCost > 0) ? "2" : "1",
			"19"
		);

		/* post mfg variance direct special to the direct special account */
		workCost = no_dec (thisRec.dSpecial + recRec.dSpecial);
		workCost = no_dec (actRec.dSpecial - workCost);
		totalCost -= workCost;
		AddPcgl
		 (
			mfgDHash [3],
			mfgDAcc [3],
			pcwc_rec.work_cntr,
			 (workCost > 0) ? workCost : -workCost,
			 (workCost > 0) ? "2" : "1",
			"19"
		);
		/* post mfg variance fixed special to the fixed special account */
		workCost = no_dec (thisRec.fSpecial + recRec.fSpecial);
		workCost = no_dec (actRec.fSpecial - workCost);
		totalCost -= workCost;
		AddPcgl
		 (
			mfgFHash [3],
			mfgFAcc [3],
			pcwc_rec.work_cntr,
			 (workCost > 0) ? workCost : -workCost,
			 (workCost > 0) ? "2" : "1",
			"19"
		);

		/* post mfg variance direct other to the direct other account */
		workCost = no_dec (thisRec.dOther + recRec.dOther);
		workCost = no_dec (actRec.dOther - workCost);
		totalCost -= workCost;
		AddPcgl
		 (
			mfgDHash [4],
			mfgDAcc [4],
			pcwc_rec.work_cntr,
			 (workCost > 0) ? workCost : -workCost,
			 (workCost > 0) ? "2" : "1",
			"19"
		);
		/* post mfg variance fixed other to the fixed other account */
		workCost = no_dec (thisRec.fOther + recRec.fOther);
		workCost = no_dec (actRec.fOther - workCost);
		totalCost -= workCost;
		AddPcgl
		 (
			mfgFHash [4],
			mfgFAcc [4],
			pcwc_rec.work_cntr,
			 (workCost > 0) ? workCost : -workCost,
			 (workCost > 0) ? "2" : "1",
			"19"
		);

		/* post total act cost */
		AddPcgl
		 (
			creditHash,
			creditAccount,
			pcwc_rec.work_cntr,
			 ((actCost / inmr_rec.outer_size) *
				 (qtyReceipt + pcwo_rec.act_prod_qty)),
			"1",
			"19"
		);
		/* post total std cost */
		AddPcgl
		 (
			creditHash,
			creditAccount,
			pcwc_rec.work_cntr,
			 ((standardCost / inmr_rec.outer_size) *
				 (qtyReceipt + pcwo_rec.act_prod_qty)),
			"2",
			"19"
		);
	}
	/*-------------------------------------------
	| Update act_prod_qty & act_rej_qty on pcwo |
	-------------------------------------------*/
	pcwo_rec.act_prod_qty 	+= (float) qtyReceipt;
	pcwo_rec.act_rej_qty 	+= (float) qtyReject;

	/*-------------------------------
	| Close job if status is "C"	|
	| AND user has requested close.	|
	-------------------------------*/
	if (closeJob)
		strcpy (pcwo_rec.order_status, "Z");

	cc = abc_update (pcwo, &pcwo_rec);
	if (cc)
		file_err (cc, pcwo, "DBUPDATE");

	if (closeJob)
	{
		strcpy (pchs_rec.co_no,		pcwo_rec.co_no);
		strcpy (pchs_rec.br_no,		pcwo_rec.br_no);
		strcpy (pchs_rec.wh_no,		pcwo_rec.wh_no);
		strcpy (pchs_rec.order_no,	pcwo_rec.order_no);
		strcpy (pchs_rec.batch_no,	pcwo_rec.batch_no);
		pchs_rec.hhwo_hash			= pcwo_rec.hhwo_hash;
		pchs_rec.hhbr_hash			= pcwo_rec.hhbr_hash;
		pchs_rec.prod_qty			= pcwo_rec.prod_qty;
		pchs_rec.act_prod_qty		= pcwo_rec.act_prod_qty;
		pchs_rec.act_rej_qty		= pcwo_rec.act_rej_qty;
		pchs_rec.batch_size			= ineiRec.std_batch;
		pchs_rec.outer_size			= inmr_rec.outer_size;
		pchs_rec.std_cost			= DOLLARS (standardCost);
		pchs_rec.act_cost			= DOLLARS (actCost);
		pchs_rec.bom_no				= pcwo_rec.bom_alt;
		pchs_rec.rtg_no				= pcwo_rec.rtg_alt;

		cc = abc_add (pchs, &pchs_rec);
		if (cc)
			file_err (cc, pchs, DBFIND);
	}

	if (pcwo_rec.hhsl_hash > 0L && envVarSoWoAllowed)
    {
		ProcessSalesOrder (pcwo_rec.hhsl_hash, processSO, qtyReceipt);
    }
	/*-----------------------------------
	| add sobg record for recalculation |
	| of final product on order qty     |
	-----------------------------------*/
	add_hash 
	(
		pcwo_rec.co_no, /* re-calc manufacturing warehouse */
		pcwo_rec.br_no,
		"RP",
		0,
		pcwo_rec.hhbr_hash,
		pcwo_rec.hhcc_hash,
		0L,
		 (double) 0
	);
 
	/* find requesting warehouse name */
	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, pcwo_rec.req_br_no);
	strcpy (ccmr_rec.cc_no, pcwo_rec.req_wh_no);
	cc = find_rec (ccmr, &ccmr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, ccmr, DBFIND);

	add_hash 
	 (
		pcwo_rec.co_no,  /* re-calc requesting warehouse */
		pcwo_rec.br_no,
		"RP",
		0,
		pcwo_rec.hhbr_hash,
		ccmr_rec.hhcc_hash,
		0L,
		 (double) 0
	);  /* decrease on order qty for item produced */

	recalc_sobg ();
	return (EXIT_SUCCESS);
}

/*===================================================
| Adds a inventory QC purchase items reveival file. |
===================================================*/
void
UpdateQchr (
 float	qcQty,
 long 	inloHash)
{
	float	qcDays;


	incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
	if (find_rec (incc, &incc_rec, EQUAL, "r"))
		incc_rec.qc_time = 0.00;
	qcDays = incc_rec.qc_time * 7; /* get qc_time in days */

	strcpy (qchr_rec.co_no,			ccmr_rec.co_no);
	strcpy (qchr_rec.br_no,			ccmr_rec.est_no);
	strcpy (qchr_rec.wh_no,			ccmr_rec.cc_no);
	/* TO FIX */
	strcpy (qchr_rec.qc_centre,		" ");
	qchr_rec.hhbr_hash				= inmr_rec.hhbr_hash;
	qchr_rec.origin_qty				= qcQty;
	qchr_rec.receipt_dt				= receiptDate;
	qchr_rec.exp_rel_dt				= receiptDate + (long) qcDays;
	qchr_rec.rel_qty				= 0.00;
	qchr_rec.rej_qty				= 0.00;

	if (LOT_CTRL)
	{
		int		i, mthIdx;
		long	expiryDate = comm_rec.inv_date;
		/*-----------------------
		| Calculate expiry date |
		-----------------------*/
		for (i = 0; i < inei_expiry_prd [0]; i++)
		{
			mthIdx = (periodMonth + i) % 12;
			expiryDate += (long) (mnths [mthIdx].noDays);
		}
	}

	qchr_rec.inlo_hash = inloHash;
	strcpy (qchr_rec.serial_no,	" ");
	strcpy (qchr_rec.ref_1,			pcwo_rec.order_no);
	strcpy (qchr_rec.ref_2,			pcwo_rec.batch_no);
	qchr_rec.next_seq				= 0;
	strcpy (qchr_rec.source_type,	"W");

	if (abc_add (qchr, &qchr_rec))
		file_err (cc, qchr, DBFIND);
}

/*=======================================================
| Update actual costs to all related fifo records.      |
=======================================================*/
void
UpdateFifo (
	long	hhwhHash, 
	double	actCost, 
	double	stdCost)
{
	abc_selfield (incf, "incf_id_no_2");

	incfRec.hhwh_hash = hhwhHash;
	strncpy (incfRec.gr_number, pcwo_rec.batch_no, 10);
	cc = find_rec (incf, &incfRec, GTEQ, "u");
	while (!cc && incfRec.hhwh_hash == hhwhHash &&
				  !strncmp (incfRec.gr_number, pcwo_rec.batch_no, 10))
	{
		incfRec.fifo_cost 	= twodec (actCost);
		incfRec.act_cost 	= twodec (stdCost);

		if ((cc = abc_update (incf, &incfRec)))
			file_err (cc, incf, "DBUPDATE");

		cc = find_rec (incf, &incfRec, NEXT, "u");
	}
	abc_selfield (incf, "incf_seq_id");
}
/*---------------------------------------
| Update $amt recptd on pcms records	|
---------------------------------------*/
double	
UpdatePcms (
	int		closeJob)
{
	double	totalCost 	= 0.00,
			workCost	= 0.00,
			quantity	= 0.00;

	pcms_rec.hhwo_hash 	= pcwo_rec.hhwo_hash;
	pcms_rec.uniq_id 	= 0;
	cc = find_rec (pcms, &pcms_rec, GTEQ, "u");
	while (!cc && pcms_rec.hhwo_hash == pcwo_rec.hhwo_hash)
	{
		/*------------------------
		| Find item master file. |
		------------------------*/
		inmr2_rec.hhbr_hash = pcms_rec.mabr_hash;
		cc = find_rec (inmr2, &inmr2_rec, COMPARISON, "r");
		if (cc)
			inmr2_rec.outer_size = 1.00;

		/*----------------------------------------------------
		| Find item branch file at the manufacturing branch. | 
		----------------------------------------------------*/
		inei2Rec.hhbr_hash = pcms_rec.mabr_hash;
		strcpy (inei2Rec.est_no, pcwo_rec.br_no);
		cc = find_rec (inei, &inei2Rec, COMPARISON, "r");
		if (cc)
			inei2Rec.std_cost = 0.00;

		ProcessInum ();

		inei2Rec.std_cost /= (double) fourdec (cnvFct);
		quantity = pcms_rec.matl_wst_pc;
		quantity += 100.00;
		quantity /= 100.00;
		quantity *= pcms_rec.matl_qty;

		/*-------------------------
		| Standard material cost. |
		-------------------------*/
		workCost = out_cost (inei2Rec.std_cost, inmr2_rec.outer_size);
		workCost *= quantity;
		workCost = twodec (workCost);
		workCost = CENTS (workCost);

		workCost *= thisReceipt;
		workCost /= NDEC (pcwo_rec.prod_qty);
		workCost = no_dec (workCost);

		if (closeJob)
			pcms_rec.amt_recptd = pcms_rec.amt_issued;
		else
			pcms_rec.amt_recptd += workCost;

		totalCost += workCost;

		/*-----------------------------------
		| add sobg record for recalculation |
		| of material items committed qty   |
		-----------------------------------*/
		add_hash 
		 (
			pcwo_rec.co_no,  /* re-calc manufacturing warehouse */
			pcwo_rec.br_no,
			"RP",
			0,
			pcms_rec.mabr_hash,
			pcwo_rec.hhcc_hash,
			0L,
			 (double) 0
		);  /* decrease committed qty for item produced */

		cc = abc_update (pcms, &pcms_rec);
		if (cc)
			file_err (cc, pcms, "DBUPDATE");

		cc = find_rec (pcms, &pcms_rec, NEXT, "u");
	}
	abc_unlock (pcms);

	return (totalCost);
}

/*---------------------------------------
| Update $amt recptd on pcln records	|
---------------------------------------*/
double	
UpdatePcln (
	int		closeJob)
{
	double	totalCost 		= 0.00,
			overheadCost 	= 0.00,
			workCost 		= 0.00,
			actCost	 		= 0.00,
			actOvh   		= 0.00;
	long	tempTime 		= 0L,
			actTime  		= 0L;
	int		TYPE 			= 0;

	pcln_rec.hhwo_hash 	= pcwo_rec.hhwo_hash;
	pcln_rec.seq_no 	= 0;
	pcln_rec.line_no 	= 0;
	cc = find_rec (pcln, &pcln_rec, GTEQ, "u");
	while (!cc && pcln_rec.hhwo_hash == pcwo_rec.hhwo_hash)
	{
		if (closeJob)
		{
			actTime = 0L;
			pcrq_rec.hhwo_hash	= pcln_rec.hhwo_hash;
			pcrq_rec.seq_no		= pcln_rec.seq_no;
			pcrq_rec.line_no	= pcln_rec.line_no;
			cc = find_rec (pcrq, &pcrq_rec, GTEQ, "r");
			while (!cc && pcrq_rec.hhwo_hash	== pcln_rec.hhwo_hash &&
						  pcrq_rec.seq_no		== pcln_rec.seq_no &&
						  pcrq_rec.line_no		== pcln_rec.line_no)
			{
				actTime += pcrq_rec.act_setup;
				actTime += pcrq_rec.act_run;
				actTime += pcrq_rec.act_clean;

				cc = find_rec (pcrq, &pcrq_rec, NEXT, "r");
			}

			actCost = (pcln_rec.rate + pcln_rec.ovhd_var) * (double) actTime;
			actCost /= 60.0;
			actCost += pcln_rec.ovhd_fix;
			actCost *= pcln_rec.qty_rsrc;
			actOvh  = pcln_rec.ovhd_var * (double) actTime;
			actOvh  /= 60.0;
			actOvh  += pcln_rec.ovhd_fix;
			actOvh  *= pcln_rec.qty_rsrc;
		}
		tempTime = pcln_rec.setup;
		tempTime += pcln_rec.run;
		tempTime += pcln_rec.clean;

		workCost = overheadCost = 0.00;
		if (tempTime)
		{
			workCost = (pcln_rec.rate + pcln_rec.ovhd_var) * (double) tempTime;
			workCost /= 60.0;
			workCost += pcln_rec.ovhd_fix;
			workCost *= pcln_rec.qty_rsrc;
			overheadCost = pcln_rec.ovhd_var * (double) tempTime;
			overheadCost /= 60.0;
			overheadCost += pcln_rec.ovhd_fix;
			overheadCost *= pcln_rec.qty_rsrc;
		}

		workCost *= thisReceipt;
		workCost /= pcwo_rec.prod_qty;
		overheadCost *= thisReceipt;
		overheadCost /= pcwo_rec.prod_qty;

		if (closeJob)
		{
			actCost = no_dec (actCost);
			actOvh  = no_dec (actOvh);
			pcln_rec.amt_recptd = actCost;
			pcln_rec.ovh_recptd = actOvh;
		}
		else
		{
			workCost = no_dec (workCost);
			overheadCost = no_dec (overheadCost);
			pcln_rec.amt_recptd += workCost;
			pcln_rec.ovh_recptd += overheadCost;
		}
		totalCost += workCost;

		/*---------------------------------------------------------
		| Check resource type, and add to appropriate total cost. |
		---------------------------------------------------------*/
		rgrs_rec.hhrs_hash = pcln_rec.hhrs_hash;
		cc = find_rec (rgrs, &rgrs_rec, EQUAL, "r");
		if (!cc)
		{
			switch (rgrs_rec.type [0])
			{
			case	'L': /* Labour 	 */	TYPE = 0; break;
			case	'M': /* Machine  */	TYPE = 1; break;
			case	'Q': /* QC-Check */	TYPE = 2; break;
			case	'S': /* Special  */	TYPE = 3; break;
			case	'O': /* Other 	 */	TYPE = 4; break;
			} /* END OF SWITCH */

			/*------------------------------------------- 
			| read G/L accounts from the resource file. |
			-------------------------------------------*/
			glmrRec.hhmr_hash = rgrs_rec.dir_hash;
			cc = find_rec (glmr, &glmrRec, EQUAL, "r");
			if (!cc)
			{
				dirHash [TYPE] = glmrRec.hhmr_hash;
				strcpy (dirAcc [TYPE], glmrRec.acc_no);
			}
			
			glmrRec.hhmr_hash = rgrs_rec.fix_hash;
			cc = find_rec (glmr, &glmrRec, EQUAL, "r");
			if (!cc)
			{
				fixHash [TYPE] = glmrRec.hhmr_hash;
				strcpy (fixAcc [TYPE], glmrRec.acc_no);
			}
			glmrRec.hhmr_hash = rgrs_rec.mfg_dir_hash;
			cc = find_rec (glmr, &glmrRec, EQUAL, "r");
			if (!cc)
			{
				mfgDHash [TYPE] = glmrRec.hhmr_hash;
				strcpy (mfgDAcc [TYPE], glmrRec.acc_no);
			}
			glmrRec.hhmr_hash = rgrs_rec.mfg_fix_hash;
			cc = find_rec (glmr, &glmrRec, EQUAL, "r");
			if (!cc)
			{
				mfgFHash [TYPE] = glmrRec.hhmr_hash;
				strcpy (mfgFAcc [TYPE], glmrRec.acc_no);
			}
		}
	    cc = abc_update (pcln, &pcln_rec);
	    if (cc)
			file_err (cc, pcln, "DBUPDATE");

	    cc = find_rec (pcln, &pcln_rec, NEXT, "u");
	}
	abc_unlock (pcln);

	pcln_rec.hhwo_hash	= pcwo_rec.hhwo_hash + 1;
	pcln_rec.seq_no		= 0;
	pcln_rec.line_no	= 0;
	if (find_rec (pcln, &pcln_rec, GTEQ, "r"))
		find_rec (pcln, &pcln_rec, LAST, "r");
	else
		find_rec (pcln, &pcln_rec, PREVIOUS, "r");

	pcwc_rec.hhwc_hash = pcln_rec.hhwc_hash;
	cc = find_rec (pcwc, &pcwc_rec, EQUAL, "r");
	if (cc)
		file_err (cc, pcwc, DBFIND);

	return (totalCost);
}

/*==============================
| Print details of data input. |
==============================*/
void
PrintDetails (
	int		closeJob)
{
	double	value;

	if (closeJob)
		value = out_cost (actCost, inmr_rec.outer_size);
	else
		value = out_cost (standardCost, inmr_rec.outer_size);

	/*---------------------------------
	| Somes details have been printed |
	---------------------------------*/
	if (!popeOpen)
		OpenAudit ();

	popeOpen = TRUE;
	fprintf (pp, "|%16.16s",	inmr_rec.item_no);
	fprintf (pp, "|%-23.23s",	inmr_rec.description);
	fprintf (pp, "|%25.25s",	" ");
	fprintf (pp, "|%7.7s",		pcwo_rec.order_no);
	fprintf (pp, "|%10.10s",	pcwo_rec.batch_no);
	fprintf (pp, "|%10.2f", 	DOLLARS (closeJob ? actCost : standardCost));
	fprintf (pp, "|%9.1f",		outerSize);
	fprintf (pp, "|%14.6f",		qtyReceipt);
	fprintf (pp, "|%10.10s",	DateToString (receiptDate));
	fprintf (pp, "|%-10.10s",	" ");
	fprintf (pp, "|%11.2f|\n",	DOLLARS (qtyReceipt * value));
	fflush (pp);
	batchTotal 		+= (double) qtyReceipt * value;
	quantityTotal   += qtyReceipt;
}

/*=========================================================================
| Routine to open output pipe to standard print to provide an audit trail |
| of events. This also sends the output straight to the spooler.          |
=========================================================================*/
void
OpenAudit (
 void)
{
	if ((pp = popen ("pformat","w")) == 0)
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);

	fprintf (pp, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (pp, ".SO\n");
	fprintf (pp, ".LP%d\n",printerNumber);
	fprintf (pp, ".PI12\n");
	fprintf (pp, ".12\n");
	fprintf (pp, ".L158\n");
	fprintf (pp, ".E%s\n", ML ("AUTOMATIC PRODUCTION RECEIPTS"));

	fprintf (pp, ".B1\n");
	fprintf (pp, ".E%s as at %24.24s\n",clip (comm_rec.co_short),SystemTime ());
	fprintf (pp, ".B2\n");

	fprintf (pp, ".E %s %s : Warehouse %s \n",
				ML ("MANUFACTURING BRANCH"),
				clip (comm_rec.est_name),
				clip (comm_rec.cc_name));

	fprintf (pp, ".R==================");
	fprintf (pp, "==========================");
	fprintf (pp, "==========================");
	fprintf (pp, "========");
	fprintf (pp, "===========");
	fprintf (pp, "===========");
	fprintf (pp, "==========");
	fprintf (pp, "===============");
	fprintf (pp, "=========");
	fprintf (pp, "===========");
	fprintf (pp, "============\n");

	fprintf (pp, "=================");
	fprintf (pp, "==========================");
	fprintf (pp, "==========================");
	fprintf (pp, "========");
	fprintf (pp, "===========");
	fprintf (pp, "===========");
	fprintf (pp, "==========");
	fprintf (pp, "===============");
	fprintf (pp, "=========");
	fprintf (pp, "===========");
	fprintf (pp, "=============\n");

	fprintf (pp, "|  ITEM NUMBER   ");
	fprintf (pp, "|   ITEM  DESCRIPTION   ");
	fprintf (pp, "|       REFERENCE         ");
	fprintf (pp, "| ORDER ");
	fprintf (pp, "|  BATCH   ");
	fprintf (pp, "|   @COST  ");
	fprintf (pp, "| PRICING ");
	fprintf (pp, "|   QUANTITY   ");
	fprintf (pp, "|   DATE   ");
	fprintf (pp, "| LOCATION ");
	fprintf (pp, "|  EXTENDED |\n");

	fprintf (pp, "|                ");
	fprintf (pp, "|                       ");
	fprintf (pp, "|                         ");
	fprintf (pp, "|NUMBER ");
	fprintf (pp, "|  NUMBER  ");
	fprintf (pp, "| RECEIPT  ");
	fprintf (pp, "|  CONV.  ");
	fprintf (pp, "|              ");
	fprintf (pp, "|          ");
	fprintf (pp, "|          ");
	fprintf (pp, "|  VALUE.   |\n");

	fprintf (pp, "|----------------");
	fprintf (pp, "+-----------------------");
	fprintf (pp, "+-------------------------");
	fprintf (pp, "+-------");
	fprintf (pp, "+----------");
	fprintf (pp, "+----------");
	fprintf (pp, "+---------");
	fprintf (pp, "+--------------");
	fprintf (pp, "+----------");
	fprintf (pp, "+----------");
	fprintf (pp, "+-----------|\n");
}

/*===============================================
| Routine to close the audit trail output file. |
===============================================*/
void
CloseAudit (
 void)
{
	fprintf (pp, "|----------------");
	fprintf (pp, "+-----------------------");
	fprintf (pp, "+-------------------------");
	fprintf (pp, "+-------");
	fprintf (pp, "+----------");
	fprintf (pp, "+----------");
	fprintf (pp, "+---------");
	fprintf (pp, "+--------------");
	fprintf (pp, "+----------");
	fprintf (pp, "+----------");
	fprintf (pp, "+-----------|\n");

	fprintf (pp, "| BATCH TOTAL    ");
	fprintf (pp, "|                       ");
	fprintf (pp, "|                         ");
	fprintf (pp, "|       ");
	fprintf (pp, "|          ");
	fprintf (pp, "|          ");
	fprintf (pp, "|         ");
	fprintf (pp, "|%14.6f", quantityTotal);
	fprintf (pp, "|          ");
	fprintf (pp, "|          ");
	fprintf (pp, "|%11.2f|\n", DOLLARS (batchTotal));

	fprintf (pp, ".EOF\n");
	pclose (pp);
}
/*===============================
| Process control Accounts	|
===============================*/
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
		"COSTSALE M",
		" ",
		category
	);

	strcpy (glmrRec.co_no,  comm_rec.co_no);
	sprintf (glmrRec.acc_no, "%-*.*s", MAXLEVEL,MAXLEVEL,GL_Account);
	cc = find_rec (glmr, &glmrRec, EQUAL, "r");
	if (cc)
		file_err (cc, glmr, DBFIND);

	strcpy (creditAccount, glmrRec.acc_no);
	creditHash = glmrRec.hhmr_hash;
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
	if (amount == 0.00)
		return;

	amount	=	no_dec (amount);

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
	pcgl_rec.amount 		= amount;
	pcgl_rec.loc_amount 	= amount;
	pcgl_rec.exch_rate		=	1.00;
	strcpy (pcgl_rec.currency, localCurrency);
	strcpy (pcgl_rec.acc_no, accountNumber);
	pcgl_rec.hhgl_hash 		= hhmrHash;

	strcpy (pcgl_rec.jnl_type, type);
	cc = abc_add (pcgl, &pcgl_rec);
	if (cc)
		file_err (cc, pcgl, "DBADD");
}

/*==========================
| Process the sales order. |
==========================*/
void
ProcessSalesOrder (
	long	hhslHash,
	char *  processSO,
	float   qtyReceipt)
{
	soln_rec.hhsl_hash	=	hhslHash;
	cc = find_rec (soln, &soln_rec, COMPARISON, "u");
	if (cc)
	{
		abc_unlock (soln);
		return;
	}

	sohr_rec.hhso_hash = soln_rec.hhso_hash;
	cc = find_rec (sohr, &sohr_rec, COMPARISON, "u");
	if (cc)
	{
		abc_unlock (sohr);
		return;
	}

	if (qtyReceipt < soln_rec.qty_bord)
	{
		/* Quantity receipt is less than back ordered */
		/* Put the difference in the soln_back_order  */
		soln_rec.qty_order += qtyReceipt;
		soln_rec.qty_bord -= qtyReceipt;
	}
	else if (qtyReceipt >= 	soln_rec.qty_bord)
	{
		/* Despatch the whole quantity */
		soln_rec.qty_order += soln_rec.qty_bord;
		soln_rec.qty_bord= 0;	
	}
	
	/* Activity type is 'P' */
	if (processSO [0] == 'P')
	{
		/* Set SO line details status into Manual Release*/
		strcpy (soln_rec.status,"M");
		strcpy (soln_rec.stat_flag, "M");
		
		cc = abc_update (soln, &soln_rec);
		if (cc)
			file_err (cc, soln, "DBUPDATE");
		
		/* Set SO header status into Manual Release*/
		strcpy (sohr_rec.status, "M");
		strcpy (sohr_rec.stat_flag, "M");

		cc = abc_update (sohr, &sohr_rec);
		if (cc)
			file_err (cc, sohr, "DBUPDATE");
	}
	else 
	{
		/* Activity type is 'F' */
		
		strcpy (soln_rec.status, 	 (envVarConOrders) ? "M" : "R");
		strcpy (soln_rec.stat_flag, (envVarConOrders) ? "M" : "R");
		
		cc = abc_update (soln, &soln_rec);
		if (cc)
			file_err (cc, soln, "DBUPDATE");
			
		strcpy (sohr_rec.status, 	 (envVarConOrders) ? "M" : "R");
		strcpy (sohr_rec.stat_flag, (envVarConOrders) ? "M" : "R");
		cc = abc_update (sohr, &sohr_rec);
		if (cc)
			file_err (cc, sohr, "DBUPDATE");
		
		/* if staus is Released set SO to P/S */
		if (soln_rec.status [0] == 'R')
		{
			add_hash 
			 (
				"  ",
				"  ",
				 (printerNumber) ? "PA" : "PC",
				printerNumber,
				soln_rec.hhso_hash,
				0L,
				0L,
				 (double) 0
			);
		}
	}
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
		XML_Error
		(
			"pc_autorec",
			errMess [0],
			errMess [1],
			" ",
			" "
		);
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

		XML_Error
		(
			"pc_autorec",
			errMess [0],
			errMess [1],
			errMess [2],
			" "
		);
		return (errCode);
	}
	if (ERR_WOR_NO_INCC == errCode)
	{
		strcpy (errMess [0], "Warehouse Item record not be found (incc)");
		sprintf (errMess [1], "incc_hhbr_hash [%ld] incc_hhcc_hash [%ld]",
							incc_rec.hhbr_hash, incc_rec.hhcc_hash);

		fprintf (stderr, "%s\n", errMess [0]);
		fprintf (stderr, "%s\n", errMess [1]);

		XML_Error
		(
			"pc_autorec",
			errMess [0],
			errMess [1],
			" ",
			" "
		);
		return (errCode);
	}
	if (ERR_WOR_GLI == errCode)
	{
		strcpy (errMess [0], "General Ledger interface not defined.");
		fprintf (stderr, "%s\n", errMess [0]);
		XML_Error
		(
			"pc_autorec",
			errMess [0],
			" ",
			" ",
			" "
		);
		return (errCode);
	}
	if (ERR_WOR_TIME == errCode)
	{
		strcpy (errMess [0], "Time sheets information has not been input");
		fprintf (stderr, "%s\n", errMess [0]);
		XML_Error
		(
			"pc_autorec",
			errMess [0],
			" ",
			" ",
			" "
		);
		return (errCode);
	}
	if (ERR_WOR_WOFFCODE == errCode)
	{
		strcpy (errMess [0], "No write-off code found or defined (exwo)");
		fprintf (stderr, "%s\n", errMess [0]);
		XML_Error
		(
			"pc_autorec",
			errMess [0],
			" ",
			" ",
			" "
		);
		return (errCode);
	}
	return (errCode);
}

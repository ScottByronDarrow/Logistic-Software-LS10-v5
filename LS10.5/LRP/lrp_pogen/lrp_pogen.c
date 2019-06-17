/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: lrp_pogen.c,v 5.4 2002/11/25 03:16:38 scott Exp $
|  Program Name  : 
|  Program Desc  : ( Generate Purchase Orders.                    )   |
|---------------------------------------------------------------------|
|  Date Written  : (18/08/1998)    | Author      : Scott B Darrow     |
|---------------------------------------------------------------------|
| $Log: lrp_pogen.c,v $
| Revision 5.4  2002/11/25 03:16:38  scott
| Updated to use chk_env instead of get_env when applicable.
|
| Revision 5.3  2002/08/14 06:52:04  scott
| Updated for fix Linux warning
|
| Revision 5.2  2001/08/09 09:29:53  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:27:38  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:07:29  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.1  2001/03/27 00:18:13  scott
| Updated to ignore works orders when routing is not defines rather than cause
| an unrecoverable error.
|
| Revision 4.0  2001/03/09 02:28:38  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.3  2001/01/25 10:07:56  scott
| Updated after testing
|
| Revision 3.2  2001/01/25 07:36:53  scott
| Updated to remove names pipe stuff created in NZ and placed generation of
| works order within program
|
| Revision 3.1  2000/11/20 07:39:03  scott
| New features related to 3PL environment
| New features related to Number Plates
| All covered in release 3 notes
|
| Revision 3.0  2000/10/10 12:15:31  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.1  2000/09/04 08:54:32  scott
| Updated to ensure program compiles with LS/10 GUI.
|
| Revision 2.0  2000/07/15 08:58:42  gerry
| Forced Revision No. Start 2.0 Rel-15072000
|
| Revision 1.32  2000/07/06 22:34:03  johno
| Corrected std unit cost in report
|
| Revision 1.31  2000/07/04 06:48:29  johno
| Change execl to execlp
|
| Revision 1.30  2000/07/03 21:16:47  johno
| Added functionality for pc_wogen to allow creation of works orders by
| piping parameters to an invokation of pc_womaint.
|
| Revision 1.29  2000/06/15 02:38:59  scott
| Updated to change FindSumr() local routine to IntFindSumr to ensure no
| conflict will exist with new routine FindSumr() that is about to be
| introduced.
|
| Revision 1.28  2000/05/31 03:32:02  scott
| Had to re-name FindInmr() to IntFindInmr() as conflicted with new routine.
|
| Revision 1.27  2000/05/17 07:40:01  scott
| S/C USL-16222 / LSDI-2763
| Updated to allow purchase orders to be generated as A(pproved) or U(napproved)
| based on environment PO_APP_FLAG
|
| Revision 1.26  2000/05/10 06:11:06  jinno
| SC#2713 - Modified PO number creation.
|
| Revision 1.25  2000/05/08 06:24:23  marnie
| SC2876 - Modified regarding the PO number generation.
|
| Revision 1.24  2000/05/05 01:51:13  nz
| Removed extra space in char* PROG_VERSION
|
| Revision 1.23  2000/05/04 11:45:47  marnie
| SC2876 - LSANZ16295 - Modified to correct the checking of duplicate PO when PO_NU_GEN is by Company level.
|
| Revision 1.22  2000/04/27 10:17:19  scott
| S/C USL-16235 / LSDI-2783
| Updated to use average cost when supplier fob cost is zero and environment
| PO_COST_CALC is set.
|
| Revision 1.21  2000/02/29 00:14:08  cam
| Fixed redeclaration of pocrRecord.
|
| Revision 1.20  2000/02/18 06:09:12  scott
| Updated as found some out_cost issues while testing previous change.
|
| Revision 1.19  2000/02/18 03:56:07  scott
| S/C LSANZ-16009  / LSDI-2532
| Updated to make use of environment PO_UOM_DEFAULT created for purchase order entry.
| Additional updated for report to show in base UOM only.
|
| Revision 1.18  2000/02/17 23:45:05  scott
| S/C LSANZ-16009  / LSDI-2532
| Updated to make use of environment PO_UOM_DEFAULT created for purchase order entry.
|
| Revision 1.17  1999/12/06 01:34:19  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.16  1999/11/04 06:19:48  scott
| Updated as poln_reg_pc and discounts a-c not being updated.
|
| Revision 1.15  1999/10/27 07:33:00  scott
| Updated for -Wall warnings + modifications for ASL on percentage error.
|
| Revision 1.14  1999/10/11 22:38:40  scott
| Updated for Date Routines
|
| Revision 1.13  1999/09/29 10:10:50  scott
| Updated to be consistant on function names.
|
| Revision 1.12  1999/09/17 07:26:40  scott
| Updated for datejul, ctime, ttod, copyright, juldate
| Updated to DateToString, SystemTime, TimeHHMM, StringToDate etc.
|
| Revision 1.11  1999/09/16 09:20:44  scott
| Updated from Ansi Project
|
| Revision 1.10  1999/06/17 07:13:46  scott
| Updated for redefine of supplier price and discount routines.
|
| Revision 1.9  1999/06/15 07:27:04  scott
| Updated to add log file.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: lrp_pogen.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/LRP/lrp_pogen/lrp_pogen.c,v 5.4 2002/11/25 03:16:38 scott Exp $";

#include 	<pslscr.h>	
#include 	<proc_sobg.h>
#include 	<twodec.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<ml_lrp_mess.h>
#include	"schema"

#define	FGN_CURR	 (strcmp (sumr_rec.curr_code, envCurrCode))

	struct	commRecord	comm_rec;
	struct	comrRecord	comr_rec;
	struct	ccmrRecord	ccmr_rec;
	struct	esmrRecord	esmr_rec;
	struct	exsiRecord	exsi_rec;
	struct	ffwkRecord	ffwk_rec;
	struct	inisRecord	inis_rec;
	struct	inldRecord	inld_rec;
	struct	inmrRecord	inmr_rec;
	struct	pocfRecord	pocf_rec;
	struct	pocrRecord	pocr_rec;
	struct	podtRecord	podt_rec;
	struct	pohrRecord	pohr_rec;
	struct	pohrRecord	pohr2_rec;
	struct	polhRecord	polh_rec;
	struct	polnRecord	poln_rec;
	struct	sumrRecord	sumr_rec;
	struct	inumRecord	inum_rec;
	struct	inccRecord	incc_rec;
	struct 	bmmsRecord	bmms_rec;
	struct 	bmmsRecord	bmms2_rec;
	struct 	cumrRecord	cumr_rec;
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
	struct 	inspRecord	insp_rec;
	struct 	sudsRecord	suds_rec;

	char	*bmms2	= "bmms2",
			*inmr2	= "inmr2",
			*pcwo2	= "pcwo2",
			*pcwo3	= "pcwo3",
			*pohr2	= "pohr2";


	int		printerNumber 	= 1,
			IkeaPoNumbers 	= 0,
			quickPo 		= FALSE,
			envPoUomDef 	= 0,
			envPoCostCalc 	= 1,
			envPoNumGen		= 0,
			envPoMaxLines	= 0,
			envPoAppFlag	= 0,
			fd[2],
			worksOrderGen	= 0;	/* if TRUE, indicates generating */
									/* works orders, not purchase orders */

	void 	CheckIncc 				(long, long);
	char	envCurrCode [4],
			envPoLocal  [16],
			envPoForeign [16],
			filename [15];

	long	hhpoHash		= 0L,
			hhwoHash		= 0L,
			hhccHash		= 0L,
			hhlcHash		= 0L,
			lsystemDate		= 0L;

	int		bomAlternate	=	0,
			rtgAlternate	=	0;

	double	exchangeRate	= 0.00,
			licenceRate		= 0.00;

	float	qtyReqCalc 	= 0.00,
			qtyPrdCalc 	= 0.00;

	FILE	*fin,
			*fout;

	char	*currentUser;
	char	envSupOrdRound[2];

#include	<SupPrice.h>

/*===========================
| Local Function Prototype. |
===========================*/
double 	CalculateDuty 		(void);
double 	CalculateFreight 	(void);
double 	CalculateLicence 	(void);
float 	GetLeadDate 		(long, long);
static 	float RoundMultiple 	(float, char *, float, float);
int	 	FindExsi 			(int);
int 	AddPoln 			(int);
int 	CheckPohr 			(char *);
int 	CheckPohr_c			(char *);
int 	CheckPcwo 			(long);
int 	FindInis 			(long, long);
int 	IntFindInmr 		(long);
void	shutdown_prog 		(void);
void 	AddPohr 			(void);
void 	AddWorksOrder		(void);
void 	CloseDB 			(void);
void 	FindPoNumber 		(void);
void 	IntFindSumr 		(long);
void 	HeadingPrint 		(void);
void 	LogError 			(int);
void 	OpenDB 				(void);
void 	Process 			(void);
void 	ReadMisc 			(void);
void	WO_Create 			(void);
void 	WO_UpdatePcwo 		(void);
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
	char	*sptr;

	currentUser = getenv ("LOGNAME");

	sptr = chk_env ("SUP_ORD_ROUND");
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

	if (argc < 3)
	{
		print_at (0,0, "Usage %s <LPNO> <FILENAME>", argv [0]);
        return (EXIT_FAILURE);
	}

	if (argc > 3)
		quickPo = TRUE;

	printerNumber = atoi (argv [1]);
	sprintf (filename,"%-14.14s",argv [2]);

	/*-------------------------------------------------------------
	| Determine whether we are generating purchase or work orders |
	-------------------------------------------------------------*/
	sptr = strrchr(argv[0], '/');
	if (sptr == NULL)
		sptr = argv[0];
	else
		sptr ++;

	if (!strncmp(sptr, "lrp_wogen", 9))
		worksOrderGen = TRUE;
	else
		worksOrderGen = FALSE;

	/*---------------------------------------
	| Ascertain what is the local currency.	|
	---------------------------------------*/
	sptr = chk_env ("CURR_CODE");
	sprintf (envCurrCode, "%-3.3s", (sptr) ? sptr : "   ");

	/*------------------------------------------------------------
	| Purchase order prefix for local purchase orders generated. |
	------------------------------------------------------------*/
	sptr = chk_env ("PO_LOCAL");
	sprintf (envPoLocal, "%-15.15s", (sptr) ? sptr : "  ");
	clip (envPoLocal);

	/*--------------------------------------------------------------
	| Purchase order prefix for foreign purchase orders generated. |
	--------------------------------------------------------------*/
	sptr = chk_env ("PO_FOREIGN");
	sprintf (envPoForeign, "%-15.15s", (sptr) ? sptr : "  ");
	clip (envPoForeign);

	/*--------------------------------------------
	| Check for purchase order approval details. |
	--------------------------------------------*/
	sptr = chk_env ("PO_APP_FLAG");
	envPoAppFlag = (sptr == (char *)0) ? 0 : atoi (sptr);

	/*-------------------------------------------------------
	| Purchase Order Cost Calc. 1 if Avgerage can be used.  |
	-------------------------------------------------------*/
	sptr = chk_env ("PO_COST_CALC");
	envPoCostCalc = (sptr == (char *)0) ? 1 : atoi (sptr);

	sptr = chk_env ("IKEA_PO_NUMBERS");
	IkeaPoNumbers = (sptr == (char *) 0) ? 0 : atoi (sptr);

	/*---------------------------------------
	| Check limit on maximum lines per P/O.	|
	---------------------------------------*/
	sptr = chk_env ("PO_MAX_LINES");
	envPoMaxLines = (sptr == (char *) 0) ? 0 : atoi (sptr);

	/*------------------------
	| Check what UOM to use. |
	------------------------*/
	sptr = chk_env ("PO_UOM_DEFAULT");
	envPoUomDef = (sptr == (char *) 0) ? 0 : atoi (sptr);

	/*-------------------------------------------------------
	| Purchase Order number is Company or branch generated. |
	-------------------------------------------------------*/
	sptr = chk_env ("PO_NUM_GEN");
	envPoNumGen = (sptr == (char *)0) ? 1 : atoi (sptr);

	lsystemDate = TodaysDate ();

	init_scr ();

	OpenDB ();

	ReadMisc ();

	sprintf (err_str,"Generate %s",(worksOrderGen) ? "Works Order" : "Purchase Order");
	dsp_screen (ML (err_str), comm_rec.co_no, comm_rec.co_name);

	if ((fout = popen ("pformat","w")) == NULL)
		sys_err ("Error in pformat During (POPEN)",errno,PNAME);
	
	HeadingPrint ();

	Process ();

	fprintf (fout,".EOF\n");
	pclose (fout);

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
	abc_dbopen ("data");

	open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (inum, inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	open_rec (inei, inei_list, INEI_NO_FIELDS, "inei_id_no");
	open_rec (exsi, exsi_list, EXSI_NO_FIELDS, "exsi_id_no");
	open_rec (ffwk, ffwk_list, FFWK_NO_FIELDS, "ffwk_id_no_2");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");


	if (!worksOrderGen)
	{
		abc_alias (pohr2, pohr);

		open_rec (inis, inis_list, INIS_NO_FIELDS, "inis_id_no");
		open_rec (inld, inld_list, INLD_NO_FIELDS, "inld_id_no");
		open_rec (pohr2,pohr_list, POHR_NO_FIELDS, "pohr_id_no2");
		open_rec (pocf, pocf_list, POCF_NO_FIELDS, "pocf_id_no");
		open_rec (pocr, pocr_list, POCR_NO_FIELDS, "pocr_id_no");
		open_rec (podt, podt_list, PODT_NO_FIELDS, "podt_id_no");
		open_rec (pohr, pohr_list, POHR_NO_FIELDS, "pohr_hhpo_hash");
		open_rec (polh, polh_list, POLH_NO_FIELDS, "polh_id_no");
		open_rec (poln, poln_list, POLN_NO_FIELDS, "poln_id_no");
		open_rec (sumr, sumr_list, SUMR_NO_FIELDS, "sumr_hhsu_hash");
		open_rec (suds, suds_list, SUDS_NO_FIELDS, "suds_id_no");
		open_rec (insp, insp_list, INSP_NO_FIELDS, "insp_id_no");
	}
	else
	{
		abc_alias (bmms2, bmms);
		abc_alias (inmr2, inmr);
		abc_alias (pcwo2, pcwo);
		abc_alias (pcwo3, pcwo);

		open_rec (bmms,  bmms_list, BMMS_NO_FIELDS, "bmms_id_no");
		open_rec (bmms2, bmms_list, BMMS_NO_FIELDS, "bmms_id_no_2");
		open_rec (cumr,  cumr_list, CUMR_NO_FIELDS, "cumr_id_no");
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
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (void)
{
	abc_fclose (esmr);
	abc_fclose (inum);
	abc_fclose (inei);
	abc_fclose (exsi);
	abc_fclose (ffwk);
	abc_fclose (incc);
	abc_fclose (inmr);
	abc_fclose (ccmr);

	if (!worksOrderGen)
	{
		abc_fclose (inis);
		abc_fclose (inld);
		abc_fclose (pohr2);
		abc_fclose (pocf);
		abc_fclose (pocr);
		abc_fclose (podt);
		abc_fclose (pohr);
		abc_fclose (polh);
		abc_fclose (poln);
		abc_fclose (sumr);
		abc_fclose (suds);
		abc_fclose (insp);
	}
	else
	{
		abc_fclose (bmms);
		abc_fclose (bmms2);
		abc_fclose (cumr);
		abc_fclose (inmr2);
		abc_fclose (pcbp);
		abc_fclose (pcln);
		abc_fclose (pcms);
		abc_fclose (pcwo);
		abc_fclose (pcwo2);
		abc_fclose (rgbp);
		abc_fclose (rghr);
		abc_fclose (rgln);
	}
	abc_dbclose ("data");
}

/*============================================
| Get common info from commom database file. |
============================================*/
void
ReadMisc (void)
{
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	strcpy (ccmr_rec.co_no,	comm_rec.co_no);
	strcpy (ccmr_rec.est_no,comm_rec.est_no);
	strcpy (ccmr_rec.cc_no,	comm_rec.cc_no);
	cc = find_rec (ccmr,&ccmr_rec,COMPARISON,"r");
	if (cc)
    {
		file_err (cc, "ccmr", "DBFIND");
    }

	hhccHash = ccmr_rec.hhcc_hash;

	open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");
	strcpy (comr_rec.co_no,comm_rec.co_no);
	cc = find_rec (comr,&comr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, "comr", "DBFIND");
}

/*===========================
| Report heading for Audit. |
===========================*/
void
HeadingPrint (void)
{
	fprintf (fout, ".START%s\n",DateToString (comm_rec.inv_date));
	fprintf (fout, ".LP%d\n",printerNumber);
	fprintf (fout, ".SO\n");

	fprintf (fout, ".14\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L150\n");
	fprintf (fout, ".B1\n");
	if (worksOrderGen)
		fprintf (fout, ".ELRP - WORK ORDER GENERATION REPORT\n");
	else
		fprintf (fout, ".ELRP - PURCHASE ORDER GENERATION REPORT\n");
	fprintf (fout, ".B1\n");
	fprintf (fout, ".E%s\n", clip (comm_rec.co_name));
	fprintf (fout, ".B1\n");
	fprintf (fout, ".E%s\n", clip (comm_rec.est_name));
	fprintf (fout, ".B1\n");
	fprintf (fout, ".EAS AT %-24.24s\n",SystemTime ());

	fprintf (fout,".R===================");
	fprintf (fout,"===========================================");
	fprintf (fout,"===================");
	fprintf (fout,"=============");
	fprintf (fout,"======");
	fprintf (fout,"===============");
	if (!worksOrderGen)
		fprintf (fout,"===============");
	fprintf (fout,"============\n");

	fprintf (fout,"===================");
	fprintf (fout,"===========================================");
	fprintf (fout,"===================");
	fprintf (fout,"=============");
	fprintf (fout,"======");
	fprintf (fout,"===============");
	if (!worksOrderGen)
		fprintf (fout,"===============");
	fprintf (fout,"============\n");

	fprintf (fout,"|   PART NUMBER.   ");
	if (worksOrderGen)
		fprintf (fout,"| WORKS ORDER NO   ");
	else
		fprintf (fout,"| SUPPLIER PART NO ");
	fprintf (fout,"|   PART DESCRIPTION                       ");
	fprintf (fout,"| ORDER QTY. ");
	fprintf (fout,"| UOM ");
	if (worksOrderGen)
	{
		fprintf (fout,"| STD UNIT COST");
	}
	else
	{
		fprintf (fout,"| FOB UNIT COST");
		fprintf (fout,"| LANDED U COST");
	}
	fprintf (fout,"| DUE DATE |\n");

	fprintf (fout,"|------------------");
	fprintf (fout,"|------------------");
	fprintf (fout,"|------------------------------------------");
	fprintf (fout,"|------------");
	fprintf (fout,"|-----");
	fprintf (fout,"|--------------");
	if (!worksOrderGen)
		fprintf (fout,"|--------------");
	fprintf (fout,"|----------|\n");

	fflush (fout);
}

/*==========================
| Main processing routine. |
==========================*/
void
Process (void)
{
	long	hhsuHash 	=	0L;
	int		lineNumber	=	0;
	int		purchase_order;

	ffwk_rec.hhcc_hash = hhccHash;
	strcpy (ffwk_rec.filename,filename);
	strcpy (ffwk_rec.crd_no,"      ");
	sprintf (ffwk_rec.sort,"%-34.34s", " ");
	
	hhsuHash = ffwk_rec.hhsu_hash;
	cc = find_rec (ffwk,&ffwk_rec,GTEQ,"u");
	while (!cc && 
	       ffwk_rec.hhcc_hash == hhccHash && 
	       !strcmp (ffwk_rec.filename, filename))
	{
		/*---------------------------------------
		| If Purchase Order Already Generated	|
		---------------------------------------*/
		if 
		(
			(!worksOrderGen && ffwk_rec.hhpo_hash != 0L) ||
			(worksOrderGen && ffwk_rec.hhwo_hash != 0L)  ||
			ffwk_rec.stat_flag [0] != 'U'
		)
		{
			abc_unlock (ffwk);
			cc = find_rec (ffwk,&ffwk_rec,NEXT,"u");
			continue;
		}
		if (ffwk_rec.order_qty <= 0.00)
		{
			abc_unlock (ffwk);
			cc = find_rec (ffwk,&ffwk_rec,NEXT,"u");
			continue;
		}

		/*---------------------------------------------------------
		| If pogen skip Manufacturing Work Orders otherwise       |
		| it is worksOrderGen: skip non Manufacturing work orders |
		---------------------------------------------------------*/
		cc = IntFindInmr(ffwk_rec.hhbr_hash);
		if (cc)
		{
			abc_unlock (ffwk);
			cc = find_rec (ffwk, &ffwk_rec, NEXT, "u");
			continue;
		}
		purchase_order = (inmr_rec.source [0] == 'M') ? FALSE : TRUE;
		if (worksOrderGen == purchase_order)
		{
			abc_unlock (ffwk);
			cc = find_rec (ffwk,&ffwk_rec,NEXT,"u");
			continue;
		}
	
		/*-------------------------------------------
		| New supplier or Purchase Order too long	|
		-------------------------------------------*/
		if (!worksOrderGen && (
				hhsuHash != ffwk_rec.hhsu_hash || 
				(envPoMaxLines && lineNumber >= envPoMaxLines)) )
		{
			hhsuHash = ffwk_rec.hhsu_hash;
			IntFindSumr (ffwk_rec.hhsu_hash);
			/*-----------------------------------
			| Find next purchase order number	|
			-----------------------------------*/
			FindPoNumber ();
			lineNumber = 0;
			AddPohr ();
		}

		if (worksOrderGen)
		{
			/*---------------------------------------------
			| Add works order(s) incrementing lineNumber. |
			---------------------------------------------*/
			AddWorksOrder ();
		}
		else
		{
			if (AddPoln (lineNumber++))
			{
				lineNumber--;
				cc = find_rec (ffwk,&ffwk_rec,NEXT,"u");
				continue;
			}
		}
		if (!worksOrderGen)
			recalc_sobg ();

		/*-------------------------
		| Mark ffwk as processed. |
		-------------------------*/
		if (worksOrderGen)
			ffwk_rec.hhwo_hash = hhwoHash;
		else
			ffwk_rec.hhpo_hash = hhpoHash;

		cc = abc_update (ffwk,&ffwk_rec);
		if (cc)
			file_err (cc, "ffwk", "DBUPDATE");

		cc = find_rec (ffwk,&ffwk_rec,NEXT,"u");
	}
	abc_unlock (ffwk);
}

/*=======================
| Find Supplier record. |
=======================*/
void
IntFindSumr (
	long   hhsuHash)
{
	sumr_rec.hhsu_hash	=	hhsuHash;
	cc = find_rec (sumr, &sumr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "sumr", "DBFIND");
}

/*===================
| Find Item record. |
===================*/
int
IntFindInmr (
	long   hhbrHash)
{
	inmr_rec.hhbr_hash	=	hhbrHash;
	return (find_rec (inmr, &inmr_rec, COMPARISON, "r"));
}

/*=================================
| Find Inventory Supplier record. |
=================================*/
int
FindInis (
	long   hhsuHash,
	long   hhbrHash)
{
	int		supplierReturnCode	=	0;

	/*-----------------------------------------------
	| Find inventory supplier record for warehouse. |
	-----------------------------------------------*/
	inis_rec.hhsu_hash	=	hhsuHash;
	inis_rec.hhbr_hash	=	hhbrHash;
	strcpy (inis_rec.co_no, comm_rec.co_no);
	strcpy (inis_rec.br_no, comm_rec.est_no);
	strcpy (inis_rec.wh_no, comm_rec.cc_no);
	supplierReturnCode = find_rec (inis ,&inis_rec ,COMPARISON ,"r");
	if (supplierReturnCode)
	{
		/*--------------------------------------------
		| Find inventory supplier record for Branch. |
		--------------------------------------------*/
		strcpy (inis_rec.wh_no, "  ");
		supplierReturnCode = find_rec (inis ,&inis_rec ,COMPARISON ,"r");
	}
	if (supplierReturnCode)
	{
		/*-----------------------------------------------
		| Find inventory supplier record for Warehouse. |
		-----------------------------------------------*/
		strcpy (inis_rec.br_no, "  ");
		strcpy (inis_rec.wh_no, "  ");
		supplierReturnCode = find_rec (inis ,&inis_rec ,COMPARISON ,"r");
	}
	if (!supplierReturnCode && inis_rec.lead_time == 0.00)
		inis_rec.lead_time = GetLeadDate (inis_rec.hhis_hash,comm_rec.inv_date);
	return (supplierReturnCode);
}

/*===============================================
| Get the number of weeks between 'date' & the	|
| next available inld_sup_date.			        |
| Return 0 if none found.			            |
===============================================*/
float	
GetLeadDate (
	long   hhisHash,
	long   orderDate)
{
	float	days;

	inld_rec.hhis_hash	=	hhisHash;
	inld_rec.ord_date	=	orderDate;

	cc = find_rec (inld, &inld_rec, GTEQ, "r");
	if (cc)
		return ((float) 0.00);

	days = inld_rec.sup_date - orderDate;
	return ((float) days);
}

/*-------------------------------------------------
| Note: AddPohr is not called for works orders    |
-------------------------------------------------*/
void
AddPohr (void)
{
	static	int		initDone;

	/*-----------------------
	| Read Currency File	|
	-----------------------*/
	strcpy (pocr_rec.co_no,comm_rec.co_no);
	strcpy (pocr_rec.code,sumr_rec.curr_code);
	cc = find_rec (pocr,&pocr_rec,COMPARISON,"r");
	exchangeRate = (cc) ? 1.00 : pocr_rec.ex1_factor;
	if (pocr_rec.ex1_factor == 0.00)
		pocr_rec.ex1_factor = 1.00;

	strcpy (pohr_rec.co_no,comm_rec.co_no);
	strcpy (pohr_rec.br_no,comm_rec.est_no);
	strcpy (pohr_rec.type,"O");
	strcpy (pohr_rec.ship_method, sumr_rec.ship_method);
	strcpy (pohr_rec.drop_ship, "N");
	strcpy (pohr_rec.sup_type, (FGN_CURR) ? "F" : "L");
	pohr_rec.hhsu_hash = ffwk_rec.hhsu_hash;
	pohr_rec.date_raised = comm_rec.inv_date;
	pohr_rec.due_date 	= 0L;
	pohr_rec.conf_date 	= 0L;
	sprintf (pohr_rec.contact,"%-40.40s",sumr_rec.cont_name);

	cc = FindExsi (sumr_rec.sic1);
	sprintf (pohr_rec.delin1,"%-60.60s",(cc) ? " " : exsi_rec.inst_text);

	cc = FindExsi (sumr_rec.sic2);
	sprintf (pohr_rec.delin2,"%-60.60s",(cc) ? " " : exsi_rec.inst_text);

	cc = FindExsi (sumr_rec.sic3);
	sprintf (pohr_rec.delin3,"%-60.60s",(cc) ? " " : exsi_rec.inst_text);
	
	cc = FindExsi (comr_rec.po_sic1);
	sprintf (pohr_rec.stdin1,"%-60.60s",(cc) ? " " : exsi_rec.inst_text);

	cc = FindExsi (comr_rec.po_sic2);
	sprintf (pohr_rec.stdin2,"%-60.60s",(cc) ? " " : exsi_rec.inst_text);

	cc = FindExsi (comr_rec.po_sic3);
	sprintf (pohr_rec.stdin3,"%-60.60s",(cc) ? " " : exsi_rec.inst_text);
	
	strcpy (pohr_rec.curr_code,sumr_rec.curr_code);
	pohr_rec.curr_rate = exchangeRate;
	strcpy (pohr_rec.status, (envPoAppFlag) ? "U" : "O");
	strcpy (pohr_rec.stat_flag, (quickPo) ? "Q" : "N");
	if (strcmp (envCurrCode, sumr_rec.curr_code))
		strcpy (pohr_rec.stat_flag, "N");
	sprintf (pohr_rec.req_usr, "%-40.40s", "Automatic P/O Generation from LRP");
	sprintf (pohr_rec.reason,  "LRP Work file : %-20.20s", ffwk_rec.filename);
	sprintf (pohr_rec.op_id, "%-14.14s", currentUser);
	pohr_rec.date_create = lsystemDate;

	strcpy (pohr_rec.time_create, TimeHHMM ());
	/*
	dsp_process ("P/Order",pohr_rec.pur_ord_no);
	*/

	cc = abc_add (pohr,&pohr_rec);
	if (cc)
		file_err (cc, "pohr", "DBADD");

	cc = find_rec (pohr,&pohr_rec,LAST,"u");
	if (cc)
		file_err (cc, "pohr", "DBFIND");

	hhpoHash = pohr_rec.hhpo_hash;

	abc_unlock (pohr);

	/*-------------------------------
	| Head Audit Page wih Supplier	|
	-------------------------------*/
	fprintf (fout,
		".PD| Supplier : %-6.6s - %-40.40s Purchase Order - %s%-46.46s|\n",
		sumr_rec.crd_no,
		sumr_rec.crd_name,
		pohr_rec.pur_ord_no,
		" ");

	if (initDone)
		fprintf (fout,".PA\n");

	fprintf (fout,"|------------------");
	fprintf (fout,"|------------------");
	fprintf (fout,"|------------------------------------------");
	fprintf (fout,"|------------");
	fprintf (fout,"|-----");
	fprintf (fout,"|--------------");
	fprintf (fout,"|--------------");
	fprintf (fout,"|----------|\n");

	initDone = 1;
}

/*=======================================================
| Main processing function for creation of works order. |
=======================================================*/
void
AddWorksOrder (void)
{
	/*-------------------------------------
	| Find part number for branch record. |
	-------------------------------------*/
	inei_rec.hhbr_hash	=	ffwk_rec.hhbr_hash;
	strcpy (inei_rec.est_no,comm_rec.est_no);
	cc = find_rec (inei,&inei_rec,COMPARISON,"r");
	if (cc) 
	{
		LogError (FALSE);
		return;
	}

	/*----------------------------------------
	| Find part number for warehouse record. |
	----------------------------------------*/
	incc_rec.hhbr_hash = ffwk_rec.hhbr_hash;
	incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	cc = find_rec (incc, &incc_rec, COMPARISON, "r");
	if (cc)
	{
		LogError (FALSE);
		return;
	}

	/*----------------------------
	| Set default bom and routes |
	----------------------------*/
	bomAlternate = inmr_rec.dflt_bom;
	rtgAlternate = inmr_rec.dflt_rtg;

	if (ffwk_rec.source[0] == 'B' && inei_rec.dflt_bom)
		bomAlternate = inei_rec.dflt_bom;

	if (ffwk_rec.source[0] == 'B' && inei_rec.dflt_rtg)
		rtgAlternate = inei_rec.dflt_rtg;

	if (ffwk_rec.source[0] == 'W' && incc_rec.dflt_bom)
		bomAlternate = incc_rec.dflt_bom;

	if (ffwk_rec.source[0] == 'W' && incc_rec.dflt_rtg)
		rtgAlternate = incc_rec.dflt_bom;

	/*------------------------------------
	| Fit quantity to batch constraints. |
	------------------------------------*/
	ffwk_rec.order_qty =	RoundMultiple 
						 	(
								ffwk_rec.order_qty, 
								envSupOrdRound, 
								inei_rec.prd_multiple, 
								inei_rec.min_batch
							);

	/*-------------------------
	| Find item standard UOM. |
	-------------------------*/
	inum_rec.hhum_hash	= inmr_rec.std_uom;
	cc = find_rec (inum, &inum_rec, COMPARISON, "r");
	if (cc)
		strcpy (inum_rec.uom, inmr_rec.sale_unit);

	/*---------------------
	| Create works order. |
	---------------------*/
	WO_Create ();

	/*-------------------
	| Write Audit Log	|
	-------------------*/
	fprintf (fout,"| %s ",		inmr_rec.item_no);
	fprintf (fout,"| %-16.16s",	pcwo_rec.order_no);
	fprintf (fout,"| %s ",		inmr_rec.description);
	fprintf (fout,"| %10.2f ",	ffwk_rec.order_qty);
	fprintf (fout,"|%-4.4s ",	inum_rec.uom);
	fprintf (fout,"| %12.2f ", 	inei_rec.std_cost);
	fprintf (fout,"|%10.10s|\n",DateToString (pcwo_rec.reqd_date));
	return;
}

int
AddPoln (
 int    lineNumber)
{
	double	cif_fgn = 0.00;
	double	cif_nor = 0.00;
	double	prntFgn = 0.00;
	float	discArray [4];	/* Regulatory and Disc A, B, C percents */
	int		cumulative;


	if (IntFindInmr (ffwk_rec.hhbr_hash))
	{
		LogError (TRUE);
		return (EXIT_FAILURE);
	}

	if (!worksOrderGen)
	{
		if (FindInis (ffwk_rec.hhsu_hash, ffwk_rec.hhbr_hash))
		{
			LogError (FALSE);
			return (EXIT_FAILURE);
		}

		if (inis_rec.fob_cost == 0.00 && envPoCostCalc)
		{
			/*-------------------------------------
			| Find part number for branch record. |
			-------------------------------------*/
			inei_rec.hhbr_hash	=	ffwk_rec.hhbr_hash;
			strcpy (inei_rec.est_no,comm_rec.est_no);
			cc = find_rec (inei, &inei_rec, COMPARISON,"r");
			if (cc) 
			{
				LogError (FALSE);
				return (EXIT_FAILURE);
			}
			inis_rec.fob_cost = inei_rec.last_cost * pohr_rec.curr_rate;
		}
		poln_rec.hhpo_hash = hhpoHash;
		poln_rec.line_no   = lineNumber;
		poln_rec.hhbr_hash = ffwk_rec.hhbr_hash;
		poln_rec.hhum_hash = (envPoUomDef) ? inis_rec.sup_uom : inmr_rec.std_uom;
		poln_rec.hhcc_hash = hhccHash;
		poln_rec.hhlc_hash = hhlcHash;
		sprintf (poln_rec.serial_no,"%-25.25s"," ");
		poln_rec.qty_ord   = twodec (ffwk_rec.order_qty);
		poln_rec.qty_rec   = 0.00;
		poln_rec.grs_fgn_cst = 	GetSupPrice
								 (
									pohr_rec.hhsu_hash,
									poln_rec.hhbr_hash,
									inis_rec.fob_cost,
									poln_rec.qty_ord
								);
						
		cumulative 			 = 	GetSupDisc 
								 (
									pohr_rec.hhsu_hash,
									inmr_rec.buygrp,
									poln_rec.qty_ord,
									discArray
								);

		poln_rec.fob_fgn_cst = CalcNet (poln_rec.grs_fgn_cst,discArray, cumulative);

		prntFgn 			 = poln_rec.fob_fgn_cst;
		poln_rec.grs_fgn_cst = (poln_rec.grs_fgn_cst * inmr_rec.outer_size);
		poln_rec.fob_fgn_cst = (poln_rec.fob_fgn_cst * inmr_rec.outer_size);
		
		/*-----------------------------------------------
		| Calculate Freight Insurance Cost  - Foreign	|
		-----------------------------------------------*/
		poln_rec.frt_ins_cst = CalculateFreight ();

		/*-----------------------
		| Duty is Default Duty	|
		-----------------------*/
		poln_rec.duty = CalculateDuty ();

		/*---------------------
		| Calculate Discounts |
		---------------------*/
		poln_rec.reg_pc		=	discArray [0];
		poln_rec.disc_a		=	discArray [1];
		poln_rec.disc_b		=	discArray [2];
		poln_rec.disc_c		=	discArray [3];

		/*-------------------
		| Calculate CIF FGN	|
		-------------------*/
		cif_fgn = poln_rec.fob_fgn_cst + poln_rec.frt_ins_cst;

		/*-------------------
		| Calculate CIF NZL	|
		-------------------*/
		if (pohr_rec.curr_rate != 0.00)
			cif_nor = cif_fgn / pohr_rec.curr_rate;
		else
			cif_nor = 0.00;

		cif_nor = cif_nor;

		/*-------------------------------------------------------
		| FOB Nor - incorrect but aggrees with po_input !!!!	|
		-------------------------------------------------------*/
		poln_rec.fob_nor_cst = cif_nor;

		/*-----------------------
		| Calculate Licence NZL	|
		-----------------------*/
		poln_rec.licence	=	CalculateLicence ();
		poln_rec.licence	*=	cif_nor;
		poln_rec.licence	=	DOLLARS (poln_rec.licence);
		poln_rec.licence	=	poln_rec.licence;

		/*-------------------------------
		| Calculate Landed Cost NZL	|
		-------------------------------*/
		poln_rec.land_cst		=	cif_nor + poln_rec.duty + poln_rec.licence;
		poln_rec.lcost_load 	=	DOLLARS (comr_rec.contingency);
		poln_rec.lcost_load 	*=	poln_rec.land_cst;
		poln_rec.lcost_load 	=	poln_rec.lcost_load;
		poln_rec.land_cst 		+=	poln_rec.lcost_load;
		poln_rec.land_cst 		=	poln_rec.land_cst;

		strcpy (poln_rec.cat_code,	inmr_rec.category);
		strcpy (poln_rec.item_desc,	inmr_rec.description);
		poln_rec.due_date = (long) 	inis_rec.lead_time;
		poln_rec.due_date += comm_rec.inv_date;
		strcpy (poln_rec.pur_status, (envPoAppFlag) ? "U" : "O");
		strcpy (poln_rec.stat_flag,"B");

		cc = abc_add (poln,&poln_rec);
		if (cc)
			file_err (cc, "poln", "DBADD");

		add_hash
		(
			comm_rec.co_no,
			comm_rec.est_no,
			"RC",
			0,
			poln_rec.hhbr_hash,
			poln_rec.hhcc_hash,
			0L,
			 (double) 0.00
		);
		inum_rec.hhum_hash	= inmr_rec.std_uom;
		cc = find_rec (inum, &inum_rec, COMPARISON, "r");
		if (cc)
			strcpy (inum_rec.uom, inmr_rec.sale_unit);

	}

	/*-------------------
	| Write Audit Log	|
	-------------------*/
	fprintf (fout,"| %s ",		inmr_rec.item_no);
	fprintf (fout,"| %s ",		inis_rec.sup_part);
	fprintf (fout,"| %s ",		inmr_rec.description);
	fprintf (fout,"| %10.2f ",	poln_rec.qty_ord);
	fprintf (fout,"|%-4.4s ",	inum_rec.uom);
	fprintf (fout,"| %12.2f ", 	prntFgn);
	fprintf (fout,"| %12.2f ", 	poln_rec.land_cst);
	fprintf (fout,"|%10.10s|\n",DateToString (poln_rec.due_date));
	fflush (fout);
	return (EXIT_SUCCESS);
}

/*===================
| Calculate freight |
===================*/
double	
CalculateFreight (void)
{
	double	freight = 0.00;

	strcpy (pocf_rec.co_no,pohr_rec.co_no);
	strcpy (pocf_rec.code,sumr_rec.ctry_code);
	cc = find_rec (pocf,&pocf_rec,COMPARISON,"r");
	if (cc)
		return (0.00);

	switch (pocf_rec.load_type [0])
	{
	case	'P':
		freight = pocf_rec.freight_load / 100;
		freight *= poln_rec.fob_fgn_cst;
		if (pohr_rec.curr_rate != 0.00)
			freight /= pohr_rec.curr_rate;
		freight = twodec (freight);
		return (freight);

	case	'U':
		return (pocf_rec.freight_load);

	default:
		return (0.00);
	}
}

/*================
| Calculate Duty |
================*/
double	
CalculateDuty (void)
{
	double	duty;

	strcpy (podt_rec.co_no,pohr_rec.co_no);
	strcpy (podt_rec.code,inis_rec.duty);

	cc = find_rec (podt,&podt_rec,COMPARISON,"r");
	if (cc)
		return (0.00);

	switch (podt_rec.duty_type [0])
	{
	case	'D':
		return (podt_rec.im_duty);

	case	'P':
		duty = podt_rec.im_duty / 100;
		duty *= poln_rec.fob_fgn_cst;
		if (pohr_rec.curr_rate != 0.00)
			duty /= pohr_rec.curr_rate;
		duty = twodec (duty);
		return (duty);

	default:
		return (0.00);
	}
}

/*===================
| Calculate Licence |
===================*/
double	
CalculateLicence (void)
{
	strcpy (polh_rec.co_no,comm_rec.co_no);
	strcpy (polh_rec.est_no,comm_rec.est_no);
	strcpy (polh_rec.lic_cate,inis_rec.licence);
	sprintf (polh_rec.lic_no,"%-10.10s"," ");
	cc = find_rec (polh,&polh_rec,GTEQ,"r");

	if (!cc && !strcmp (polh_rec.co_no,comm_rec.co_no) &&  
 		       !strcmp (polh_rec.est_no,comm_rec.est_no) &&  
 		       !strcmp (polh_rec.lic_cate,inis_rec.licence))
	{
		hhlcHash = polh_rec.hhlc_hash;
		licenceRate = polh_rec.ap_lic_rate;
	}
	else
	{
		hhlcHash = 0L;
		licenceRate = 0.00;
	}
	return ((double) licenceRate);
}

void
FindPoNumber (void)
{

	int		year;
    time_t	tloc = -1;
	char	WorkYear [5];
	char	tmp_po_no [16];
	char	tempNumber [16];
	int		PoCounter	=	0;
	int		workLen		=	0;
	struct tm	*tmPtr;

	workLen	=	(FGN_CURR) ? strlen (envPoForeign) : strlen (envPoLocal);

	/*------------------------------------------------------
	| Specific purchase order number generation from Ikea. |
	| Number is last digit of year + number of days since  |
	| Jan 1 + a sequence number i.e no of po's today.	   |
	------------------------------------------------------*/
	if (IkeaPoNumbers)
	{
		tloc = time (&tloc);
		tmPtr = localtime (&tloc);

		DateToDMY (lsystemDate, NULL, NULL, &year);
		sprintf (WorkYear,"%04d", year);

		sprintf (tmp_po_no, "%-1.1s%03d%03d",
					WorkYear + 3, (int) tmPtr->tm_yday, ++PoCounter);

		while (CheckPohr (tmp_po_no) == 0)
			sprintf (tmp_po_no, "%-1.1s%03d%03d", WorkYear + 3, (int) tmPtr->tm_yday, ++PoCounter);

		strcpy (pohr_rec.pur_ord_no, tmp_po_no);
	}
	else if (!envPoNumGen)
	{
		strcpy (comr_rec.co_no,comm_rec.co_no);
		cc = find_rec (comr,&comr_rec,COMPARISON,"u");
        if (cc)
        	file_err (cc, "comr", "DBFIND");

		sprintf (tempNumber, "%ld", (FGN_CURR)  ? ++comr_rec.nx_po_no_fgn 
												: ++comr_rec.nx_po_no);
		sprintf 
		(
			tmp_po_no,
			"%-*.*s%s",
			workLen, workLen,
			(FGN_CURR) ? envPoForeign : envPoLocal,
			zero_pad (tempNumber, 15 - workLen)
		);
		while (CheckPohr_c (tmp_po_no) == 0)
		{
			sprintf 
			(
				tmp_po_no,
				"%-*.*s%s",
				workLen, workLen,
				(FGN_CURR) ? envPoForeign : envPoLocal,
				zero_pad (tempNumber, 15 - workLen)
			);
		}
		cc = abc_update (comr,&comr_rec);
        if (cc)
        	file_err (cc, "comr", "DBUPDATE");

		abc_unlock (comr);

		strcpy (pohr_rec.pur_ord_no, tmp_po_no);
		abc_selfield(pohr2, "pohr_id_no2");
	}
	else
	{
		strcpy (esmr_rec.co_no,comm_rec.co_no);
		sprintf (esmr_rec.est_no, comm_rec.est_no);
		cc = find_rec (esmr,&esmr_rec,COMPARISON,"u");
		if (cc)
			file_err (cc, "esmr", "DBFIND");

		sprintf (tempNumber, "%ld", (FGN_CURR)  ? ++esmr_rec.nx_pur_fgn 
												: ++esmr_rec.nx_pur_ord_no);
		sprintf 
		(
			tmp_po_no,
			"%-*.*s%s",
			workLen, workLen,
			(FGN_CURR) ? envPoForeign : envPoLocal,
			zero_pad (tempNumber, 15 - workLen)
		);
		while (CheckPohr (tmp_po_no) == 0)
		{
			sprintf 
			(
				tmp_po_no,
				"%-*.*s%s",
				workLen, workLen,
				(FGN_CURR) ? envPoForeign : envPoLocal,
				zero_pad (tempNumber, 15 - workLen)
			);
		}
		cc = abc_update (esmr,&esmr_rec);
		if (cc)
			file_err (cc, "esmr", "DBUPDATE");
			
		abc_unlock (esmr);
		strcpy (pohr_rec.pur_ord_no, tmp_po_no);
	}
}

/*============================
| Find Special instructions. |
============================*/
int
FindExsi (
 int    INST_NO)
{
	strcpy (exsi_rec.co_no,comm_rec.co_no);
	exsi_rec.inst_code 	=	INST_NO;
	return (find_rec (exsi,&exsi_rec,COMPARISON,"r"));
}

/*=========================================
| Log error as it is better than sys_err. |
=========================================*/
void
LogError (
 int    item)
{
	fprintf (fout,"| *** ERROR IN FILE");
	fprintf (fout,"%-14.14s /  ", ffwk_rec.filename);
	fprintf (fout,"SORT[%-34.34s] ", ffwk_rec.sort);

	fprintf (fout, (item) ? " -> No inventory item found <-    " 
			      : " -> No Supplier record found <-   ");

	fprintf (fout,"               ");
	fprintf (fout,"           |\n");
}

/*=================================================
| Check for purchase order number by Branch Level. |
==================================================*/
int
CheckPohr (
 char*  po_no)
{
	strcpy (pohr2_rec.co_no,comm_rec.co_no);
	strcpy (pohr2_rec.br_no,comm_rec.est_no);
	sprintf (pohr2_rec.pur_ord_no,"%-15.15s", po_no);
	return (find_rec (pohr2,&pohr2_rec,COMPARISON,"r"));
}

/*=================================================
| Check for purchase order number by Company Level. |
==================================================*/
int
CheckPohr_c (
 char*  po_no)
{
	abc_selfield(pohr2, "pohr_id_no3");

	strcpy (pohr2_rec.co_no,comm_rec.co_no);
	sprintf (pohr2_rec.pur_ord_no,"%-15.15s", po_no);
	return (find_rec (pohr2,&pohr2_rec,COMPARISON,"r"));
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

void
WO_Create (void)
{
	WO_UpdatePcwo ();

	strcpy (rghr_rec.co_no, comm_rec.co_no);
	strcpy (rghr_rec.br_no, comm_rec.est_no);
	rghr_rec.hhbr_hash 	= pcwo_rec.hhbr_hash;
	rghr_rec.alt_no 	= pcwo_rec.rtg_alt;
	cc = find_rec (rghr, &rghr_rec, COMPARISON, "r");
	if (cc)
	{
		/*------------------------------------
		| Routing Maintenance not performed. |
		------------------------------------*/
		return;
	}

	if (qtyReqCalc == qtyPrdCalc)
		return;

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
}

/*======================================
| Production Control Works Order file. |
======================================*/
void
WO_UpdatePcwo (void)
{
	qtyPrdCalc = (float) n_dec (inei_rec.std_batch, inmr_rec.dec_pt);

	strcpy (pcwo_rec.co_no, comm_rec.co_no);
	strcpy (pcwo_rec.br_no, comm_rec.est_no);
	strcpy (pcwo_rec.wh_no, comm_rec.cc_no);

	/*----------------------------------------------------
	| get the next works order number from the ccmr file |
	----------------------------------------------------*/
	strcpy (ccmr_rec.co_no,  comm_rec.co_no);
	strcpy (ccmr_rec.est_no, comm_rec.est_no);
	strcpy (ccmr_rec.cc_no,	 comm_rec.cc_no);
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
	pcwo_rec.prod_qty 		= ffwk_rec.order_qty;
	pcwo_rec.act_prod_qty 	= (float) 0.00;
	pcwo_rec.act_rej_qty 	= (float) 0.00;
	strcpy (pcwo_rec.order_status, "P");
	strcpy (pcwo_rec.stat_flag, "0");
	cc = abc_add (pcwo, &pcwo_rec);
	if (cc)
		file_err (cc, pcwo, "DBADD");

	cc = find_rec (pcwo, &pcwo_rec, EQUAL, "u");
	if (cc)
		file_err (cc, pcwo, "DBFIND");

	hhwoHash = pcwo_rec.hhwo_hash;
	qtyReqCalc = ffwk_rec.order_qty;

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
	pcms_rec.hhwo_hash 	= pcwo_rec.hhwo_hash;
	pcms_rec.uniq_id 	= 0;
	cc = find_rec (pcms, &pcms_rec, GTEQ, "u");
	while (!cc && pcms_rec.hhwo_hash == pcwo_rec.hhwo_hash)
	{
		pcms_rec.matl_qty *= qtyReqCalc;
		pcms_rec.matl_qty /= qtyPrdCalc;
		cc = abc_update (pcms, &pcms_rec);
		if (cc)
			file_err (cc, pcms, "DBUPDATE");

		CheckIncc (pcwo_rec.hhcc_hash, pcms_rec.mabr_hash);

		/*---------------------------------------------------------
		| Add sobg record to re-calc. the material committed qty. |
		---------------------------------------------------------*/
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
		cc = find_rec (pcms, &pcms_rec, NEXT, "u");
	}
	abc_unlock (pcms);
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
| Checks if the item has an incc record,                 |
| if the record is not found the record is then created. |
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
		strcpy (incc_rec.ff_option, "A");
		strcpy (incc_rec.allow_repl, "E");
		strcpy (incc_rec.abc_code, "A");
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
	strcpy (pcwo2_rec.co_no, comm_rec.co_no);
	strcpy (pcwo2_rec.br_no, comm_rec.est_no);
	strcpy (pcwo2_rec.wh_no, comm_rec.cc_no);
	sprintf (pcwo2_rec.order_no, "%07ld", orderNo);
	return (find_rec (pcwo3, &pcwo2_rec, COMPARISON, "r"));
}

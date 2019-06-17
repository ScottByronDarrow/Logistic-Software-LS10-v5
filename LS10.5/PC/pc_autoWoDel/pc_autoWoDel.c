/*=====================================================================
|  Copyright (C) 1999 - 2001 Logistic Software Limited   .            |
|=====================================================================|
| $Id: pc_autoWoDel.c,v 5.2 2001/08/09 09:14:28 scott Exp $
|  Program Name  : (pc_autoWoDel.c)
|  Program Desc  : (Automatic Works Order Delete Program)
|---------------------------------------------------------------------|
|  Date Written  : 20th April 2001 | Author       : Scott B Darrow.   |
|---------------------------------------------------------------------|
| $Log: pc_autoWoDel.c,v $
| Revision 5.2  2001/08/09 09:14:28  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:34:52  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:10:03  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 1.2  2001/05/14 05:51:10  scott
| Updated to add new XML error logging
|
| Revision 1.1  2001/04/20 07:28:46  scott
| New program for XML automatic delete of works order.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: pc_autoWoDel.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PC/pc_autoWoDel/pc_autoWoDel.c,v 5.2 2001/08/09 09:14:28 scott Exp $";

#include	<pslscr.h>
#include	<ml_std_mess.h>
#include	<ml_pc_mess.h>
#include	<GlUtils.h>
#include	<twodec.h>
#include	<proc_sobg.h>
#include	<pc_autoWoDel.h>
#include	<XML_Error.h>

#include	"schema"

struct comrRecord	comr_rec;
struct ccmrRecord	ccmr_rec;
struct commRecord	comm_rec;
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

	char	*data	= "data";

	FILE	*fout;

	int		envVarPcGenNum	=	FALSE,
			printerOpen		=	FALSE,
			printerNo		=	1;

	long	hhwoHash		=	0L,
			currHhcc		=	0L,
			wip_mat_hash	=	0L, 
			wip_lbr_hash	=	0L, 
			wip_ovh_hash	=	0L, 
			wip_oth_hash	=	0L, 
			delJobHhmrHash	=	0L;

	char	wip_mat_acc [MAXLEVEL + 1], 
			wip_lbr_acc [MAXLEVEL + 1], 
			wip_ovh_acc [MAXLEVEL + 1], 
			wip_oth_acc [MAXLEVEL + 1], 
			del_job_acc [MAXLEVEL + 1],
			localCurrency [4],
			oldStatus [2];

#include	<SrchPcwo2.h>

/*=====================
| function prototypes |
=====================*/
int		PRError 				(int);
int 	heading 				(int);
int 	spec_valid 				(int);
int 	DisplayDetails 			(int);
int 	DeleteFunc 				(long);
void 	shutdown_prog 			(void);
void	StartProgram 			(void);
void 	ReadDefault 			(void);
void 	OpenDB 					(void);
void 	CloseDB 				(void);
void 	ReportStock 			(void);
void 	AddGlpc 				(long, char *, char *, double, char *);

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int  argc, 
 char *argv [])
{
	long	hhwoHash	=	0L;

	if (scanf ("%d", &printerNo) == EOF)
		return (EXIT_FAILURE);

	StartProgram ();
	
	while (scanf ("%ld", &hhwoHash) != EOF)
	{
		DeleteFunc (hhwoHash);
	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*===========================================================================
| Start up program, Open database files, read default accounts, open audit. |
===========================================================================*/
void
StartProgram (void)
{
	printerOpen = FALSE;

	/*----------------------
	| Open database files. |
	----------------------*/
	OpenDB ();

}
/*=======================
| Program exit sequence	|
=======================*/
void
shutdown_prog (void)
{
	if (printerOpen)
		pclose (fout);

	recalc_sobg ();
	CloseDB (); 
	FinishProgram ();
}
void
ReadDefault (void)
{
	abc_selfield (glmr, "glmr_id_no");

	strcpy (ccmr_rec.co_no,  comm_rec.co_no);
	strcpy (ccmr_rec.est_no, comm_rec.est_no);
	strcpy (ccmr_rec.cc_no,  comm_rec.cc_no);
	cc = find_rec (ccmr, &ccmr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");

	currHhcc = ccmr_rec.hhcc_hash;

	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"WIP D MATL",
		" ",
		inmr_rec.category
	);
	wip_mat_hash = glmrRec.hhmr_hash;
	strcpy (wip_mat_acc, glmrRec.acc_no);

	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"WIP D LABR",
		" ",
		inmr_rec.category
	);
	wip_lbr_hash = glmrRec.hhmr_hash;
	strcpy (wip_lbr_acc, glmrRec.acc_no);

	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"WIP D MACH",
		" ",
		inmr_rec.category
	);
	wip_ovh_hash = glmrRec.hhmr_hash;
	strcpy (wip_ovh_acc, glmrRec.acc_no);

	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"WIP D OTH ",
		" ",
		inmr_rec.category
	);
	wip_oth_hash = glmrRec.hhmr_hash;
	strcpy (wip_oth_acc, glmrRec.acc_no);

	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"MIS SK ISS",
		" ",
		inmr_rec.category
	);
	delJobHhmrHash = glmrRec.hhmr_hash;
	strcpy (del_job_acc, glmrRec.acc_no);

	abc_selfield (glmr, "glmr_hhmr_hash");
}

/*=======================
| Open data base files	|
=======================*/
void
OpenDB (void)
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

	open_rec (inmr,  inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (inum,  inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	open_rec (pcgl,  pcgl_list, PCGL_NO_FIELDS, "pcgl_id_no");
	open_rec (pcln,  pcln_list, PCLN_NO_FIELDS, "pcln_id_no");
	open_rec (pcms,  pcms_list, PCMS_NO_FIELDS, "pcms_id_no");
	open_rec (pcrq,  pcrq_list, PCRQ_NO_FIELDS, "pcrq_id_no2");
	open_rec (pcwc,  pcwc_list, PCWC_NO_FIELDS, "pcwc_hhwc_hash");
	open_rec (pcwo,  pcwo_list, PCWO_NO_FIELDS, "pcwo_hhwo_hash");
	open_rec (rgrs,  rgrs_list, RGRS_NO_FIELDS, "rgrs_hhrs_hash");
	open_rec (ccmr,  ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");

	OpenGlmr ();
}

/*=======================
| Close data base files	|
=======================*/
void
CloseDB (
 void)
{
	abc_fclose (inmr);
	abc_fclose (inum);
	abc_fclose (pcgl);
	abc_fclose (pcln);
	abc_fclose (pcms);
	abc_fclose (pcrq);
	abc_fclose (pcwc);
	abc_fclose (pcwo);
	abc_fclose (rgrs);
	abc_fclose (ccmr);
	GL_Close ();

	abc_dbclose (data);
}
/*------------------------
| Update pcwo to 'D' etc |
------------------------*/
int
DeleteFunc (
	long	hhwoHash)
{
	char	currentWorkCentre [11];
	int		firstTime = TRUE;
	long	tempTime;
	double	deleteMaterial	= 0.00,
			deleteTotal		= 0.00,
			tempLabour		= 0.00, 
			tempOverHead	= 0.00; 

	/*-------------------
	| Find Works order. |
	-------------------*/
	pcwo_rec.hhwo_hash	=	hhwoHash;
	cc = find_rec (pcwo, &pcwo_rec, EQUAL, "u");
	if (cc)
	{
		abc_unlock (pcwo);
		return (PRError (ERR_WOD_NO_ORDER));
	}

	/*---------------------------------------------------------------
	| Deleted orders must be of the following types:				|
	| P (lanned, F (irm Planned, I (ssuing, A (llocated, R (eleased 		|
	---------------------------------------------------------------*/
	if 
	(
		pcwo_rec.order_status [0] != 'P' &&
		pcwo_rec.order_status [0] != 'F' &&
		pcwo_rec.order_status [0] != 'I' &&
		pcwo_rec.order_status [0] != 'A' &&
		pcwo_rec.order_status [0] != 'R'
	)
		return (PRError (ERR_WOD_BAD_FLAG));

	if (pcwo_rec.act_prod_qty != 0.00)
		return (PRError (ERR_WOD_PROD_QTY));

	inmr_rec.hhbr_hash	=	pcwo_rec.hhbr_hash;
	cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
	if (cc)
		return (PRError (ERR_WOD_NO_MKU));

	/*---------------------------------------
	| Read default general ledger accounts. |
	---------------------------------------*/
	ReadDefault ();

	pcms_rec.hhwo_hash 	= pcwo_rec.hhwo_hash;
	pcms_rec.uniq_id 	= 0;
	cc = find_rec (pcms, &pcms_rec, GTEQ, "u");
	while (!cc && pcms_rec.hhwo_hash == pcwo_rec.hhwo_hash)
	{
		ReportStock ();
		deleteMaterial += pcms_rec.amt_issued;

		/*-----------------------------------
		| add sobg record for recalculation |
		| of BOM committed qty              |
		-----------------------------------*/
		add_hash 
		(
			comm_rec.co_no, 
			comm_rec.est_no, 
			"RC",
			0,
			pcms_rec.mabr_hash, 
			currHhcc, 
			0L, 
			(double) 0
		);

		abc_unlock (pcms);
		cc = find_rec (pcms, &pcms_rec, NEXT, "u");
	}
	abc_unlock (pcms);

	pcln_rec.hhwo_hash 	= hhwoHash;
	pcln_rec.seq_no 	= 0;
	pcln_rec.line_no 	= 0;
	cc = find_rec (pcln, &pcln_rec, GTEQ, "r");
	if (!cc)
	{	
		pcwc_rec.hhwc_hash	=	pcln_rec.hhwc_hash;
		if (!find_rec (pcwc, &pcwc_rec, EQUAL, "r"))
			strcpy (currentWorkCentre, pcwc_rec.work_cntr);
	}
	while (!cc && pcln_rec.hhwo_hash == hhwoHash)
	{
		pcrq_rec.hhwo_hash 	= pcln_rec.hhwo_hash;
		pcrq_rec.seq_no 	= pcln_rec.seq_no;
		pcrq_rec.line_no 	= pcln_rec.line_no;
		cc = find_rec (pcrq, &pcrq_rec, EQUAL, "r");
		/*-----------------------------------------------------------
		| If we can't find the pcrq, I think it's fair to assume	|
		| that it doesn't have any act.	times on it.!!				|
		-----------------------------------------------------------*/
		if (cc)
		{
			cc = find_rec (pcln, &pcln_rec, NEXT, "r");
			continue;
		}
		tempTime = pcrq_rec.act_setup + pcrq_rec.act_run + pcrq_rec.act_clean;
		tempLabour	= (double) tempTime * pcln_rec.rate / 60.00;
		tempOverHead = (double) tempTime * pcln_rec.ovhd_var / 60.00;

		if (pcln_rec.seq_no < pcwo_rec.rtg_seq)
		{
			deleteMaterial += tempLabour;
			deleteMaterial += tempOverHead;
			deleteMaterial += pcln_rec.ovhd_fix;
			cc = find_rec (pcln, &pcln_rec, NEXT, "r");
			continue;
		}
		if (firstTime && pcln_rec.seq_no == pcwo_rec.rtg_seq)
		{
			firstTime = FALSE;
			pcwc_rec.hhwc_hash	=	pcln_rec.hhwc_hash;
			cc = find_rec (pcwc, &pcwc_rec, EQUAL, "r");
			if (cc)
				file_err (cc, pcwc, "DBFIND");

			strcpy (currentWorkCentre, pcwc_rec.work_cntr);
		}
		if (tempLabour != 0.00 || tempOverHead != 0.00)
		{
			rgrs_rec.hhrs_hash	=	pcln_rec.hhrs_hash;
			cc = find_rec (rgrs, &rgrs_rec, EQUAL, "r");
			if (cc)
				file_err (cc, rgrs, "DBFIND");

			pcwc_rec.hhwc_hash	=	pcln_rec.hhwc_hash;
			cc = find_rec (pcwc, &pcwc_rec, EQUAL, "r");
			if (cc)
				file_err (cc, pcwc, "DBFIND");
		}

		if (tempLabour != 0.00)
		{
			glmrRec.hhmr_hash	= rgrs_rec.dir_hash;
			cc = find_rec (glmr, &glmrRec, EQUAL, "r");
			if (cc)
				file_err (cc, glmr, "DBFIND");

			AddGlpc
			(
				rgrs_rec.dir_hash, 
				glmrRec.acc_no, 
				pcwc_rec.work_cntr, 
				tempLabour, 
				"2"
			);
			deleteTotal += tempLabour;
		}

		if (tempOverHead != 0.00)
		{
			AddGlpc
			(
				wip_ovh_hash, 
				wip_ovh_acc, 
				pcwc_rec.work_cntr, 
				tempOverHead, 
				"2"
			);
			deleteTotal += tempOverHead;
		}

		deleteMaterial += pcln_rec.ovhd_fix;
		cc = find_rec (pcln, &pcln_rec, NEXT, "r");
	}

	deleteTotal += deleteMaterial;
	AddGlpc (wip_mat_hash, wip_mat_acc, currentWorkCentre, deleteMaterial, "2");
	AddGlpc (delJobHhmrHash, del_job_acc, "          ", deleteTotal, "1");

	/*-------------------------
	| Remove any pcrq records |
	-------------------------*/
	if (oldStatus [0] == 'A' || oldStatus [0] == 'R')
	{
		pcrq_rec.hhwo_hash 	= hhwoHash;
		pcrq_rec.seq_no 	= 0;
		pcrq_rec.line_no 	= 0;
		cc = find_rec (pcrq, &pcrq_rec, GTEQ, "u");
		while (!cc && pcrq_rec.hhwo_hash == hhwoHash)
		{
			cc = abc_delete (pcrq);
			if (cc)
				file_err (cc, pcrq, "DBDELETE");

			pcrq_rec.hhwo_hash 	= hhwoHash;
			pcrq_rec.seq_no 	= 0;
			pcrq_rec.line_no 	= 0;
			cc = find_rec (pcrq, &pcrq_rec, GTEQ, "u");
		}
		abc_unlock (pcrq);
	}

	/*-------------------------------
	| Close pipe to pformat if open |
	-------------------------------*/
	if (printerOpen)
	{
		fprintf (fout, ".EOF\n");
		fflush (fout);
		pclose (fout);
		printerOpen = FALSE;
	}

	/*---------------------------------------------------------------------
	| add sobg record for recalculation of final product's on order qty   |
	| decrease the on order qty for the final product 	                  |
	---------------------------------------------------------------------*/
	add_hash 
	(
		pcwo_rec.co_no, 
		pcwo_rec.br_no, 
		"RP", 
		0,
		pcwo_rec.hhbr_hash, 
		pcwo_rec.hhcc_hash, 
		0L, 
		(double) 0
	); 

	/* find requesting warehouse name */
	strcpy (ccmr_rec.co_no, pcwo_rec.co_no);
	strcpy (ccmr_rec.est_no, pcwo_rec.req_br_no);
	strcpy (ccmr_rec.cc_no, pcwo_rec.req_wh_no);
	cc = find_rec (ccmr, &ccmr_rec, EQUAL, "r");
	if (cc)
		return (PRError (ERR_WOD_NO_CCMR));

	/*--------------------------------------------------
	| Decrease the on order qty for the final product. |
	--------------------------------------------------*/
	add_hash 
	(
		pcwo_rec.co_no, /* At requesting branch/warehouse */
		pcwo_rec.req_br_no, 
		"RP", 
		0,
		pcwo_rec.hhbr_hash, 
		ccmr_rec.hhcc_hash, 
		0L, 
		(double) 0
	); 

	/*---------------------------
	| Update pcwo to status 'D' |
	---------------------------*/
	strcpy (pcwo_rec.order_status, "D");
	cc = abc_update (pcwo, &pcwo_rec);
	if (cc)
		file_err (cc, pcwo, "DBUPDATE");

	return (EXIT_SUCCESS);
}

/*---------------------------------------------
| Report on stock that can't be automatically |
| deissued and receipted into stock. Stock in |
| this category will need to be written on    |
---------------------------------------------*/
void
ReportStock (
 void)
{
	char	orderStatus [21];

	if (!printerOpen)
	{
		if ( (fout = popen ("pformat", "w")) == (FILE *)0)
			sys_err ("Error in pformat during (POPEN)", errno, PNAME);

		printerOpen = TRUE;

		fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
		fprintf (fout, ".LP%d\n", printerNo);
	
		fprintf (fout, ".15\n");
		fprintf (fout, ".PI10\n");
		fprintf (fout, ".L120\n");

		fprintf (fout, ".B1\n");
		fprintf (fout, ".ESTOCK IMBALANCE AFTER AUTOMATIC WORKS ORDER DELETION\n");
		fprintf (fout, ".ECOMPANY %s : %s\n", 
			comm_rec.co_no, clip (comm_rec.co_name));
		fprintf (fout, ".EBRANCH %s : %s\n", 
			comm_rec.est_no, clip (comm_rec.est_name));
		fprintf (fout, ".EWAREHOUSE %s : %s\n",
			comm_rec.cc_no, clip (comm_rec.cc_name));
		fprintf (fout, ".B1\n");

		fprintf (fout, ".CWORKS ORDER NUMBER: %-7.7s ", pcwo_rec.order_no);
		fprintf (fout, "BATCH NUMBER: %-10.10s\n", pcwo_rec.batch_no);
		fprintf (fout, ".CITEM: %-16.16s   ", inmr_rec.item_no);
		fprintf (fout, "DESCRIPTION: %-40.40s\n", inmr_rec.description);
	
		strcpy (orderStatus, " ");

		switch (pcwo_rec.order_status [0])
		{
		case	'P':
			strcpy (orderStatus, ML ("Planned"));
			break;
	
		case	'F':
			strcpy (orderStatus, ML ("Firm Planned"));
			break;
	
		case	'I':
			strcpy (orderStatus, ML ("Issued"));
			break;
	
		case	'A':
			strcpy (orderStatus, ML ("Allocated"));
			break;
	
		case	'R':
			strcpy (orderStatus, ML ("Released"));
			break;
		}
		fprintf (fout, ".CSTATUS WHEN DELETED: %-10.10s     ", orderStatus);
		fprintf (fout, "ROUTING SEQUENCE WHEN DELETED: %2d\n",pcwo_rec.rtg_seq);
	
		fprintf (fout, "========================================");
		fprintf (fout, "========================================");
		fprintf (fout, "=======================================\n");
	
		fprintf (fout, "|     MATERIAL     ");
		fprintf (fout, "|               DESCRIPTION                ");
		fprintf (fout, "| ISSUED FOR SEQ. ");
		fprintf (fout, "| QUANTITY REQUIRED ");
		fprintf (fout, "| QUANTITY ISSUED |\n");
	
		fprintf (fout, "|------------------");
		fprintf (fout, "+------------------------------------------");
		fprintf (fout, "+-----------------");
		fprintf (fout, "+-------------------");
		fprintf (fout, "+-----------------|\n");

		fprintf (fout, ".R======================================");
		fprintf (fout, "========================================");
		fprintf (fout, "=========================================\n");
	}

	inmr2_rec.hhbr_hash	= pcms_rec.mabr_hash;
	cc = find_rec (inmr, &inmr2_rec, COMPARISON, "r");
	if (cc)
	{
		strcpy (inmr2_rec.item_no, "UNKNOWN ITEM    ");
		sprintf (inmr2_rec.description, "%-40.40s", "UNKNOWN ITEM");
	}

	fprintf (fout, "| %-16.16s ", inmr2_rec.item_no);
	fprintf (fout, "| %-40.40s ", inmr2_rec.description);
	fprintf (fout, "|       %2d        ", pcms_rec.iss_seq);
	fprintf (fout, "|  %14.6f   ", pcms_rec.matl_qty);
	fprintf (fout, "| %14.6f  |\n", pcms_rec.qty_issued);

	fflush (fout);
}

/*===============================
| Add a trans to the pcgl file.	|
| NB: amount should be in cents	|
===============================*/
void
AddGlpc
 (
	long	hhmrHash,
	char	*accountNumber, 
	char	*workCentre, 
	double	amount,
	char	*type
)
{
	int		periodMonth;

	if (amount == 0.00)
		return; 

	strcpy (pcgl_rec.co_no, comm_rec.co_no);
	strcpy (pcgl_rec.tran_type, "19");
	pcgl_rec.post_date = comm_rec.inv_date;
	pcgl_rec.tran_date = comm_rec.inv_date;
	DateToDMY (comm_rec.inv_date, NULL, &periodMonth, NULL);
	sprintf (pcgl_rec.period_no,"%02d", periodMonth);

	sprintf (pcgl_rec.sys_ref, "%5.1d", comm_rec.term);
	sprintf (pcgl_rec.user_ref, "%8.8s", workCentre);
	strcpy (pcgl_rec.stat_flag, "2");
	sprintf (pcgl_rec.narrative, "%-20.20s", pcwo_rec.order_no);
	pcgl_rec.amount 	= amount;
	pcgl_rec.loc_amount = amount;
	pcgl_rec.exch_rate	= 1.00;
	strcpy (pcgl_rec.currency, localCurrency);
	strcpy (pcgl_rec.acc_no, accountNumber);
	pcgl_rec.hhgl_hash = hhmrHash;

	strcpy (pcgl_rec.jnl_type, type);
	cc = abc_add (pcgl, &pcgl_rec);
	if (cc)
		file_err (cc, pcgl, "DBADD");
}

/*================
| Process Errors |
================*/
int
PRError (
	int	errCode)
{
	char	errMess [3][81];

	if (ERR_WOD_BAD_FLAG == errCode)
	{
		sprintf (errMess [0], "Works order status not P (lanned, F (irm Planned, I (ssuing, A (llocated, R (eleased");

		fprintf (stderr, "%s\n", errMess [0]);
		XML_Error
		(
			"pc_autoWoDel",
			errMess [0],
			" ",
			" ",
			" "
		);
		return (errCode);
	}
	if (ERR_WOD_NO_ORDER == errCode)
	{
		sprintf (errMess [0], "Works order hash (pcwo_hhwo_hash) must be valid");
		sprintf (errMess [1], "pcwo_hhwo_hash = [%ld]", pcwo_rec.hhwo_hash);

		fprintf (stderr, "%s\n", errMess [0]);
		fprintf (stderr, "%s\n", errMess [1]);
		XML_Error
		(
			"pc_autoWoDel",
			errMess [0],
			errMess [1],
			" ",
			" "
		);
		return (errCode);
	}
	if (ERR_WOD_PROD_QTY == errCode)
	{
		sprintf (errMess [0], "Prduction quantity non zero.");
		sprintf (errMess [1], "pcms_hhwo_hash = [%ld] pcms_uniq_id [%d]", pcms_rec.hhwo_hash, pcms_rec.uniq_id);

		fprintf (stderr, "%s\n", errMess [0]);
		fprintf (stderr, "%s\n", errMess [1]);
		XML_Error
		(
			"pc_autoWoDel",
			errMess [0],
			errMess [1],
			" ",
			" "
		);
		return (errCode);
	}
	if (ERR_WOD_NO_MKU == errCode)
	{
		sprintf (errMess [0], "MKU could not be found for works order");
		sprintf (errMess [1], "inmr_hhbr_hash = [%ld]", inmr_rec.hhbr_hash);
		sprintf (errMess [2], "pcms_hhwo_hash = [%ld]", pcwo_rec.hhwo_hash);

		fprintf (stderr, "%s\n", errMess [0]);
		fprintf (stderr, "%s\n", errMess [1]);
		fprintf (stderr, "%s\n", errMess [2]);
		XML_Error
		(
			"pc_autoWoDel",
			errMess [0],
			errMess [1],
			errMess [2],
			" "
		);
		return (errCode);
	}
	if (ERR_WOD_NO_SKU == errCode)
	{
		sprintf (errMess [0], "SKU could not be found for works order");
		sprintf (errMess [1], "inmr_hhbr_hash = [%ld] ", inmr_rec.hhbr_hash);
		sprintf (errMess [2], "pcms_hhwo_hash = [%ld]", pcwo_rec.hhwo_hash);

		fprintf (stderr, "%s\n", errMess [0]);
		fprintf (stderr, "%s\n", errMess [1]);
		fprintf (stderr, "%s\n", errMess [2]);
		XML_Error
		(
			"pc_autoWoDel",
			errMess [0],
			errMess [1],
			errMess [2],
			" "
		);
		return (errCode);
	}
	if (ERR_WOD_NO_CCMR == errCode)
	{
		sprintf (errMess [0], "Warehouse record could not be found (ccmr)");
		sprintf (errMess [1], "ccmr_co_no [%s] ccmr_est_no [%s] ccmr_cc_no [%s]",
							ccmr_rec.co_no, ccmr_rec.est_no, ccmr_rec.cc_no);

		fprintf (stderr, "%s\n", errMess [0]);
		fprintf (stderr, "%s\n", errMess [1]);
		XML_Error
		(
			"pc_autoWoDel",
			errMess [0],
			errMess [1],
			" ",
			" "
		);
		return (errCode);
	}
	return (errCode);
}

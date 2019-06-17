/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_trandel.c,v 5.3 2001/08/14 02:54:17 scott Exp $
|  Program Name  : (sk_trandel.c)                                     |
|  Program Desc  : (Print & Purge Inventory Transactions      )       |
|---------------------------------------------------------------------|
|  Author        : Fui Choo Yap    | Date Written  : 03/03/88         |
|---------------------------------------------------------------------|
| $Log: sk_trandel.c,v $
| Revision 5.3  2001/08/14 02:54:17  scott
| Updated for new delete wizard
|
| Revision 5.2  2001/08/09 09:20:20  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:46:07  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:18:11  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:39:29  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2001/02/06 10:07:50  scott
| Updated to deal with length change in the following fields
| intr_ref1 from 10 to 15 characters
| intr_ref2 from 10 to 15 characters
| inaf_ref1 from 10 to 15 characters
| inaf_ref2 from 10 to 15 characters
|
| Revision 3.0  2000/10/10 12:21:31  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:12:14  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.5  2000/07/10 01:53:45  scott
| Updated to replace "@ (" with "@(" to ensure psl_what works correctly
|
| Revision 1.4  2000/02/16 09:16:15  scott
| S/C ASL-15980 / LSDI-2498
| New form of transaction delete that is time based and till stop of allocated time expires. Moved away from executing SQL scripts etc.
|
| Revision 1.3  1999/10/08 05:33:00  scott
| First Pass checkin by Scott.
|
| Revision 1.2  1999/06/20 05:20:55  scott
| Updated to add log for cvs + remove old read_comm + fixed all warnings.
|
|                                                                     |
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_trandel.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_trandel/sk_trandel.c,v 5.3 2001/08/14 02:54:17 scott Exp $";

#define	MAXDESC		7
#define	PRINT_TRANS	 (printFlag [0] == 'Y')

#include 	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<pr_format3.h>
#include 	<DeleteControl.h>

#include	"schema"

struct ccmrRecord	ccmr_rec;
struct commRecord	comm_rec;
struct excfRecord	excf_rec;
struct inmrRecord	inmr_rec;
struct intrRecord	intr_rec;

	int		printerNumber = 1,
			defaultNoTrans,
			defaultNoDays,
			noTrans,
			noDays;

	char	printFlag [2];

	double	lineTotal		= 0.00,
			itemAmount		= 0.00,
			totalAmount 	= 0.00;

	float	quantity		= 0.00,
			itemQuantity	= 0.00,
			totalQuantity	= 0.00;

	long	maximumTime 	= 0L;
	FILE	*fout, *fin;

	char	envSkTranDelEx [21];

	static	char	*transType [] =
	{
		" STK BAL ",
		"STK RECPT",
		" STK ISS ",
		" STK ADJ ",
		" STK PUR ",
		" INVOICE ",
		" CREDIT  ",
		" PRD-ISS ",
		" STK-TRN ",
		" PRD-ORD ",
		" STK-W/O ",
		" DD-PUR  ",
		" DD-ISS  "
	};

/*=======================
| Function Declarations |
=======================*/
int		check_page 			(void);
int  	CheckForAltDelete	(void);
long	GetLongTime 		(void);
void 	shutdown_prog 		(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
void 	HeadOutput 			(void);
void 	ProcessInmr 		(void);
void 	ProcessIntr 		(long);
void 	PrintIntr 			(void);
void 	PrintItemTotals		(void);
void 	PrintTotals 		(void);

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int argc, 
 char * argv [])
{
	char	*sptr;

	if (argc != 2)
	{
		printf ("Usage : %s <printerNumber>\n\r", argv [0]);
		return (EXIT_FAILURE);
	}

	sptr = chk_env ("SK_TRAN_DEL_EX");
	if (sptr == (char *)0)
		sprintf (envSkTranDelEx, "%-20.20s", "YYYYYYYYYYYYYYYYYYYY");
	else
		sprintf (envSkTranDelEx, "%-20.20s", sptr);

	/*-------------------
	| Printer Number	|
	-------------------*/
	printerNumber = atoi (argv [1]);

	strcpy (printFlag, (printerNumber) ? "Y" : "N");

	init_scr ();

	OpenDB ();
	/*
	 * Check if delete control file defined for purge.
	 */
	cc = FindDeleteControl (comm_rec.co_no, "INVENTORY-TRANS");
	if (!cc)
	{
		sprintf (envSkTranDelEx, "%-20.20s", delhRec.reference);
		defaultNoDays 	= delhRec.purge_days;
		defaultNoTrans 	= delhRec.spare_fg1;
		maximumTime 	= (long) delhRec.spare_fg2;
		maximumTime 	+= GetLongTime ();
	}

	dsp_screen ("Printing Inventory Transactions To be Purged.", comm_rec.co_no, comm_rec.co_name);

	if (PRINT_TRANS)
	{
		if ((fin = pr_open ("sk_trandel.p")) == NULL)
			sys_err ("Error in opening sk_trandel.p during (PROPEN)", errno, PNAME);

		if ((fout = popen ("pformat", "w")) == NULL)
			sys_err ("Error in opening pformat During (DBPOPEN)", errno, PNAME);

		HeadOutput ();
	}

	ProcessInmr ();

	if (PRINT_TRANS)
	{
		pr_format (fin, fout, "END_FILE", 0, 0);
		pclose (fout);
		fclose (fin);
	}

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

int
check_page (
 void)
{
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
	abc_dbopen ("data");

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_hhcc_hash");
	open_rec (excf, excf_list, EXCF_NO_FIELDS, "excf_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no_3");
	open_rec (intr, intr_list, INTR_NO_FIELDS, "intr_id_no2");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (ccmr);
	abc_fclose (excf);
	abc_fclose (inmr);
	abc_fclose (intr);
	abc_dbclose ("data");
}

/*===================================
| Start Out Put To Standard Print . |
===================================*/
void
HeadOutput (
 void)
{
	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (fout, ".LP%d\n", printerNumber);

	fprintf (fout, ".9\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L158\n");
	fprintf (fout, ".E%s\n", clip (comm_rec.co_name));
	pr_format (fin, fout, "HEADING0", 0, 0);
	fprintf (fout, ".B1\n");
	pr_format (fin, fout, "LINE1", 0, 0);
	pr_format (fin, fout, "HEADING1", 0, 0);
	pr_format (fin, fout, "LINE2", 0, 0);
	pr_format (fin, fout, "RULER", 0, 0);

	fflush (fout);
}

void
ProcessInmr (void)
{
	long	checkTime	=	0L;

	noTrans 	= defaultNoTrans;
	noDays		= defaultNoDays;

	strcpy (inmr_rec.co_no, comm_rec.co_no);
	sprintf (inmr_rec.inmr_class, "%-1.1s", " ");
	sprintf (inmr_rec.category, "%-11.11s", " ");
	sprintf (inmr_rec.item_no, "%-16.16s", " ");
	cc = find_rec (inmr, &inmr_rec, GTEQ, "r");
	while (!cc && !strcmp (inmr_rec.co_no, comm_rec.co_no))
	{
		checkTime	=	GetLongTime	();

		if (checkTime > maximumTime)
		{
			if (PRINT_TRANS)
				fprintf (fout, "Transaction delete halted as time limit reached\n");
			break;
		}
		dsp_process ("Item No : ", inmr_rec.item_no);

		if (inmr_rec.lot_ctrl [0] == 'Y')
		{
			cc = find_rec (inmr, &inmr_rec, NEXT, "r");
			continue;
		}
		/*-------------------------------------------
		| Item category and excf category are diff. |
		-------------------------------------------*/
		if (strcmp (excf_rec.cat_no, inmr_rec.category))
		{
			strcpy (excf_rec.co_no, comm_rec.co_no);
			strcpy (excf_rec.cat_no, inmr_rec.category);
			if (find_rec (excf, &excf_rec, COMPARISON, "r"))
			{
				noTrans	= defaultNoTrans;
				noDays	= defaultNoDays;
			}
			else
			{
				noTrans = excf_rec.no_trans;
				noDays	= excf_rec.no_days;
			}

		}
		if (noTrans == 0 || noDays == 0)
		{
			noTrans	= defaultNoTrans;
			noDays	= defaultNoDays;
		}
		if (noTrans == 9999 || noDays == 9999)
		{
			cc = find_rec (inmr, &inmr_rec, NEXT, "r");
			continue;
		}
		ProcessIntr (inmr_rec.hhbr_hash);

		cc = find_rec (inmr, &inmr_rec, NEXT, "r");
	}
	if (PRINT_TRANS)
		PrintTotals ();
}

void
ProcessIntr (
	long	hhbrHash)
{
	long	cutoffDate	=	0L;
	int		i = 0;
	int		deleted		= 	FALSE;

	intr_rec.hhbr_hash	=	hhbrHash;
	intr_rec.date		=	1L;
	cc = find_rec (intr, &intr_rec, GTEQ, "r");
	while (!cc && intr_rec.hhbr_hash == hhbrHash && (i < noTrans ||
			(intr_rec.date < (comm_rec.inv_date - (long) noDays))))
	{
		i++;
		cutoffDate	=	intr_rec.date;
		cc = find_rec (intr, &intr_rec, NEXT, "r");
	}
	if (intr_rec.hhbr_hash != hhbrHash || i < noTrans)
		return;

	intr_rec.hhbr_hash	=	hhbrHash;
	intr_rec.date		=	1L;
	cc = find_rec (intr, &intr_rec, GTEQ, "u");
	while (!cc && intr_rec.hhbr_hash == hhbrHash)
	{
		/*----------------------------------------
		| Check transaction type can be deleted. |
		----------------------------------------*/
		if (envSkTranDelEx [intr_rec.type - 1] == 'N')
		{
			abc_unlock (intr);
			cc = find_rec (intr, &intr_rec, NEXT, "u");
			continue;
		}
		/*---------------------------------------------------------------
		| CutoffDate is the date of last transaction that makes up the  |
		| minumum number of transactions, if this reached then you exit |
		---------------------------------------------------------------*/
		if (intr_rec.date >= cutoffDate)
			break;
		
		deleted	=	TRUE;
		if (PRINT_TRANS)
			PrintIntr ();

		abc_delete (intr);
		cc = find_rec (intr, &intr_rec, GTEQ, "u");
	}
	abc_unlock (intr);

	if (deleted == TRUE && PRINT_TRANS)
		PrintItemTotals ();
}

void
PrintIntr (
 void)
{
	char	warehouse [3];
	char	transferType [10];
	double	total = 0.00;
	float	quantity = 0.00;

	ccmr_rec.hhcc_hash	=	intr_rec.hhcc_hash;	
	cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
	if (cc)
		strcpy (warehouse, "??");
	else
		strcpy (warehouse, ccmr_rec.cc_no);

	strcpy (transferType, transType [intr_rec.type - 1]);

	switch (intr_rec.type)
	{
	case	1:
	case	2:
	case	4:
	case	5:
		quantity	= intr_rec.qty;
		total 		= intr_rec.cost_price * (double) intr_rec.qty;
		break;

	case	6:
		quantity 	= -1 * intr_rec.qty;
		total 		= -1 * intr_rec.sale_price * (double) intr_rec.qty;
		break;

	case	7:
		quantity 	= intr_rec.qty;
		lineTotal 	= intr_rec.sale_price * (double) intr_rec.qty;
		break;

	default:	/* 3, 8, 9, 10, 11 and others	*/
		quantity 	= -1 * intr_rec.qty;
		lineTotal 	= -1 * intr_rec.cost_price * (double) intr_rec.qty;
		break;
	}
	pr_format (fin, fout, "TRANS", 1, intr_rec.br_no);
	pr_format (fin, fout, "TRANS", 2, warehouse);
	pr_format (fin, fout, "TRANS", 3, inmr_rec.item_no);
	pr_format (fin, fout, "TRANS", 4, inmr_rec.category);
	pr_format (fin, fout, "TRANS", 5, intr_rec.batch_no);
	pr_format (fin, fout, "TRANS", 6, transferType);
	pr_format (fin, fout, "TRANS", 7, intr_rec.date);
	pr_format (fin, fout, "TRANS", 8, intr_rec.ref1);
	pr_format (fin, fout, "TRANS", 9, intr_rec.ref2);
	pr_format (fin, fout, "TRANS", 10, intr_rec.cost_price);
	pr_format (fin, fout, "TRANS", 11, intr_rec.sale_price);
	pr_format (fin, fout, "TRANS", 12, quantity);
	pr_format (fin, fout, "TRANS", 13, lineTotal);

	fflush (fout);
	itemQuantity 	+= quantity;
	itemAmount 		+= lineTotal;
	totalQuantity 	+= quantity;
	totalAmount 	+= lineTotal;
}

void
PrintItemTotals (void)
{
	pr_format (fin, fout, "LINE3", 0, 0);
	pr_format (fin, fout, "ITEM_TOTAL", 1, inmr_rec.item_no);
	pr_format (fin, fout, "ITEM_TOTAL", 2, itemQuantity);
	pr_format (fin, fout, "ITEM_TOTAL", 3, itemAmount);
	pr_format (fin, fout, "LINE3", 0, 0);
	fflush (fout);
	itemQuantity = 0.00;
	itemAmount	 = 0.00;
}

void
PrintTotals (void)
{
	pr_format (fin, fout, "CO_TOTAL", 1, totalQuantity);
	pr_format (fin, fout, "CO_TOTAL", 2, totalAmount);
	fflush (fout);
}
/*====================================
| Routime to get time as long value. |
====================================*/
long
GetLongTime (void)
{
	static char buff [6];

	strcpy (buff, TimeHHMM());

	return ((long) atot (buff));
}

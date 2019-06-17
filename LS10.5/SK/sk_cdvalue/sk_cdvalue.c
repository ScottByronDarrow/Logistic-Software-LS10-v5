/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_cdvalue.c,v 5.2 2001/08/09 09:18:14 scott Exp $
|  Program Name  : (sk_cdvalue.c & sk_bdvalue.c)
|  Program Desc  : (Detailed Stock Valuation Report)
|                  (Serial and FIFO/LIFO items)
|---------------------------------------------------------------------|
|  Date Written  : (15/04/87)      | Author       : Scott Darrow.     |
|---------------------------------------------------------------------|
| $Log: sk_cdvalue.c,v $
| Revision 5.2  2001/08/09 09:18:14  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/07/25 02:18:54  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_cdvalue.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_cdvalue/sk_cdvalue.c,v 5.2 2001/08/09 09:18:14 scott Exp $";

#include	<pslscr.h>
#include	<twodec.h>
#include	<ml_std_mess.h>
#include	<ml_sk_mess.h>
#include	<Costing.h>

#define		CHECK_LEN	 (groupLength < 12)
#define		BY_MODE		 (stockTakeMode [0] != ' ')
#define		MODE_OK		 (incc_rec.stat_flag [0] == stockTakeMode [0])
#define		BY_BR		 (reportBy [0] == 'B')

#include	<dsp_screen.h>
#include	<dsp_process.h>

#include	"schema"

	struct	commRecord	comm_rec;
	struct	ccmrRecord	ccmr_rec;
	struct	excfRecord	excf_rec;
	struct	inccRecord	incc_rec;
	struct	inmrRecord	inmr_rec;
	struct	sttfRecord	sttf_rec;

	char 	*data	= "data";

	FILE	*fout;
			
	int		printerNumber = 1,
			groupLength,
			warehouseMode [101];
	long	warehouseHhcc [101],
			warehouseDate [101];
	char	stockTakeMode [2],
			reportBy [2],
			lower [13],
			upper [13];
	char	defaultQuantity [15],
			reportQtyMask [10],
			tmpStr [30];

	int		stockNegativeValue	=	FALSE;

/*=======================
| Function Declarations |
=======================*/
void	HeadingOutput 		(void);
void	shutdown_prog 		(void);
void	OpenDB 				(void);
void	CloseDB 			(void);
void	ReadControlFiles 	(void);
void	ProcessWarehouse 	(void);
void	ProcessBranch 		(void);
void	PrintCategory 		(int, double);
int		ValidateHash 		(long);
double	GetClosingStock 	(float, long);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{
	char	*sptr;
	int		after,
			before;

	/*---------------------------------------
	|	parameters							|
	|	1:	program name					|
	|	2:	printer number					|
	|	3:	lower bound						|
	|	4:	upper bound						|
	|	5:	Stock Take Mode					|
	|		' ' - no modes expected 		|
	---------------------------------------*/
	if (argc < 5)
	{
		print_at (0,0, mlSkMess018,argv [0]);
		return (EXIT_FAILURE);
	}

	strcpy (reportBy, (!strncmp (argv [0], "sk_cdvalue", 10)) ? "C" : "B");

	printerNumber = atoi (argv [1]);
	sprintf (lower, "%-12.12s", argv [2]);
	sprintf (upper, "%-12.12s", argv [3]);
	if (argc == 5)
		sprintf (stockTakeMode, "%-1.1s", argv [4]);
	else
		strcpy (stockTakeMode, " ");

	sptr = chk_env ("SK_VAL_NEGATIVE");
	stockNegativeValue = (sptr == (char *)0) ? FALSE : atoi (sptr);

	sptr = chk_env ("SK_QTY_MASK");
	if (sptr == (char *)0)
		strcpy (defaultQuantity, "NNNNNNN.NNNNNN");
	else
		strcpy (defaultQuantity, sptr);
	before = strlen (defaultQuantity);
	sptr = strrchr (defaultQuantity, '.');
	if (sptr)
		after = (int) ((sptr + strlen (sptr) - 1) - sptr);
	else
		after = 0;
	if (after == 0)
		sprintf (reportQtyMask, "%%%df", before);
	else
		sprintf (reportQtyMask, "%%%d.%df", before, after);


	if (strlen (clip (lower)) > strlen (clip (upper)))
		groupLength = strlen (clip (lower));
	else
		groupLength = strlen (clip (upper));

	IN_STAKE = (BY_MODE) ? TRUE : FALSE;

	OpenDB ();
	ReadControlFiles ();

	init_scr ();

	if (BY_BR)
		dsp_screen
		 (
			"Processing : Detail Stock Valuation Report by Branch",
			 comm_rec.co_no,
			 comm_rec.co_name
		);
	else
		dsp_screen
		 (
			"Processing : Detail Stock Valuation Report",
			comm_rec.co_no,
			comm_rec.co_name
		);

	/*=================================
	| Open pipe work file to pformat. |
	=================================*/
	if ((fout = popen ("pformat", "w")) == 0)
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);

	HeadingOutput ();

	if (BY_BR)
		ProcessBranch ();
	else
		ProcessWarehouse ();

	fprintf (fout, ".EOF\n");

	/*=========================
	| Program exit sequence	. |
	=========================*/
	pclose (fout);

	shutdown_prog ();
	return (EXIT_SUCCESS);
}

void
HeadingOutput (
 void)
{
	int	i;

	i = (BY_MODE) ? 1 : 0;
	
	fprintf (fout, ".START%s<%s>\n", DateToString (comm_rec.inv_date), PNAME);
	fprintf (fout, ".PI12\n");

	fprintf (fout, ".LP%d\n", printerNumber);
	fprintf (fout, ".%d\n", (BY_BR) ? 11+i : 13+i);
	fprintf (fout, ".L130\n");

	fprintf (fout, ".B1\n");
	if (BY_BR)
		fprintf (fout, ".EDETAILED STOCK VALUATION REPORT BY BRANCH\n");
	else
		fprintf (fout, ".EDETAILED STOCK VALUATION REPORT\n");

	if (BY_MODE)
		fprintf (fout, ".ESTOCK TAKE SELECTION [%s]\n", stockTakeMode);

	fprintf (fout, ".E%s \n", clip (comm_rec.co_name));
	fprintf (fout, ".EBranch: %s \n", clip (comm_rec.est_name));

	if (!BY_BR)
	{
		fprintf (fout, ".B1\n");
		fprintf (fout, ".EWarehouse: %s \n", clip (comm_rec.cc_name));
	}
	fprintf (fout, ".B1\n");
	fprintf (fout, ".E AS AT : %s\n", SystemTime ());

	fprintf (fout, ".R===================");
	fprintf (fout, "===========================================");
	fprintf (fout, "=================");
	fprintf (fout, "================");
	fprintf (fout, "====================\n");

	fprintf (fout, "===================");
	fprintf (fout, "===========================================");
	fprintf (fout, "=================");
	fprintf (fout, "================");
	fprintf (fout, "====================\n");

	fprintf (fout, "|   ITEM NUMBER    ");
	fprintf (fout, "|   I T E M     D E S C R I P T I O N      ");
	fprintf (fout, "|    QUANTITY    ");
	fprintf (fout, "|   C O S T     ");
	fprintf (fout, "|     EXTENDED     |\n");

	fprintf (fout, "|------------------");
	fprintf (fout, "|------------------------------------------");
	fprintf (fout, "|----------------");
	fprintf (fout, "|---------------");
	fprintf (fout, "|------------------|\n");
}

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

	open_rec (excf, excf_list, EXCF_NO_FIELDS, "excf_id_no");
	if (BY_BR)
		open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_hhbr_hash");
	else
		open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no_2");
	if (BY_BR)
		open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no_3");
	else
		open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (sttf, sttf_list, STTF_NO_FIELDS, "sttf_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (excf);
	abc_fclose (incc);
	abc_fclose (inmr);
	abc_fclose (sttf);
	CloseCosting ();
	abc_dbclose (data);
}

void
ReadControlFiles (
 void)
{
	int	i = 0;

	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");

	for (i = 0; i < 100; i++);
	{
		warehouseHhcc [i] = 0L;
		warehouseDate [i] = 0L;
		warehouseMode [i] = 0;
	}

	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, comm_rec.est_no);

	strcpy (ccmr_rec.cc_no, (BY_BR) ? "  " : comm_rec.cc_no);
	cc = find_rec (ccmr, &ccmr_rec, (BY_BR) ? GTEQ : COMPARISON, "r");
	if (cc)
		file_err (cc, "ccmr", "DBFIND");

	/*---------------------------------------------
	| Find stock take record for date validation. |
	---------------------------------------------*/
	FindInsc (ccmr_rec.hhcc_hash, stockTakeMode, BY_MODE);

	i = 0;
	warehouseHhcc [i] = ccmr_rec.hhcc_hash;
	warehouseDate [i] = STAKE_COFF;
	warehouseMode [i] = IN_STAKE;
	warehouseHhcc [i + 1] = 0L;
	warehouseDate [i + 1] = 0L;
	warehouseMode [i + 1] = 0;

	while
	 (
		BY_BR &&
		!cc &&
		!strcmp (ccmr_rec.co_no, comm_rec.co_no) &&
		!strcmp (ccmr_rec.est_no, comm_rec.est_no)
	)
	{
		/*---------------------------------------------
		| Find stock take record for date validation. |
		---------------------------------------------*/
		FindInsc (ccmr_rec.hhcc_hash, stockTakeMode, BY_MODE);

		warehouseDate [i]	= STAKE_COFF;
		warehouseMode [i]	= IN_STAKE;
		warehouseHhcc [i++]	= ccmr_rec.hhcc_hash;
		warehouseHhcc [i]	= 0L;

		cc = find_rec (ccmr, &ccmr_rec, NEXT, "r");
	}

	abc_fclose (ccmr);
}

void
ProcessWarehouse (
 void)
{
	char	oldGroup [13], 
			newGroup [13],
			upperCategory [12];
	double	extend = 0.00, 
			value = 0.00, 
			tot_extend = 0.00, 
			cat_extend = 0.00;
	double	incc_qty = 0.00;
	int		first_time = TRUE;

	/*-----------------------
	|	read first incc	|
	-----------------------*/
	fflush (fout);
	incc_rec.hhcc_hash = warehouseHhcc [0];
	sprintf (incc_rec.sort, "%-28.28s", lower);
	cc = find_rec (incc, &incc_rec, GTEQ, "r");

	sprintf (oldGroup, "%12.12s", 	lower);
	sprintf (upperCategory,	"%-11.11s",	&upper [1]);

	while (!cc && ValidateHash (incc_rec.hhcc_hash))
	{
		fifoError = FALSE;

		if (BY_MODE && !MODE_OK)
		{
			cc = find_rec (incc, &incc_rec, NEXT, "r");
			continue;
		}

		strcpy (inmr_rec.co_no, comm_rec.co_no);
		sprintf (inmr_rec.item_no, "%-16.16s", incc_rec.sort+12);

		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");

		/*---------------------------------------
		| read error or item is superceeded		|
		---------------------------------------*/
		if (cc || strcmp (inmr_rec.supercession, "                "))
		{
			cc = find_rec (incc, &incc_rec, NEXT, "r");
			continue;
		}

		sprintf (newGroup, "%1.1s%-11.11s", inmr_rec.inmr_class, inmr_rec.category);

		if (strncmp (newGroup, upper, groupLength) > 0)
			break;

		if (strcmp (inmr_rec.category, upperCategory) > 0)
			break;

		incc_qty = (BY_MODE) ?
				GetClosingStock (
					n_dec (incc_rec.closing_stock, inmr_rec.dec_pt),
					incc_rec.hhwh_hash) : n_dec (incc_rec.closing_stock,
					inmr_rec.dec_pt);

		incc_qty = n_dec (incc_qty, inmr_rec.dec_pt);

		if (incc_qty <= 0.00 && !stockNegativeValue)
		{
			if (stockNegativeValue)
				strcpy (inmr_rec.costing_flag, "L");
			else
			{
				cc = find_rec (incc, &incc_rec, NEXT, "r");
				continue;
			}
		}
		dsp_process (" Item: ", inmr_rec.item_no);

		value	=	StockValue
					 (
						inmr_rec.costing_flag,
						comm_rec.est_no,
						inmr_rec.hhbr_hash,
						incc_rec.hhwh_hash,
						incc_qty,
						inmr_rec.dec_pt,
						TRUE
					);
		value	=	twodec (value);
		extend	=	value;
		extend	*=	 (double) n_dec (incc_qty, inmr_rec.dec_pt);
		extend	=	out_cost (extend, inmr_rec.outer_size);

		/*---------------------------------------
		| Print if avge_cost or stock on hand	|
		---------------------------------------*/
		if (value != 0.00 || incc_qty != 0.00)
		{
			if (strncmp (newGroup, oldGroup, groupLength))
			{
				strcpy (oldGroup, newGroup);
				PrintCategory (!first_time, cat_extend);
				first_time = FALSE;
				cat_extend = 0.00;
			}
			cat_extend += extend;
			tot_extend += extend;
			fprintf (fout, "| %-16.16s ", inmr_rec.item_no);
			fprintf (fout, "| %-40.40s ", inmr_rec.description);
			sprintf (tmpStr, reportQtyMask, n_dec (incc_qty, inmr_rec.dec_pt));
			fprintf (fout, "| %14s ", tmpStr);
			fprintf (fout, "| %13.2f ", value);
			fprintf (fout, "| %16.2f |%s\n", extend, (fifoError) ? "**" : " ");
			fflush (fout);
		}
		cc = find_rec (incc, &incc_rec, NEXT, "r");
	}
	fprintf (fout, "| %-16.16s ", " ");
	fprintf (fout, "| %-40.40s ", " ");
	fprintf (fout, "| %30.30s ", "*** GROUP TOTAL *** ");
	fprintf (fout, "  %16.2f |\n\n", cat_extend);

	fprintf (fout, "| %-16.16s ", " ");
	fprintf (fout, "| %-40.40s ", " ");
	fprintf (fout, "| %30.30s ", "*** TOTAL TOTAL *** ");
	fprintf (fout, "  %16.2f |\n", tot_extend);
}

void
ProcessBranch (
 void)
{
	char	oldGroup [13], 
			newGroup [13],
			upperCategory [12];
	double	extend		= 0.00, 
			value		= 0.00, 
			br_extend	= 0.00, 
			br_value	= 0.00, 
			tot_extend	= 0.00, 
			cat_extend	= 0.00;
	float	incc_qty	= 0.00,
			br_incc_qty = 0.00;
	int		first_time	= TRUE;

	sprintf (oldGroup, "%12.12s", 	lower);
	sprintf (upperCategory,	"%-11.11s",	&upper [1]);

	strcpy (inmr_rec.co_no, comm_rec.co_no);
	sprintf (inmr_rec.inmr_class, "%-1.1s", lower);
	sprintf (inmr_rec.category, "%-11.11s", &lower [1]);
	cc = find_rec (inmr, &inmr_rec, GTEQ, "r");
	sprintf (newGroup, "%1.1s%-11.11s", inmr_rec.inmr_class, inmr_rec.category);
	while (!cc && strncmp (newGroup, upper, groupLength) <= 0)
	{
		if (strcmp (inmr_rec.category, upperCategory) > 0)
			break;

	    if (strcmp (inmr_rec.supercession, "                "))
	    {
			cc = find_rec (inmr, &inmr_rec, NEXT, "r");
			sprintf (newGroup, "%1.1s%-11.11s", inmr_rec.inmr_class, inmr_rec.category);
			continue;
	    }
		incc_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
	    cc = find_rec (incc, &incc_rec, GTEQ, "r");
	    while (!cc && incc_rec.hhbr_hash == inmr_rec.hhbr_hash)
	    {
			if (!ValidateHash (incc_rec.hhcc_hash))
			{
				cc = find_rec (incc, &incc_rec, NEXT, "r");
				continue;
			}
			fifoError = FALSE;

			if (BY_MODE && !MODE_OK)
			{
				cc = find_rec (incc, &incc_rec, NEXT, "r");
				continue;
			}

			if (!BY_MODE)
				incc_qty = n_dec (incc_rec.closing_stock, inmr_rec.dec_pt);
			else
				incc_qty = GetClosingStock
				 (
					n_dec (incc_rec.closing_stock, inmr_rec.dec_pt),
					incc_rec.hhwh_hash
				);
			incc_qty = n_dec (incc_qty, inmr_rec.dec_pt);

			if (incc_qty <= 0.00)
			{
				cc = find_rec (incc, &incc_rec, NEXT, "r");
				continue;
			}

			dsp_process (" Item: ", inmr_rec.item_no);

			value	=	StockValue
						 (
							inmr_rec.costing_flag,
							comm_rec.est_no,
							inmr_rec.hhbr_hash,
							incc_rec.hhwh_hash,
							incc_qty,
							inmr_rec.dec_pt,
							TRUE
						);

			value	=	twodec (value);
			extend	=	value;
			extend	*=	 (double) n_dec (incc_qty, inmr_rec.dec_pt);
			extend	=	out_cost (extend, inmr_rec.outer_size);

			br_incc_qty 	+= incc_qty;
			br_extend 		+= extend;
			br_value 		+= value;
			cc = find_rec (incc, &incc_rec, NEXT, "r");
	    }

	    /*---------------------------------------
	    | Print if avge_cost or stock on hand	|
	    ---------------------------------------*/
	    if (br_value != 0.00 || br_incc_qty != 0.00)
	    {
			if (strncmp (newGroup, oldGroup, groupLength))
			{
				strcpy (oldGroup, newGroup);
				PrintCategory (!first_time, cat_extend);
				first_time = FALSE;
				cat_extend = 0.00;
			}

			cat_extend += br_extend;
			tot_extend += br_extend;
			fprintf (fout, "| %-16.16s ", inmr_rec.item_no);
			fprintf (fout, "| %-40.40s ", inmr_rec.description);
			sprintf (tmpStr, reportQtyMask, n_dec (br_incc_qty, inmr_rec.dec_pt));
			fprintf (fout, "| %14s ", tmpStr);
			fprintf (fout, "| %13.2f ", br_extend / br_incc_qty);
			fprintf (fout, "| %16.2f |%s\n", br_extend, (fifoError) ? "**" : " ");
			fflush (fout);
	    }
	    br_value = 0.00;
	    br_incc_qty = 0.00;
	    br_extend = 0.00;
	    cc = find_rec (inmr, &inmr_rec, NEXT, "r");
	    sprintf (newGroup, "%1.1s%-11.11s", inmr_rec.inmr_class, inmr_rec.category);
	}
	fprintf (fout, "| %-16.16s ", " ");
	fprintf (fout, "| %-40.40s ", " ");
	fprintf (fout, "| %30.30s ", "*** GROUP TOTAL *** ");
	fprintf (fout, "  %16.2f |\n\n", cat_extend);

	fprintf (fout, "| %-16.16s ", " ");
	fprintf (fout, "| %-40.40s ", " ");
	fprintf (fout, "| %30.30s ", "*** TOTAL TOTAL *** ");
	fprintf (fout, "  %16.2f |\n", tot_extend);
}

/*===============================================================
| Search thru hhcc_hash array for valid hhcc_hashes for branch	|
===============================================================*/
int
ValidateHash (
 long hhcc)
{
	int		i;

	for (i = 0;warehouseHhcc [i] != 0L && warehouseHhcc [i] != hhcc;i++);

	STAKE_COFF = warehouseDate [i];
	IN_STAKE   = warehouseMode [i];

	return (warehouseHhcc [i] != 0L);
}

void
PrintCategory (
 int prt_extend, 
 double cat_extend)
{
	int		len = groupLength - 1;

	/*------------------------------------------------------------
	| If the selected group length < 12, then get the desc.of the|
	| higher (shorter) level category.                           |
	------------------------------------------------------------*/
	strcpy (excf_rec.co_no, comm_rec.co_no);
	if (CHECK_LEN)
		sprintf
		 (
			excf_rec.cat_no, "%-*.*s%-*.*s",
			len, len, inmr_rec.category,
			 (11 - len), (11 - len), " "
		);
	else
		strcpy (excf_rec.cat_no, inmr_rec.category);

	cc = find_rec (excf, &excf_rec, COMPARISON, "r");
	if (cc)
		sprintf (excf_rec.cat_desc, "%40.40s", "No Category description found.");

	expand (err_str, excf_rec.cat_desc);

	fprintf (fout, ".PD| %-111.111s |\n", err_str);

	if (prt_extend)
	{
		fprintf (fout, "| %-16.16s ", " ");
		fprintf (fout, "| %-40.40s ", " ");
		fprintf (fout, "| %30.30s ", "*** GROUP TOTAL *** ");
		fprintf (fout, "  %16.2f |\n", cat_extend);
		fprintf (fout, ".PA\n");
	}
}

/*===========================
| Get closing stock actual. |
===========================*/
double 
GetClosingStock (
 float on_hand, 
 long hhwh_hash)
{
	float	counted = 0.00;

	sttf_rec.hhwh_hash = hhwh_hash;
	sprintf (sttf_rec.location, "%-10.10s", " ");
	cc = find_rec (sttf, &sttf_rec, GTEQ, "r");
	while (!cc && sttf_rec.hhwh_hash == hhwh_hash)
	{
		counted += n_dec (sttf_rec.qty, inmr_rec.dec_pt);
		cc = find_rec (sttf, &sttf_rec, NEXT, "r");
	}
	return (on_hand - (on_hand - counted));
}

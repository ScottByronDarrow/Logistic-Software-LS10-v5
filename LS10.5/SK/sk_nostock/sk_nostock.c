/*=====================================================================
|  Copyright (C) 1996 - 2001 Logistic Software Limited   .            |
|=====================================================================|
| $Id: sk_nostock.c,v 5.2 2001/08/09 09:19:20 scott Exp $
|  Program Name  : (sk_nostock.c)
|  Program Desc  : (Print details of item where incc_closing >= 0)
|                  (and inei_last_cost = 0) 
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 07/04/87         |
|---------------------------------------------------------------------|
| $Log: sk_nostock.c,v $
| Revision 5.2  2001/08/09 09:19:20  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/07/25 02:19:17  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_nostock.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_nostock/sk_nostock.c,v 5.2 2001/08/09 09:19:20 scott Exp $";

#include <pslscr.h>	
#include <twodec.h>	/*  Charater type macros			*/
#include <dsp_screen.h>
#include <dsp_process.h>
#include <ml_std_mess.h>
#include <ml_sk_mess.h>
#include <Costing.h>

#include	"schema"

struct commRecord	comm_rec;
struct ccmrRecord	ccmr_rec;
struct inmrRecord	inmr_rec;
struct inccRecord	incc_rec;

	int		printerNo = 1;

	FILE	*ftmp;

	char	reportType [2];
    double	totalExtendedValue	=	0.00;

#include	<Costing.h>
#include 	<RealCommit.h>

/*=======================
| Function Declarations |
=======================*/
void 	HeadingOutput 		(void);
void 	shutdown_prog 		(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
void 	ReadMisc 			(void);
void 	ProcessData 		(void);
void 	EndReport 			(void);
int  	CheckForZeroFifo 	(long, float);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{
	if (argc != 3)
	{
		print_at (0,0,mlSkMess053,argv [0]);
		return (EXIT_FAILURE);
	}
	sprintf (reportType, "%-1.1s", argv [2]);
	switch (reportType [0])
	{
		case	'Z':
		case	'N':
		case	'A':
		case	'O':
			break;

		default :
			print_at (0,0,mlSkMess053,argv [0]);
			return (EXIT_FAILURE);
	}
		
	printerNo = atoi (argv [1]);

	OpenDB ();

	init_scr ();

	switch (reportType [0])
	{
		case	'Z':
			dsp_screen ("Printing Zero Cost Report", 
					comm_rec.co_no, comm_rec.co_name);
			break;

		case	'N':
			dsp_screen ("Printing Negative stock on hand Report", 
					comm_rec.co_no, comm_rec.co_name);
			break;

		case	'A':
			dsp_screen ("Printing Negative Available stock Report", 
					comm_rec.co_no, comm_rec.co_name);
			break;

		case	'O':
			dsp_screen ("Printing Negative Available + On Order stock Report", 
					comm_rec.co_no, comm_rec.co_name);
			break;

		default :
			dsp_screen ("Printing Unknown stock Report", 
					comm_rec.co_no, comm_rec.co_name);
			break;

	}
	/*=================================
	| Open pipe work file to pformat. |
 	=================================*/
	if ((ftmp = popen ("pformat","w")) == 0)
		file_err (errno,  "pformat", "POPEN");

	HeadingOutput ();
	ProcessData ();
	EndReport ();

	fprintf (ftmp,".EOF\n");

	/*========================= 
	| Program exit sequence	. |
	=========================*/
	pclose (ftmp);

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*===================================
| Start Out Put To Standard Print . |
===================================*/
void
HeadingOutput (
 void)
{
	fprintf (ftmp, ".START%s<%s>\n", DateToString (comm_rec.inv_date), PNAME);
	fprintf (ftmp, ".PI12\n");
	fprintf (ftmp, ".LP%d\n",printerNo);
	fprintf (ftmp, ".13\n");
	fprintf (ftmp, ".L115\n");

	fprintf (ftmp, ".B1\n");
	switch (reportType [0])
	{
		case	'Z':
			fprintf (ftmp, ".EZERO COST REPORT.\n");
			break;

		case	'N':
			fprintf (ftmp, ".ENEGATIVE STOCK ON HAND REPORT.\n");
			break;

		case	'A':
			fprintf (ftmp, ".ENEGATIVE AVAILABLE STOCK REPORT.\n");
			break;

		case	'O':
			fprintf (ftmp, ".ENEGATIVE AVAILABLE + ON ORDER STOCK REPORT.\n");
			break;

		default :
			fprintf (ftmp, ".EUNKNOWN STOCK REPORT.\n");
			break;

	}
	fprintf (ftmp, ".E%s \n",clip (comm_rec.co_name));
	fprintf (ftmp, ".EBranch: %s \n",clip (comm_rec.est_name));
	fprintf (ftmp, ".B1\n");
	fprintf (ftmp, ".EWarehouse: %s \n",clip (comm_rec.cc_name));
	fprintf (ftmp, ".B1\n");
	fprintf (ftmp, ".E AS AT : %s\n",SystemTime ());

	fprintf (ftmp, ".R===============================");
	fprintf (ftmp, "===========================================");
	fprintf (ftmp, "==============");
	fprintf (ftmp, "=============");
	fprintf (ftmp, "==============\n");

	fprintf (ftmp, "===============================");
	fprintf (ftmp, "===========================================");
	fprintf (ftmp, "==============");
	fprintf (ftmp, "=============");
	fprintf (ftmp, "==============\n");

	fprintf (ftmp, "|  CATEGORY |   ITEM NUMBER    ");
	fprintf (ftmp, "|   I T E M    D E S C R I P T I O N       ");
	fprintf (ftmp, "|  AVAILABLE  ");
	fprintf (ftmp, "|    COST    ");
	fprintf (ftmp, "|  EXTENDED. |\n");

	fprintf (ftmp, "|-----------|------------------");
	fprintf (ftmp, "|------------------------------------------");
	fprintf (ftmp, "|-------------");
	fprintf (ftmp, "|------------");
	fprintf (ftmp, "|------------|\n");
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
	abc_dbopen ("data");
	ReadMisc ();

	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no_2");
	open_rec (soic, soic_list, soic_no_fields, "soic_id_no2");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (inmr);
	abc_fclose (incc);
	abc_fclose (soic);
	CloseCosting ();
	abc_dbclose ("data");
}

/*===================================== 
| Get info from commom database file .|
=====================================*/
void
ReadMisc (
 void)
{
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	strcpy (ccmr_rec.co_no,comm_rec.co_no);
	strcpy (ccmr_rec.est_no,comm_rec.est_no);
	strcpy (ccmr_rec.cc_no,comm_rec.cc_no);
	cc = find_rec (ccmr,&ccmr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");

	abc_fclose (ccmr);
}

void
ProcessData (
 void)
{
	double	extend 		= 0.00;
	double	value 		= 0.00;
	float	available 	= 0.00;
	float	avail_ord 	= 0.00;
	float	closing 	= 0.00;
	float	realCommitted;

	/*
	 * Process incc.
	 */
	fflush (ftmp);
	incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	sprintf (incc_rec.sort,"%-28.28s"," ");
	cc = find_rec (incc, &incc_rec, GTEQ, "r");
	while (!cc && incc_rec.hhcc_hash == ccmr_rec.hhcc_hash)
	{
		strcpy (inmr_rec.co_no,comm_rec.co_no);
		sprintf (inmr_rec.item_no,"%-16.16s",incc_rec.sort + 12);

		cc = find_rec (inmr, &inmr_rec, COMPARISON,"r");
		if (strcmp (inmr_rec.supercession,"                ") != 0) 
		{
			cc = find_rec (incc,&incc_rec,NEXT,"r");
			continue;
		}
		if (check_class (inmr_rec.inmr_class))
		{
			cc = find_rec (incc,&incc_rec,NEXT,"r");
			continue;
		}
		if (inmr_rec.inmr_class [0] == 'P' || inmr_rec.inmr_class [0] == 'K')
		{
			cc = find_rec (incc,&incc_rec,NEXT,"r");
			continue;
		}
		value	=	StockValue
					(
						inmr_rec.costing_flag, 
						comm_rec.est_no,
						inmr_rec.hhbr_hash,
						incc_rec.hhwh_hash,
						incc_rec.closing_stock, 
						inmr_rec.dec_pt,
						TRUE
					);

		/*---------------------------------
		| Calculate Actual Qty Committed. |
		---------------------------------*/
		realCommitted = RealTimeCommitted 
						(
							incc_rec.hhbr_hash,
							incc_rec.hhcc_hash
						);

		available = incc_rec.closing_stock - 
					incc_rec.committed - 
					realCommitted -
					incc_rec.backorder - 
					incc_rec.forward;

		available = (float) (twodec (available));

		avail_ord = incc_rec.closing_stock - 
					incc_rec.committed - 
					realCommitted -
					incc_rec.backorder - 
					incc_rec.forward + 
					incc_rec.on_order;

		avail_ord = (float) (twodec (avail_ord));

		closing = (float) (n_dec (incc_rec.closing_stock, inmr_rec.dec_pt));

		switch (reportType [0])
		{
		case 'Z':
			if ((closing > 0.00 && value == 0.00) ||
				 (closing > 0.00 && value != 0.00 &&
					CheckForZeroFifo (incc_rec.hhwh_hash,
					incc_rec.closing_stock)))
			{
		   	    dsp_process (" Item: ", inmr_rec.item_no);
		   	    fprintf (ftmp, "|%-11.11s",inmr_rec.category);
		   	    fprintf (ftmp, "| %-16.16s ",inmr_rec.item_no);
		   	    fprintf (ftmp, "| %-40.40s ",inmr_rec.description);
		   	    fprintf (ftmp, "| %11.2f ",incc_rec.closing_stock);
		   	    fprintf (ftmp, "|%11.2f ", value);
		   	    fprintf (ftmp, "|%11.2f |\n",value * incc_rec.closing_stock);
				totalExtendedValue	+= extend;
			}
			break;

		case 'N':

		   	if (closing < 0.00)
		   	{
		   	    dsp_process (" Item: ", inmr_rec.item_no);
		   	    fprintf (ftmp, "|%-11.11s",inmr_rec.category);
		   	    fprintf (ftmp, "| %-16.16s ",inmr_rec.item_no);
		   	    fprintf (ftmp, "| %-40.40s ",inmr_rec.description);
		   	    fprintf (ftmp, "| %11.2f ",incc_rec.closing_stock);
		   	    fprintf (ftmp, "|%11.2f ", value);
		   	    fprintf (ftmp, "|%11.2f |\n",value * incc_rec.closing_stock);
				totalExtendedValue	+= extend;
		   	}
			break;

		case 'A':
			if (available < 0.00)
			{
		   	    dsp_process (" Item: ", inmr_rec.item_no);
		   	    fprintf (ftmp, "|%-11.11s",inmr_rec.category);
		   	    fprintf (ftmp, "| %-16.16s ",inmr_rec.item_no);
		   	    fprintf (ftmp, "| %-40.40s ",inmr_rec.description);
		   	    fprintf (ftmp, "| %11.2f ", available);
		   	    fprintf (ftmp, "|%11.2f ", value);
		   	    fprintf (ftmp, "|%11.2f |\n",value * incc_rec.closing_stock);
				totalExtendedValue	+= extend;
		   	}
			break;

		case 'O':
			if (avail_ord < 0.00)
			{
		   	    dsp_process (" Item: ", inmr_rec.item_no);
		   	    fprintf (ftmp, "|%-11.11s",inmr_rec.category);
		   	    fprintf (ftmp, "| %-16.16s ",inmr_rec.item_no);
		   	    fprintf (ftmp, "| %-40.40s ",inmr_rec.description);
		   	    fprintf (ftmp, "| %11.2f ",avail_ord);
		   	    fprintf (ftmp, "|%11.2f ", value);
		   	    fprintf (ftmp, "|%11.2f |\n",value * incc_rec.closing_stock);
				totalExtendedValue	+= extend;
			}
			break;
		}
		cc = find_rec (incc, &incc_rec, NEXT, "r");
	}
}

void
EndReport (
 void)
{
	fprintf (ftmp, "|-----------|------------------");
	fprintf (ftmp, "|------------------------------------------");
	fprintf (ftmp, "|-------------");
	fprintf (ftmp, "|------------");
	fprintf (ftmp, "|------------|\n");

	fprintf (ftmp, "|  GRAND TOTAL                 ");
	fprintf (ftmp, "|                                          ");
	fprintf (ftmp, "|             ");
	fprintf (ftmp, "|            ");
	fprintf (ftmp, "|%11.2f |\n", totalExtendedValue);
}

int
CheckForZeroFifo (
	long	hhwhHash,
	float	onHand)
{
	int	err_c = 0;
	float	qtyOnHand = onHand;

	/*----------------------------------
	| find oldest lifo record first or |
	| find newest fifo record first    |
	----------------------------------*/
	err_c = FindIncf (hhwhHash, FALSE, "r");
	if (err_c)
	{
		return (FALSE);
	}

	while (!err_c && incfRec.hhwh_hash == hhwhHash && qtyOnHand > 0.00)
	{
		/*------------------------------------------------------------
		| Check (1) There is a stock date cutoff date (STAKE_COFF)   |
		|       (2) Item is in stock take mode.       (IN_STAKE)     |
		|       (3) FIFO date is greater-than STAKE_COFF.            |
		------------------------------------------------------------*/
		if (STAKE_COFF && IN_STAKE && incfRec.fifo_date > STAKE_COFF)
		{
			err_c = FindIncf (0L, FALSE, "r");
			continue;
		}

		if (incfRec.fifo_qty > 0.00 && incfRec.fifo_cost == 0.00 && incfRec.act_cost)
			return (TRUE);

		qtyOnHand -= incfRec.fifo_qty;

		err_c = FindIncf (0L, FALSE, "r");
	}
	return (FALSE);
}


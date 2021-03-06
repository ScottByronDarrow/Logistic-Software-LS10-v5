/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_age.c,v 5.2 2001/08/09 09:17:59 scott Exp $
|  Program Name  : (sk_age.c) 
|  Program Desc  : (Print Detail stock ageing report (FIFO))
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 07/04/87         |
|---------------------------------------------------------------------|
| $Log: sk_age.c,v $
| Revision 5.2  2001/08/09 09:17:59  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/07/25 02:18:46  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_age.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_age/sk_age.c,v 5.2 2001/08/09 09:17:59 scott Exp $";

#include	<pslscr.h>
#include	<twodec.h>
#include	<ml_std_mess.h>
#include	<ml_sk_mess.h>
#include	<Costing.h>

#define		VALID_FIFO	 (inmr_rec.costing_flag [0] == 'F' || \
			          	  inmr_rec.costing_flag [0] == 'I')
#include 	<dsp_screen.h>
#include 	<dsp_process.h>

#define	BY_MODE 	 (stockTakeMode [0] != ' ')
#define	MODE_OK		 (incc_rec.stat_flag [0] == stockTakeMode [0])
#define	MAX_BR		5
#define	CHECK_LEN	 (groupLength < 12)

#include	"schema"

	struct	commRecord	comm_rec;
	struct	ccmrRecord	ccmr_rec;
	struct	inmrRecord	inmr_rec;
	struct	inccRecord	incc_rec;
	struct	excfRecord	excf_rec;
	struct	sttfRecord	sttf_rec;

	int		printerNumber = 1;
	int		groupLength;
	int		stockNegativeValue	=	FALSE;

	FILE	*ftmp;

	char	lower [13], 
			upper [13]; 

	char	stockTakeMode [2];

	long	moduleDates [6];

/*=======================
| Function Declarations |
=======================*/
void	HeadingOutput	 (void);
void	shutdown_prog	 (void);
void	CloseDB			 (void);
void	OpenDB			 (void);
void	PrintCategory	 (int, double *);
void	ProcessData		 (void);
void	ReadMisc		 (void);
void	SetPeriod		 (void);
int		PeriodIn		 (long);
float	GetClosingStock  (float, long);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{
	char	*sptr;
	/*---------------------------------------
	|	parameters							|
	|	1:	program name					|
	|	2:	printer number					|
	|	3:	lower bound   					|
	|	4:	upper bound   					|
	|	5:	Stock Take Mode   				|
	|	  	' ' - no modes expected		    |
	---------------------------------------*/
	if (argc != 5)
	{
		print_at (0,0, mlSkMess018 ,argv [0]);
		return (EXIT_FAILURE);
	}
	
	printerNumber = atoi (argv [1]);
	sprintf (lower,"%-12.12s",argv [2]);
	sprintf (upper,"%-12.12s",argv [3]);
	sprintf (stockTakeMode,"%-1.1s",argv [4]);

	sptr = chk_env ("SK_VAL_NEGATIVE");
	stockNegativeValue = (sptr == (char *)0) ? FALSE : atoi (sptr);

	OpenDB ();

	moduleDates [0] = comm_rec.inv_date;

	if (strlen (clip (lower)) > strlen (clip (upper)))
		groupLength = strlen (clip (lower));
	else
		groupLength = strlen (clip (upper));

	init_scr ();

	dsp_screen ("Processing : Detail Stock Ageing Report (FIFO)", 
				comm_rec.co_no, comm_rec.co_name);

	IN_STAKE = (BY_MODE) ? TRUE : FALSE;
	
	/*---------------------------------------------
	| Find stock take record for date validation. |
	---------------------------------------------*/
	FindInsc (ccmr_rec.hhcc_hash, stockTakeMode, BY_MODE);

	/*=================================
	| Open pipe work file to pformat. |
 	=================================*/
	if ( (ftmp = popen ("pformat","w")) == 0)
		file_err (errno, "pformat", "popen");

	HeadingOutput ();

	ProcessData ();

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
	int	i;

	i = (BY_MODE) ? 1 : 0;

	fprintf (ftmp, ".START%s<%s>\n", DateToString (comm_rec.inv_date), PNAME);
	fprintf (ftmp, ".LP%d\n",printerNumber);
	fprintf (ftmp, ".%d\n",14 + i);
	fprintf (ftmp, ".L162\n");

	fprintf (ftmp, ".EDETAIL STOCK AGEING REPORT (FIFO)\n");
	fprintf (ftmp, ".EFOR YEAR ENDING (%s)\n", DateToString (moduleDates [0]));
	if (BY_MODE)
		fprintf (ftmp, ".ESTOCK TAKE SELECTION [%s]\n",stockTakeMode);

	fprintf (ftmp, ".E%s \n",clip (comm_rec.co_name));
	fprintf (ftmp, ".EBranch: %s \n",clip (comm_rec.est_name));
	fprintf (ftmp, ".B1\n");
	fprintf (ftmp, ".EWarehouse: %s \n",clip (comm_rec.cc_name));
	fprintf (ftmp, ".B1\n");
	fprintf (ftmp, ".EAS AT : %s\n",SystemTime ());

	fprintf (ftmp, ".R===================");
	fprintf (ftmp, "======================================");
	fprintf (ftmp, "================");
	fprintf (ftmp, "==============");
	fprintf (ftmp, "==============");
	fprintf (ftmp, "==============");
	fprintf (ftmp, "==============");
	fprintf (ftmp, "==============");
	fprintf (ftmp, "================\n");

	fprintf (ftmp, "===================");
	fprintf (ftmp, "======================================");
	fprintf (ftmp, "================");
	fprintf (ftmp, "==============");
	fprintf (ftmp, "==============");
	fprintf (ftmp, "==============");
	fprintf (ftmp, "==============");
	fprintf (ftmp, "==============");
	fprintf (ftmp, "================\n");

	fprintf (ftmp, "|   ITEM NUMBER    ");
	fprintf (ftmp, "|   I T E M    D E S C R I P T I O N   ");
	fprintf (ftmp, "|   T O T A L   ");
	fprintf (ftmp, "| CURRENT YEAR");
	fprintf (ftmp, "| 1 YEAR OLD  ");
	fprintf (ftmp, "| 2 YEARS OLD ");
	fprintf (ftmp, "| 3 YEARS OLD ");
	fprintf (ftmp, "| 4 YEARS OLD ");
	fprintf (ftmp, "| 5+ YEARS OLD|\n");

	fprintf (ftmp, "|------------------");
	fprintf (ftmp, "|--------------------------------------");
	fprintf (ftmp, "|---------------");
	fprintf (ftmp, "|-------------");
	fprintf (ftmp, "|-------------");
	fprintf (ftmp, "|-------------");
	fprintf (ftmp, "|-------------");
	fprintf (ftmp, "|-------------");
	fprintf (ftmp, "|-------------|\n");
	fprintf (ftmp, ".PI12\n");

	fflush (ftmp);
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
	open_rec (excf, excf_list, EXCF_NO_FIELDS, "excf_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no_2");
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
	abc_fclose (inmr);
	abc_fclose (incc);
	abc_fclose (sttf);
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
	int		i,
			firstTime = TRUE,
			fifoRecordFound;

	char	oldGroup [13],
			newGroup [13];

	float	onHand = 0.00,
			needed = 0.00;

	double	extend,
			fifoValue,
			totalItem [7],
			totalGroup [7],
			totalGrand [7],
			addCost = 0.00;

	int		fifoError = FALSE;

	SetPeriod ();

	for (i = 0;i < 7;i++)
		totalGrand [i] = 0.00;

	for (i = 0;i < 7;i++)
		totalGroup [i] = 0.00;

	for (i = 0;i < 7;i++)
		totalItem [i] = 0.00;

	/*-----------------------
	|	read first incc	|
	-----------------------*/
	fflush (ftmp);
	incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	sprintf (incc_rec.sort,"%-12.12s%-16.16s",lower, " ");
	cc = find_rec (incc,&incc_rec,GTEQ,"r");

	while (!cc && incc_rec.hhcc_hash == ccmr_rec.hhcc_hash)
	{
		fifoError = FALSE;

		if (BY_MODE && !MODE_OK)
		{
			cc = find_rec (incc,&incc_rec,NEXT,"r");
			continue;
		}
		strcpy (inmr_rec.co_no,comm_rec.co_no);
		sprintf (inmr_rec.item_no,"%-16.16s",incc_rec.sort + 12);
		cc = find_rec (inmr, &inmr_rec, COMPARISON,"r");

		if (cc || !VALID_FIFO)
		{
			cc = find_rec (incc,&incc_rec,NEXT,"r");
			continue;
		}
		/*
		dsp_process (" Item: ", inmr_rec.item_no);
		*/

		sprintf (newGroup,"%1.1s%-11.11s", inmr_rec.inmr_class, inmr_rec.category);

		if (strncmp (newGroup, upper,groupLength) > 0)
			break;
		
		onHand = (BY_MODE) ? GetClosingStock (incc_rec.closing_stock,
						      incc_rec.hhwh_hash) : incc_rec.closing_stock;

		needed = onHand;

		fifoRecordFound = TRUE;

		cc = FindIncf (incc_rec.hhwh_hash, FALSE, "r");
		while (onHand > 0.00 && !cc && incfRec.hhwh_hash == incc_rec.hhwh_hash)
		{
			if (!ValidStDate (incfRec.fifo_date))
			{
				cc = FindIncf (0L,FALSE,"r");
				continue;
			}

			if (n_dec (incfRec.fifo_qty,inmr_rec.dec_pt) <= 0.00)
			{
				cc = FindIncf (0L,FALSE,"r");
				continue;
			}

			if (onHand < n_dec (incfRec.fifo_qty,inmr_rec.dec_pt))
			{
				incfRec.fifo_qty = n_dec (onHand, inmr_rec.dec_pt);
				onHand = 0.00;
			}
			else
				onHand -= n_dec (incfRec.fifo_qty, inmr_rec.dec_pt);

			if (fifoRecordFound)
			{
				if (strncmp (newGroup,oldGroup,groupLength))
				{
					strcpy (oldGroup,newGroup);
					PrintCategory (!firstTime,totalGroup);
					firstTime = FALSE;
					for (i = 0;i < 7;i++)
						totalGroup [i] = 0.00;
				}
				fifoRecordFound = FALSE;
			}

			i = PeriodIn (incfRec.fifo_date);

			if (++i > 0)
			{
				fifoValue 	= (double)	n_dec 
										 (
											incfRec.fifo_qty, 
											inmr_rec.dec_pt
										);
				extend 			= twodec (incfRec.fifo_cost) * fifoValue;
				extend 			= out_cost (extend, inmr_rec.outer_size);
				totalItem [0]	+= extend;
				totalItem [i]	+= extend;
				totalGroup [0] 	+= extend;
				totalGroup [i] 	+= extend;
				totalGrand [0] 	+= extend;
				totalGrand [i] 	+= extend;
			}
			cc = FindIncf (0L,FALSE,"r");
		}
		if (onHand > 0.00)
		{
			fifoError = TRUE;
			strcpy (ineiRec.est_no, ccmr_rec.est_no);
			ineiRec.hhbr_hash = inmr_rec.hhbr_hash;
			cc = find_rec (inei,&ineiRec,COMPARISON,"r");
			if (cc)
				file_err (cc, inei,"DBFIND");
		

			addCost	= 	CheckIncf 
					  	(
					 		incc_rec.hhwh_hash,
						 	(inmr_rec.costing_flag [0] == 'F') ? TRUE : FALSE,
							needed,
							inmr_rec.dec_pt
					 	);
					       
			i = PeriodIn (comm_rec.inv_date);

			if (++i > 0)
			{
				extend = out_cost (addCost / onHand, inmr_rec.outer_size);

				extend = twodec (extend);
				extend *= (double) onHand;
				extend = twodec (extend);

				totalItem [0] += extend;
				totalItem [i] += extend;
				totalGroup [0] += extend;
				totalGroup [i] += extend;
				totalGrand [0] += extend;
				totalGrand [i] += extend;
			}
		}
		if (needed < 0.00 && stockNegativeValue)
		{
			cc = FindInei (inmr_rec.hhbr_hash, ccmr_rec.est_no, "r");
			if (cc)
				file_err (cc, inei,"DBFIND");
					       
			i = PeriodIn (comm_rec.inv_date);

			extend	= out_cost (ineiRec.last_cost, inmr_rec.outer_size);
			extend	= twodec (extend);
			extend	*= (double) onHand;
			extend	= twodec (extend);

			totalItem [0]	+= extend;
			totalItem [i]	+= extend;
			totalGroup [0]	+= extend;
			totalGroup [i]	+= extend;
			totalGrand [0]	+= extend;
			totalGrand [i]	+= extend;
		}

		/*-------------------------------
		| fifo records found for item	|
		-------------------------------*/
		if (!fifoRecordFound && totalItem [0] != 0.00)
		{
			fprintf (ftmp, "| %-16.16s ",inmr_rec.item_no);
			fprintf (ftmp, "|%-38.38s",inmr_rec.description);
			if (totalItem [0] == 0.00)
				fprintf (ftmp, "| %13.13s "," ");
			else
				fprintf (ftmp, "|%14.2f ",totalItem [0]);

			for (i = 1;i < 7;i++)
			{
				if (totalItem [i] == 0.00)
					fprintf (ftmp, "| %11.11s "," ");
				else
					fprintf (ftmp, "|%12.2f ",totalItem [i]);
			}
			fprintf (ftmp, "|%s\n", (fifoError) ? "**" : " ");
			for (i = 0;i < 7;i++)
				totalItem [i] = 0.00;
		}

		cc = find_rec (incc,&incc_rec,NEXT,"r");
	}
	fprintf (ftmp, "| %-16.16s "," ");
	fprintf (ftmp, "|%38.38s","*** GROUP TOTAL ***");
	if (totalGroup [0] == 0.00)
		fprintf (ftmp, "| %13.13s "," ");
	else
		fprintf (ftmp, "|%14.2f ",totalGroup [0]);

	for (i = 1;i < 7;i++)
	{
		if (totalGroup [i] == 0.00)
			fprintf (ftmp, "| %11.11s "," ");
		else
			fprintf (ftmp, "|%12.2f ",totalGroup [i]);
	}
	fprintf (ftmp, "|\n");

	fprintf (ftmp, "| %-16.16s "," ");
	fprintf (ftmp, "|%38.38s","*** STOCK TOTAL ***");
	if (totalGrand [0] == 0.00)
		fprintf (ftmp, "| %13.13s "," ");
	else
		fprintf (ftmp, "|%14.2f ",totalGrand [0]);

	for (i = 1;i < 7;i++)
	{
		if (totalGrand [i] == 0.00)
			fprintf (ftmp, "| %11.11s "," ");
		else
			fprintf (ftmp, "|%12.2f ",totalGrand [i]);
	}
	fprintf (ftmp, "|\n");
}

void
SetPeriod (void)
{
	int	i;
	int	yr;

	for (i = 1,yr = 1; i < 6; i++, yr++)
	{
		moduleDates [i] = comm_rec.inv_date; 
		moduleDates [i] = AddYears (moduleDates [i], -yr);
	}
}
int
PeriodIn (
	long	fifoDate)
{
	int	i;

	if (fifoDate > moduleDates [0])
		return (-1);


	for (i = 0;i < 6;i++)
		if (moduleDates [i] >= fifoDate && fifoDate > moduleDates [i + 1])
			return (i);
	return (5);
}

void
PrintCategory (
	int 	printValue, 
	double *totalGroup)
{
	int	i;
	int	len = groupLength - 1;

	/*------------------------------------------------------------
	| If the selected group length < 12, then get the desc.of the|
	| higher (shorter) level category.                           |
	------------------------------------------------------------*/
	strcpy (excf_rec.co_no, comm_rec.co_no);
	if (CHECK_LEN)
		sprintf (excf_rec.cat_no,"%-*.*s%-*.*s",len,len,
				inmr_rec.category, (11 - len), (11 - len)," ");
	else
		strcpy (excf_rec.cat_no,inmr_rec.category);

	cc = find_rec (excf, &excf_rec, COMPARISON, "r");
	if (cc)
		sprintf (excf_rec.cat_desc, "%40.40s","No Category description found.");

	expand (err_str,excf_rec.cat_desc);

	fprintf (ftmp, ".PD| %-155.155s |\n",err_str);


	if (printValue)
	{
		fprintf (ftmp, "| %-16.16s "," ");
		fprintf (ftmp, "|%38.38s","*** GROUP TOTAL ***");
		if (totalGroup [0] == 0.00)
			fprintf (ftmp, "| %13.13s "," ");
		else
			fprintf (ftmp, "|%14.2f ",totalGroup [0]);

		for (i = 1;i < 7;i++)
		{
			if (totalGroup [i] == 0.00)
				fprintf (ftmp, "| %11.11s "," ");
			else
				fprintf (ftmp, "|%12.2f ",totalGroup [i]);
		}
		fprintf (ftmp, "|\n.PA\n");
	}
}

/*===========================
| Get closing stock actual. |
===========================*/
float 
GetClosingStock (
	 float	onHand, 
	 long	 hhwhHash)
{
	float	counted = 0.00;

	sttf_rec.hhwh_hash = hhwhHash;
	sprintf (sttf_rec.location, "%-10.10s", " ");
	cc = find_rec (sttf, &sttf_rec, GTEQ, "r");
	while (!cc && sttf_rec.hhwh_hash == hhwhHash)
	{
		counted += sttf_rec.qty;
		cc = find_rec (sttf, &sttf_rec, NEXT, "r");
	}

	return (onHand - (onHand - counted));
}

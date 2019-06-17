/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_catage.c,v 5.2 2001/08/09 09:18:13 scott Exp $
|  Program Name  : (sk_catage.c)
|  Program Desc  : (Print Summary Stock ageing report)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 24/03/87         |
|---------------------------------------------------------------------|
|  Date Modified : (26/07/89)      | Modified  by  : Fui Choo Yap.    |
|---------------------------------------------------------------------|
| $Log: sk_catage.c,v $
| Revision 5.2  2001/08/09 09:18:13  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/07/25 02:18:53  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_catage.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_catage/sk_catage.c,v 5.2 2001/08/09 09:18:13 scott Exp $";

#include	<pslscr.h>
#include	<twodec.h>

#define	BY_MODE 	 (stockTakeMode [0] != ' ')
#define	MODE_OK		 (incc_rec.stat_flag [0] == stockTakeMode [0])
#define	FIFO		 (inmr_rec.costing_flag [0] == 'F')
#define	LIFO		 (inmr_rec.costing_flag [0] == 'I')

#include 	<dsp_screen.h>
#include 	<dsp_process.h>
#include 	<ml_std_mess.h>
#include 	<ml_sk_mess.h>
#include 	<Costing.h>

#include	"schema"

	struct	commRecord	comm_rec;
	struct	ccmrRecord	ccmr_rec;
	struct	inmrRecord	inmr_rec;
	struct	inccRecord	incc_rec;
	struct	excfRecord	excf_rec;
	struct	sttfRecord	sttf_rec;

	int		printerNumber = 1;
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

	/*-------------------------------
	|	parameters					|
	|	1:	program name			|
	|	2:	printer number			|
	|	3:	lower bound   			|
	|	4:	upper bound   			|
	|	5:	Stock Take Mode   		|
	|	  	' ' - no modes expected |
	-------------------------------*/
	if (argc != 5)
	{
		print_at (0,0,mlSkMess018,argv [0]);
		return (EXIT_FAILURE);
	}
	
	printerNumber = atoi (argv [1]);
	sprintf (lower,"%-12.12s",argv [2]);
	sprintf (upper,"%-12.12s",argv [3]);
	sprintf (stockTakeMode,"%-1.1s",argv [4]);

	sptr = chk_env ("SK_VAL_NEGATIVE");
	stockNegativeValue = (sptr == (char *)0) ? FALSE : atoi (sptr);

	/*======================
	| Open database files. |
	======================*/
	OpenDB ();

	/*=======================
	| Get End of year Date. |
	=======================*/
	moduleDates [0] = comm_rec.inv_date;

	init_scr ();

	dsp_screen ("Now Processing : Summary Stock Age Report (FIFO)", 
				comm_rec.co_no, comm_rec.co_name);


	IN_STAKE = (BY_MODE) ? TRUE : FALSE;

	/*---------------------------------------------
	| Find stock take record for date validation. |
	---------------------------------------------*/
	FindInsc (ccmr_rec.hhcc_hash, stockTakeMode, BY_MODE);

	/*=================================
	| Open pipe work file to pformat. |
 	=================================*/
	if ((ftmp = popen ("pformat","w")) == 0)
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);

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
	fprintf (ftmp, ".SO\n");
	fprintf (ftmp, ".%d\n",14 + i);
	fprintf (ftmp, ".L110\n");

	fprintf (ftmp, ".B1\n");
	fprintf (ftmp, ".ESUMMARY STOCK AGE REPORT\n");
	if (BY_MODE)
		fprintf (ftmp, ".ESTOCK TAKE SELECTION [%s]\n",stockTakeMode);

	fprintf (ftmp, ".E%s \n",clip (comm_rec.co_name));
	fprintf (ftmp, ".EBranch: %s \n",clip (comm_rec.est_name));
	fprintf (ftmp, ".B1\n");
	fprintf (ftmp, ".EWarehouse: %s \n",clip (comm_rec.cc_name));
	fprintf (ftmp, ".B1\n");
	fprintf (ftmp, ".E AS AT : %s\n",SystemTime ());

	fprintf (ftmp, ".R================");
	fprintf (ftmp, "==============");
	fprintf (ftmp, "==============");
	fprintf (ftmp, "==============");
	fprintf (ftmp, "==============");
	fprintf (ftmp, "==============");
	fprintf (ftmp, "===============\n");

	fprintf (ftmp, "================");
	fprintf (ftmp, "==============");
	fprintf (ftmp, "==============");
	fprintf (ftmp, "==============");
	fprintf (ftmp, "==============");
	fprintf (ftmp, "==============");
	fprintf (ftmp, "===============\n");

	fprintf (ftmp, "|   T O T A L   ");
	fprintf (ftmp, "| CURRENT YEAR");
	fprintf (ftmp, "| 1 YEAR OLD  ");
	fprintf (ftmp, "| 2 YEARS OLD ");
	fprintf (ftmp, "| 3 YEARS OLD ");
	fprintf (ftmp, "| 4 YEARS OLD ");
	fprintf (ftmp, "| 5+ YEARS OLD|\n");

	fprintf (ftmp, "|---------------");
	fprintf (ftmp, "|-------------");
	fprintf (ftmp, "|-------------");
	fprintf (ftmp, "|-------------");
	fprintf (ftmp, "|-------------");
	fprintf (ftmp, "|-------------");
	fprintf (ftmp, "|-------------|\n");
	fprintf (ftmp, ".PI12\n");
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

	/*===================================================
	| Read common terminal record & cost centre master. |
	===================================================*/
	ReadMisc ();

	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no_2");
	open_rec (excf, excf_list, EXCF_NO_FIELDS, "excf_id_no");
	open_rec (sttf, sttf_list, STTF_NO_FIELDS, "sttf_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (inei);
	abc_fclose (ccmr);
	abc_fclose (excf);
	abc_fclose (inmr);
	abc_fclose (incc);
	abc_fclose (incf);
	abc_fclose (sttf);
	abc_fclose (insc);
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
	int	i;
	char	oldGroup [13],
			newGroup [13];

	int		firstTime = TRUE;

	float	onHand = 0.00,
			needed = 0.00;

	double	extend,
			fifoValue,
			totalItem [7],
			totalGroup [7],
			totalGrand [7],
			addCost = 0.00;

	SetPeriod ();

	for (i = 0;i < 7;i++)
	{
		totalGrand [i]	= 0.00;
		totalGroup [i]	= 0.00;
		totalItem  [i]	= 0.00;
	}
	/*-----------------------
	|	read first incc	    |
	-----------------------*/
	incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	sprintf (incc_rec.sort,"%-12.12s,%-16.16s",lower, " ");
	cc = find_rec (incc,&incc_rec,GTEQ,"r");

	sprintf (oldGroup,"%-12.12s",lower);
    
	/*----------------
	| loop thru incc |
	----------------*/
	while (!cc && incc_rec.hhcc_hash == ccmr_rec.hhcc_hash)
	{
		if (BY_MODE && !MODE_OK)
		{
			cc = find_rec (incc,&incc_rec,NEXT,"r");
			continue;
		}

		strcpy (inmr_rec.co_no, comm_rec.co_no);
		sprintf (inmr_rec.item_no, "%-16.16s",incc_rec.sort + 12);
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		if (cc || (!FIFO && !LIFO))
		{
			cc = find_rec (incc,&incc_rec,NEXT,"r");
			continue;
		}
		dsp_process (" Item: ", inmr_rec.item_no);

		sprintf (newGroup,"%1.1s%-11.11s", inmr_rec.inmr_class, inmr_rec.category);
		if (strncmp (newGroup, upper,12) > 0)
			break;

		onHand = (BY_MODE) ? 	GetClosingStock 
							 	(
									incc_rec.closing_stock, incc_rec.hhwh_hash
								) : incc_rec.closing_stock;

		needed = onHand;

		if (firstTime)
		{
			firstTime = FALSE;
			PrintCategory (FALSE,totalGroup);
			strcpy (oldGroup, newGroup);
		}

		if (strcmp (newGroup,oldGroup))
		{
			strcpy (oldGroup, newGroup);
			PrintCategory (TRUE, totalGroup);

			for (i = 0;i < 7;i++)
				totalGroup [i] = 0.00;
		}

		for (i = 0;i < 7;i++)
			totalItem [i] = 0.00;

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
			cc = FindInei (inmr_rec.hhbr_hash, ccmr_rec.est_no, "r");
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
		if (onHand < 0.00 && stockNegativeValue)
		{
			cc = FindInei (inmr_rec.hhbr_hash, ccmr_rec.est_no, "r");
			if (cc)
				file_err (cc, inei,"DBFIND");
		
			i = PeriodIn (comm_rec.inv_date);

			if (++i > 0)
			{
				extend = out_cost (ineiRec.last_cost, inmr_rec.outer_size);
				extend	= twodec (extend);
				extend	*= (double) onHand;
				extend	= twodec (extend);

				totalItem 	[0] += extend;
				totalItem 	[i] += extend;
				totalGroup 	[0] += extend;
				totalGroup 	[i] += extend;
				totalGrand 	[0] += extend;
				totalGrand 	[i] += extend;
			}
		}
		cc = find_rec (incc,&incc_rec,NEXT,"r");
	}
	
	if (totalGroup [0] == 0.00)
		fprintf (ftmp, "| %13.13s "," ");
	else
		fprintf (ftmp, "|%14.2f ",totalGroup [0]);

	for (i = 1;i < 7;i++)
		if (totalGroup [i] == 0.00)
			fprintf (ftmp, "| %11.11s "," ");
		else
			fprintf (ftmp, "|%12.2f ",totalGroup [i]);
	fprintf (ftmp, "|\n");

	fflush (ftmp);
	fprintf (ftmp, "|===============");
	fprintf (ftmp, "|=============");
	fprintf (ftmp, "|=============");
	fprintf (ftmp, "|=============");
	fprintf (ftmp, "|=============");
	fprintf (ftmp, "|=============");
	fprintf (ftmp, "|=============|\n");

	if (totalGrand [0] == 0.00)
		fprintf (ftmp, "| %13.13s "," ");
	else
		fprintf (ftmp, "|%14.2f ",totalGrand [0]);

	for (i = 1;i < 7;i++)
		if (totalGrand [i] == 0.00)
			fprintf (ftmp, "| %11.11s "," ");
		else
			fprintf (ftmp, "|%12.2f ",totalGrand [i]);
	fprintf (ftmp, "|\n");
	fflush (ftmp);
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
 long fifo_date)
{
	int	i;

	if (fifo_date > moduleDates [0])
		return (-1);

	for (i = 0;i < 6;i++)
		if (moduleDates [i] >= fifo_date && fifo_date > moduleDates [i + 1])
			return (i);

	return (5);
}

void
PrintCategory (
	int		prt_value, 
	double *totalGroup)
{
	int	i;

	strcpy (excf_rec.co_no, comm_rec.co_no);
	strcpy (excf_rec.cat_no,inmr_rec.category);
	cc = find_rec (excf, &excf_rec, COMPARISON, "r");
	if (cc)
		sprintf (excf_rec.cat_desc, "%40.40s", ML (mlStdMess220));

	expand (err_str, excf_rec.cat_desc);
	
	if (prt_value)
	{
		if (totalGroup [0] == 0.00)
			fprintf (ftmp, "| %13.13s "," ");
		else
			fprintf (ftmp, "|%14.2f ",totalGroup [0]);

		for (i = 1;i < 7;i++)
			if (totalGroup [i] == 0.00)
				fprintf (ftmp, "| %11.11s "," ");
			else
				fprintf (ftmp, "|%12.2f ",totalGroup [i]);

		fprintf (ftmp, "|\n");
		fprintf (ftmp, "|---------------");
		fprintf (ftmp, "|-------------");
		fprintf (ftmp, "|-------------");
		fprintf (ftmp, "|-------------");
		fprintf (ftmp, "|-------------");
		fprintf (ftmp, "|-------------");
		fprintf (ftmp, "|-------------|\n");
		fprintf (ftmp, ".LRP3\n");
	}
	fprintf (ftmp, "| %s%18.18s|\n",err_str, " ");
}

/*===========================
| Get closing stock actual. |
===========================*/
float 
GetClosingStock (
 float onHand, 
 long hhwh_hash)
{
	float	counted = 0.00;

	sttf_rec.hhwh_hash = hhwh_hash;
	sprintf (sttf_rec.location, "%-10.10s", " ");
	cc = find_rec (sttf, &sttf_rec, GTEQ, "r");
	while (!cc && sttf_rec.hhwh_hash == hhwh_hash)
	{
		counted += sttf_rec.qty;
		cc = find_rec (sttf, &sttf_rec, NEXT, "r");
	}

	return (onHand - (onHand - counted));
}

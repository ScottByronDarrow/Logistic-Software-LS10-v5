/*=====================================================================
|  Copyright (C) 1999 - 2000 Logistic Software Limited.               |
|=====================================================================|
|  Program Name  : (sk_selage.c   )                                   |
|  Program Desc  : (Print Selective detail stock ageing report (FIFO) |
|                  (                                                ) |
|                  (                                                ) |
|---------------------------------------------------------------------|
|  Date Written  : (DD/MM/YYYY)    | Author      :                    |
|---------------------------------------------------------------------|
|  Date Modified : (DD/MM/YYYY)    | Modified By :                    |
|   Comments     :                                                    |
|  (DD/MM/YYYY)  :                                                    |
|                :                                                    |
|                :                                                    |
|                                                                     |
| $Log: sk_selage.c,v $
| Revision 5.2  2001/08/09 09:19:52  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/07/25 02:19:32  scott
| Update - LS10.5
|
| Revision 4.2  2001/04/06 02:23:02  cha
| Updated AGAIN!!!!! for ORACLE
|
| Revision 4.1  2001/04/05 00:50:18  scott
| Updated to use LTEQ and PREVIOUS calls to work with SQL and CISAM
|
| Revision 4.0  2001/03/09 02:38:48  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.0  2000/10/10 12:21:13  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:11:53  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.9  2000/07/10 01:53:42  scott
| Updated to replace "@ (" with "@(" to ensure psl_what works correctly
|
| Revision 1.8  2000/02/09 05:44:30  scott
| Updated stock valuations to use new common Stock valuation routine. In addition all costing methods are supported. F(ifo) L(I)fo L(ast) S(T)andard P(revious) S(erial) A(verage). All report balance to the absolute cent, what a treat.
|
| Revision 1.7  1999/12/06 01:31:15  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.6  1999/11/03 07:32:32  scott
| Updated to fix small compile errors due to -Wall flag being set.
|
| Revision 1.5  1999/10/13 02:42:13  nz
| Updated to ensure read_comm was in correct place.
|
| Revision 1.4  1999/10/12 21:20:41  scott
| Updated by Gerry from ansi project.
|
| Revision 1.3  1999/10/08 05:32:51  scott
| First Pass checkin by Scott.
|
| Revision 1.2  1999/06/20 05:20:41  scott
| Updated to add log for cvs + remove old read_comm + fixed all warnings.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_selage.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_selage/sk_selage.c,v 5.2 2001/08/09 09:19:52 scott Exp $";

#include	<pslscr.h>
#include	<twodec.h>
#include	<ml_std_mess.h>
#include	<ml_sk_mess.h>
#include	<Costing.h>

#define		VALID_FIFO	 (inmr_rec.costing_flag [0] == 'F' || \
			          	  inmr_rec.costing_flag [0] == 'I')

#define		BY_YEAR		 (yearMonthFlag [0] == 'Y' || yearMonthFlag [0] == 'y')
#define		BY_MONTH	 (yearMonthFlag [0] == 'M' || yearMonthFlag [0] == 'm')

#include 	<dsp_screen.h>
#include 	<dsp_process.h>

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

	char	lowerGroup [13], 
			upperGroup [13],
			yearMonthFlag [2],
			lowerYearMonth [8],
			upperYearMonth [8]; 

	long	moduleDates [6];
	short	tmp_dmy [3];

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
void	SetPeriod		 (int);
int		PeriodIn		 (long);

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
	|	5:	By M(onth) Y(ear)               |
	|	6:	Start Month/Year                |
	|	7:	End Month/Year                  |
	---------------------------------------*/
	if (argc != 7)
	{
		print_at (0,0, "Usage : %s [LPNO] [StartGroup] [EndGroup] [Y(ear)/M(onth)] [StartMonth/Year] [EndMonth/Year]" ,argv [0]);
		return (EXIT_FAILURE);
	}
	
	printerNumber = atoi (argv [1]);
	sprintf (lowerGroup,	"%-12.12s",	argv [2]);
	sprintf (upperGroup,	"%-12.12s",	argv [3]);
	sprintf (yearMonthFlag,	"%-1.1s",	argv [4]);
	sprintf (lowerYearMonth,"%-7.7s",	argv [5]);
	sprintf (upperYearMonth,"%-7.7s",	argv [6]);

	sptr = chk_env ("SK_VAL_NEGATIVE");
	stockNegativeValue = (sptr == (char *)0) ? FALSE : atoi (sptr);

	OpenDB ();

	tmp_dmy [0] = 1;
	tmp_dmy [1] = atoi (upperYearMonth);
	tmp_dmy [2] = atoi (upperYearMonth + 3);
	moduleDates [0] = DMYToDate (tmp_dmy [0], tmp_dmy [1], tmp_dmy [2]);

	if (strlen (clip (lowerGroup)) > strlen (clip (upperGroup)))
		groupLength = strlen (clip (lowerGroup));
	else
		groupLength = strlen (clip (upperGroup));

	init_scr ();

	dsp_screen ("Processing : Detail Stock Ageing Report (FIFO)", 
				comm_rec.co_no, comm_rec.co_name);

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
	fprintf (ftmp, ".START%s<%s>\n", DateToString (comm_rec.inv_date), PNAME);
	fprintf (ftmp, ".LP%d\n",printerNumber);
	fprintf (ftmp, ".%d\n",14);
	fprintf (ftmp, ".L162\n");

	fprintf (ftmp, ".EDETAIL STOCK AGEING REPORT (FIFO)\n");
	fprintf (ftmp, ".EREPORT BY %s FROM MONTH/YEAR : %s TO MONTH/YEAR : %s\n", 
				 (BY_YEAR) ? "YEAR" : "MONTH", lowerYearMonth, upperYearMonth);
	
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
	if (BY_YEAR)
	{
		fprintf (ftmp, "| CURRENT YEAR");
		fprintf (ftmp, "| 1 YEAR OLD  ");
		fprintf (ftmp, "| 2 YEARS OLD ");
		fprintf (ftmp, "| 3 YEARS OLD ");
		fprintf (ftmp, "| 4 YEARS OLD ");
		fprintf (ftmp, "| 5+ YEARS OLD|\n");
	}
	else
	{
		fprintf (ftmp, "| 1-3 MONTHS  ");
		fprintf (ftmp, "| 4-6 MONTHS  ");
		fprintf (ftmp, "| 7-9 MONTHS  ");
		fprintf (ftmp, "|10-12 MONTHS ");
		fprintf (ftmp, "|12-15 MONTHS ");
		fprintf (ftmp, "| 15+ MONTHS  |\n");
	}

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

	SetPeriod (BY_YEAR);

	for (i = 0;i < 7;i++)
		totalGrand [i] = 0.00;

	for (i = 0;i < 7;i++)
		totalGroup [i] = 0.00;

	for (i = 0;i < 7;i++)
		totalItem [i] = 0.00;

	/*-------------------
	|	read first incc	|
	-------------------*/
	fflush (ftmp);
	incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	sprintf (incc_rec.sort,"%-12.12s%-16.16s",lowerGroup, " ");
	cc = find_rec (incc,&incc_rec,GTEQ,"r");

	while (!cc && incc_rec.hhcc_hash == ccmr_rec.hhcc_hash)
	{
		fifoError = FALSE;

		strcpy (inmr_rec.co_no,comm_rec.co_no);
		sprintf (inmr_rec.item_no,"%-16.16s",incc_rec.sort + 12);
		cc = find_rec (inmr, &inmr_rec, COMPARISON,"r");

		if (cc || !VALID_FIFO)
		{
			cc = find_rec (incc,&incc_rec,NEXT,"r");
			continue;
		}

		if (cc)
			file_err (cc, inmr, "DBFIND");

		dsp_process (" Item: ", inmr_rec.item_no);

		sprintf (newGroup,"%1.1s%-11.11s", 	inmr_rec.inmr_class,
						   					inmr_rec.category);

		if (strncmp (newGroup, upperGroup,groupLength) > 0)
			break;
		
		onHand = incc_rec.closing_stock;

		needed = onHand;

		fifoRecordFound = TRUE;

		cc = FindIncf (incc_rec.hhwh_hash, FALSE, "r");
		while (onHand > 0.00 && !cc && incfRec.hhwh_hash == incc_rec.hhwh_hash)
		{
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
					strcpy (oldGroup, newGroup);
					PrintCategory (!firstTime, totalGroup);
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
			cc = FindIncf (0L, FALSE, "r");
		}
		if (onHand > 0.00)
		{
			fifoError = TRUE;
			cc = FindInei (inmr_rec.hhbr_hash,ccmr_rec.est_no, "r");
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
			cc = FindInei (inmr_rec.hhbr_hash,ccmr_rec.est_no, "r");
			if (cc)
				file_err (cc, inei,"DBFIND");
		
			i = PeriodIn (comm_rec.inv_date);

			if (++i > 0)
			{
				extend = out_cost (ineiRec.last_cost, inmr_rec.outer_size);

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
SetPeriod (
 int	CalcYear)
{
	int		tmp_dmy [3];
	int	i;
	int	yr;
	int	mth;

	if (CalcYear)
	{
		DateToDMY (moduleDates [0], &tmp_dmy [0], &tmp_dmy [1], &tmp_dmy [2]);
		yr = tmp_dmy [2];
		for (i = 1,yr--;i < 6;i++,yr--)
		{
			tmp_dmy [2] = yr;
			moduleDates [i] = DMYToDate (tmp_dmy [0], tmp_dmy [1], tmp_dmy [2]);
		}
	}
	else
	{
		DateToDMY (moduleDates [0], &tmp_dmy [0], &tmp_dmy [1], &tmp_dmy [2]);
		mth = tmp_dmy [1];
		for (i = 1;i < 6;i++)
		{
			mth -= 3;
			if (mth < 1)
			{
				mth += 12;
				tmp_dmy [2]--;
			}
			tmp_dmy [1] = mth;
			moduleDates [i] = DMYToDate (tmp_dmy [0], tmp_dmy [1], tmp_dmy [2]);
		}
	}
}
	
int
PeriodIn (
 long fifoDate)
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
	int		printValue,
	double	totalGroup [6])
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
	      	sprintf (excf_rec.cat_desc, "%40.40s",
					"No Category description found.");

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

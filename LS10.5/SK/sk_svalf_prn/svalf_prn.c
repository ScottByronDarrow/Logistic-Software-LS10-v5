/*=====================================================================
|  Copyright (C) 1996 - 2001 Logistic Software Limited.               |
|=====================================================================|
| $Id: svalf_prn.c,v 5.2 2001/08/09 09:20:16 scott Exp $
|  Program Name  : (sk_svalf_prn.c)
|  Program Desc  : (Print Company Summary Stock Valuation)
|---------------------------------------------------------------------|
|  Author        : Roger Gibbison  | Date Written  : 24/03/87         |
|---------------------------------------------------------------------|
| $Log: svalf_prn.c,v $
| Revision 5.2  2001/08/09 09:20:16  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/07/25 02:19:38  scott
| Update - LS10.5
|
| Revision 4.1  2001/04/23 10:41:20  scott
| Updated to add app.schema - removes code related to tables from program as it
| allows for better quality contol.
| Updated to perform routine maintenance to ensure standards are maintained.
| Updated to remove usage of old include files.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: svalf_prn.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_svalf_prn/svalf_prn.c,v 5.2 2001/08/09 09:20:16 scott Exp $";

#include	<pslscr.h>
#include	<twodec.h>
#include 	<dsp_screen.h>
#include 	<dsp_process.h>
#include 	<ml_sk_mess.h>
#include 	<ml_std_mess.h>
#include 	<Costing.h>

#include	"schema"

struct commRecord	comm_rec;
struct ccmrRecord	ccmr_rec;
struct inmrRecord	inmr_rec;
struct inccRecord	incc_rec;
struct excfRecord	excf_rec;

	int		printerNumber 	= 1,
			loopCounter 	= 0;

	FILE	*ftmp;

	long	hhccHash [100];
	char	branchNo [100] [3];
	char	warehouseNo [100] [3];

	char	lower [13], upper [13];

	long	yearEnd [6];

/*=======================
| Function Declarations |
=======================*/
void 	HeadingOutput 		 (void);
void 	OpenDB 				 (void);
void 	CloseDB 			 (void);
void 	ProcessData 		 (long, char *, char *);
void 	SetPeriod 			 (void);
int  	GetPeriodNo 		 (long);
void 	PrintCategory 		 (int, double *, char *, char *);
void 	FindCcmr 			 (void);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{
	int	count = 0;

	if (argc < 5)
	{
		print_at (0,0,mlSkMess369, argv [0]);
		return (EXIT_FAILURE);
	}

	printerNumber = atoi (argv [1]);
	sprintf (lower,"%-12.12s",argv [2]);
	sprintf (upper,"%-12.12s",argv [3]);
	yearEnd [0] = StringToDate (argv [4]);

	if (yearEnd [0] == -1L)
	{
		print_at (20,0,ML (mlSkMess530));
		return (EXIT_FAILURE);
	}

	/*======================
	| Open database files. |
	======================*/
	OpenDB ();

	init_scr ();

	dsp_screen ("Now Processing : Summary Stock Age Report (FIFO)", comm_rec.co_no, comm_rec.co_name);

	/*=================================
	| Open pipe work file to pformat. |
 	=================================*/
	if ((ftmp = popen ("pformat","w")) == 0)
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);

	HeadingOutput ();

	for (count = 0 ; count < loopCounter ; count++)
		ProcessData (hhccHash [count], branchNo [count], warehouseNo [count]);

    fprintf (ftmp,".EOF\n");

	/*========================= 
	| Program exit sequence	. |
	=========================*/
	pclose (ftmp);

	CloseDB (); 
	FinishProgram ();
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
	fprintf (ftmp, ".SO\n");
	fprintf (ftmp, ".11\n");
	fprintf (ftmp, ".L116\n");

	fprintf (ftmp, ".B1\n");
	fprintf (ftmp, ".ESUMMARY STOCK AGE REPORT\n");
	fprintf (ftmp, ".E%s \n",clip (comm_rec.co_name));
	fprintf (ftmp, ".B1\n");
	fprintf (ftmp, ".E AS AT : %s\n",SystemTime ());

	fprintf (ftmp, ".R======================");
	fprintf (ftmp, "==============");
	fprintf (ftmp, "==============");
	fprintf (ftmp, "==============");
	fprintf (ftmp, "==============");
	fprintf (ftmp, "==============");
	fprintf (ftmp, "===============\n");

	fprintf (ftmp, "======================");
	fprintf (ftmp, "==============");
	fprintf (ftmp, "==============");
	fprintf (ftmp, "==============");
	fprintf (ftmp, "==============");
	fprintf (ftmp, "==============");
	fprintf (ftmp, "===============\n");

	fprintf (ftmp, "|BR|WH|   T O T A L   ");
	fprintf (ftmp, "| Current Year");
	fprintf (ftmp, "| 1 year old  ");
	fprintf (ftmp, "| 2 years old ");
	fprintf (ftmp, "| 3 years old ");
	fprintf (ftmp, "| 4 years old ");
	fprintf (ftmp, "| 5+ years old|\n");

	fprintf (ftmp, "|--|--|---------------");
	fprintf (ftmp, "|-------------");
	fprintf (ftmp, "|-------------");
	fprintf (ftmp, "|-------------");
	fprintf (ftmp, "|-------------");
	fprintf (ftmp, "|-------------");
	fprintf (ftmp, "|-------------|\n");
	fprintf (ftmp, ".PI12\n");
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
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	open_rec (excf, excf_list, EXCF_NO_FIELDS, "excf_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no_2");
	FindCcmr ();
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
	abc_fclose (incc);
	CloseCosting ();
	abc_dbclose ("data");
}

void
ProcessData (
	long	hhccHash, 
	char	*branchNo, 
	char	*warehouseNo)
{
	int	i;
	char	oldGroup [13];
	char	newGroup [13];
	int		firstTime = TRUE;
	float	on_hand;
	double	calc;
	double	value [7];
	double	groupTotal [7];
	double	grandTotal [7];

	SetPeriod ();

	for (i = 0;i < 7;i++)
		grandTotal [i] = 0.00;

	/*-----------------------
	|	read first incc	    |
	-----------------------*/
	incc_rec.hhcc_hash = hhccHash;
	sprintf (incc_rec.sort,"%-28.28s",lower);
	cc = find_rec (incc,&incc_rec,GTEQ,"r");

	sprintf (oldGroup,"%12.12s",lower);
    
	/*----------------
	| loop thru incc |
	----------------*/
	while (!cc && incc_rec.hhcc_hash == hhccHash && 
				strncmp (incc_rec.sort,upper,12) <= 0)
	{
		strcpy (inmr_rec.co_no, comm_rec.co_no);
		sprintf (inmr_rec.item_no, "%-16.16s",incc_rec.sort + 12);
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		if (cc || (inmr_rec.costing_flag [0] != 'F' && inmr_rec.costing_flag [0] != 'I'))
		{
			cc = find_rec (incc,&incc_rec,NEXT,"r");

			continue;
		}
		dsp_process (" Item: ", inmr_rec.item_no);

		sprintf (newGroup,"%1.1s%-11.11s",inmr_rec.inmr_class,inmr_rec.category);

		if (firstTime)
		{
			firstTime = FALSE;
			PrintCategory (FALSE, groupTotal, branchNo, warehouseNo);
			strcpy (oldGroup, newGroup);
		}
		if (strcmp (newGroup,oldGroup))
		{
			strcpy (oldGroup, newGroup);
			PrintCategory (TRUE, groupTotal, branchNo, warehouseNo);
			
			for (i = 0;i < 7;i++)
				groupTotal [i] = 0.00;
		}

		for (i = 0;i < 7;i++)
			value [i] = 0.00;

		cc = FindIncf (incc_rec.hhwh_hash,FALSE,"r");
		on_hand = incc_rec.closing_stock;
		while (on_hand > 0.00 && !cc && incfRec.hhwh_hash == incc_rec.hhwh_hash)
		{
			if (on_hand < incfRec.fifo_qty)
			{
				incfRec.fifo_qty = on_hand;
				on_hand = 0.00;
			}
			else
				on_hand -= incfRec.fifo_qty;

			i = GetPeriodNo (incfRec.fifo_date);
			if (++i != 0)
			{
				calc = (double) incfRec.fifo_qty;
				calc *= twodec (incfRec.fifo_cost);
				calc = twodec (calc);
				value [i] += calc;
				value [0] += calc;
				groupTotal [i] += calc;
				groupTotal [0] += calc;
				grandTotal [i] += calc;
				grandTotal [0] += calc;
					
			}
			cc = FindIncf (0L,FALSE,"r");
		}
		cc = find_rec (incc,&incc_rec,NEXT,"r");
	}
	fprintf (ftmp, "|%-2.2s|%-2.2s",branchNo,warehouseNo);
	if (groupTotal [0] == 0.00)
		fprintf (ftmp, "| %13.13s "," ");
	else
		fprintf (ftmp, "| %13.2f ",groupTotal [0]);

	for (i = 1;i < 7;i++)
		if (groupTotal [i] == 0.00)
			fprintf (ftmp, "| %11.11s "," ");
		else
			fprintf (ftmp, "| %11.2f ",groupTotal [i]);
	fprintf (ftmp, "|\n");
	fflush (ftmp);
	fprintf (ftmp, "======================");
	fprintf (ftmp, "==============");
	fprintf (ftmp, "==============");
	fprintf (ftmp, "==============");
	fprintf (ftmp, "==============");
	fprintf (ftmp, "==============");
	fprintf (ftmp, "===============\n");

	fprintf (ftmp, "|%-2.2s|%-2.2s",branchNo,warehouseNo);
	if (grandTotal [0] == 0.00)
		fprintf (ftmp, "| %13.13s "," ");
	else
		fprintf (ftmp, "| %13.2f ",grandTotal [0]);
	for (i = 1;i < 7;i++)
		if (grandTotal [i] == 0.00)
			fprintf (ftmp, "| %11.11s "," ");
		else
			fprintf (ftmp, "| %11.2f ",grandTotal [i]);
	fprintf (ftmp, "|\n");
	fflush (ftmp);
}

/*=======================================
|	set cutoff dates for the eoy	|
=======================================*/
void
SetPeriod (void)
{
	int	i;

	for (i = 1;i < 6;i++)
		yearEnd [i] = AddYears (yearEnd [i - 1], -1);
}

/*=======================================
|	parameter: fifo_date				|
|	returns:							|
|		-1 newer than eoy				|
|		0  current						|
|		1  1 yr old						|
|		2  2 yr old						|
|		3  3 yr old						|
|		4  4+ yr old					|
|		5  5+ yr old					|
=======================================*/
int
GetPeriodNo (
 long l_date)
{
	int	i;

	if (l_date > yearEnd [0])
		return (-1);

	for (i = 0;i < 6;i++)
		if (l_date <= yearEnd [i] && l_date > yearEnd [i + 1])
			return (i);
	return (5);
}

void
PrintCategory (
 int prt_value, 
 double *groupTotal,
 char	*branchNo,
 char	*warehouseNo)
{
	int	i;

	strcpy (excf_rec.co_no, comm_rec.co_no);
	strcpy (excf_rec.cat_no,inmr_rec.category);
	cc = find_rec (excf, &excf_rec, COMPARISON, "r");
	if (cc)
	      strcpy (excf_rec.cat_desc, "No Category description found.");

	expand (err_str, excf_rec.cat_desc);
	
	if (prt_value)
	{
		fprintf (ftmp, "|%-2.2s|%-2.2s",branchNo,warehouseNo);
		if (groupTotal [0] == 0.00)
			fprintf (ftmp, "| %13.13s "," ");
		else
			fprintf (ftmp, "| %13.2f ",groupTotal [0]);

		for (i = 1;i < 7;i++)
			if (groupTotal [i] == 0.00)
				fprintf (ftmp, "| %11.11s "," ");
			else
				fprintf (ftmp, "| %11.2f ",groupTotal [i]);

		fprintf (ftmp, "|\n");
		fprintf (ftmp, ".LRP3\n");
		fprintf (ftmp, "|---------------------");
		fprintf (ftmp, "|-------------");
		fprintf (ftmp, "|-------------");
		fprintf (ftmp, "|-------------");
		fprintf (ftmp, "|-------------");
		fprintf (ftmp, "|-------------");
		fprintf (ftmp, "|-------------|\n");
	}
	fprintf (ftmp, "| %s%24.24s|\n",err_str, " ");
}

/*==========================================================================
| Read All branch master records for company one and two and store values. |
==========================================================================*/
void
FindCcmr (
 void)
{
	loopCounter = 0;
	
	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no,"  ");
	strcpy (ccmr_rec.cc_no, "  ");
	cc = find_rec (ccmr, &ccmr_rec, GTEQ,"r");
	while (!cc && !strcmp (ccmr_rec.co_no, comm_rec.co_no))
	{
		if (ccmr_rec.reports_ok [0] == 'N')
		{
			cc = find_rec (ccmr, &ccmr_rec, NEXT,"r");
			continue;
		}

		hhccHash [loopCounter] = ccmr_rec.hhcc_hash;
		strcpy (branchNo [loopCounter], ccmr_rec.est_no);
		strcpy (warehouseNo [loopCounter], ccmr_rec.cc_no);

		loopCounter++;

		cc = find_rec (ccmr, &ccmr_rec, NEXT,"r");
	}
}

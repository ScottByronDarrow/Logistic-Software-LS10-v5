/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_age_pc.c,v 5.3 2001/08/09 09:18:00 scott Exp $
|  Program Name  : (sk_age_pc.c)
|  Program Desc  : (Print ageing of stock)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 07/04/87         |
|---------------------------------------------------------------------|
| $Log: sk_age_pc.c,v $
| Revision 5.3  2001/08/09 09:18:00  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:44:37  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:18:47  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_age_pc.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_age_pc/sk_age_pc.c,v 5.3 2001/08/09 09:18:00 scott Exp $";

#include	<pslscr.h>
#include	<twodec.h>
#include	<ml_sk_mess.h>
#include	<Costing.h>

#define		CHECK_LEN	 (groupLength < 12)
#define		BY_MODE 	 (stockTakeMode [0] != ' ')
#define		MODE_OK		 (incc_rec.stat_flag [0] == stockTakeMode [0])

#define		VALID_FIFO	 (inmr_rec.costing_flag [0] == 'F' || \
			          	  inmr_rec.costing_flag [0] == 'I')

#include 	<dsp_screen.h>
#include 	<dsp_process.h>

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
void	Setperiod		 (void);
int		PeriodIn		 (long);
int		ColumnIn 		 (double);
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
		sprintf (err_str,"%s\n", mlSkMess018);
		print_at (0,0, err_str, argv [0]);
		return (EXIT_FAILURE);
	}
		
	printerNumber = atoi (argv [1]);
	sprintf (lower,"%-12.12s",argv [2]);
	sprintf (upper,"%-12.12s",argv [3]);
	sprintf (stockTakeMode,"%-1.1s",argv [4]);

	sptr = chk_env ("SK_VAL_NEGATIVE");
	stockNegativeValue = (sptr == (char *)0) ? FALSE : atoi (sptr);

	OpenDB ();

	if (strlen (clip (lower)) > strlen (clip (upper)))
		groupLength = strlen (clip (lower));
	else
		groupLength = strlen (clip (upper));

	init_scr ();

	dsp_screen ("Processing : Stock Valuation (Value Breakdown) (FIFO)", 
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
		sys_err ("Error in pformat During (POPEN)",errno,PNAME);

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
	fprintf (ftmp, ".%d\n",12 + i);
	fprintf (ftmp, ".L149\n");

	fprintf (ftmp, ".B1\n");
	fprintf (ftmp, ".ESTOCK VALUATION (VALUE BREAKDOWN) (FIFO)\n");
	if (BY_MODE)
		fprintf (ftmp, ".ESTOCK TAKE SELECTION [%s]\n",stockTakeMode);
	fprintf (ftmp, ".E%s \n",clip (comm_rec.co_name));
	fprintf (ftmp, ".EBranch: %s \n",clip (comm_rec.est_name));
	fprintf (ftmp, ".B1\n");
	fprintf (ftmp, ".EAS AT : %s\n",SystemTime ());

	fprintf (ftmp, ".R===================");
	fprintf (ftmp, "=======================================");
	fprintf (ftmp, "================================================");
	fprintf (ftmp, "=============\n");

	fprintf (ftmp, "===================");
	fprintf (ftmp, "=======================================");
	fprintf (ftmp, "================================================");
	fprintf (ftmp, "=============\n");

	fprintf (ftmp, "|   ITEM NUMBER    ");
	fprintf (ftmp, "|   I T E M    D E S C R I P T I O N   ");
	fprintf (ftmp, "|   0-200   |  201-500  |  501-1000 | 1001-5000 ");
	fprintf (ftmp, "| 5001-PLUS |\n");

	fprintf (ftmp, "|------------------");
	fprintf (ftmp, "|--------------------------------------");
	fprintf (ftmp, "|-----------|-----------|-----------|-----------");
	fprintf (ftmp, "|-----------|\n");
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

	ReadMisc ();
	open_rec (excf, excf_list, EXCF_NO_FIELDS, "excf_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no_2");
	open_rec (sttf, sttf_list, STTF_NO_FIELDS, "sttf_id_no");
	open_rec (insc, insc_list, INSC_NO_FIELDS, "insc_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (inei);
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
	int		i,
			firstTime = TRUE,
			fifoRecordFound;

	char	oldGroup [13],
			newGroup [13];

	float	onHand = 0.00,
			needed = 0.00;

	double	extend,
			fifoValue,
			totalItem [6],
			totalGrand [6],
			totalGroup [6],
			add_cost = 0.00;

	int		fifoError = FALSE;

	for (i = 0;i < 6;i++)
		totalGrand [i] = 0.00;

	for (i = 0;i < 6;i++)
		totalGroup [i] = 0.00;

	for (i = 0;i < 6;i++)
		totalItem [i] = 0.00;

	/*-----------------------
	|	read first incc	|
	-----------------------*/
	fflush (ftmp);
	incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	sprintf (incc_rec.sort,"%-12.12s%-16.16s",lower, " ");
	cc = find_rec (incc, &incc_rec,GTEQ,"r");
	sprintf (oldGroup,"%12.12s"," ");

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
		if (cc)
			sys_err ("Error in inmr During (DBFIND)", cc, PNAME);

		dsp_process (" Item: ", inmr_rec.item_no);

		sprintf (newGroup,"%1.1s%-11.11s", 	inmr_rec.inmr_class,
					           				inmr_rec.category);
		if (strncmp (newGroup, upper, groupLength) > 0)
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
					for (i = 0;i < 6;i++)
						totalGroup [i] = 0.00;
				}
				fifoRecordFound = FALSE;
			}

			fifoValue 	= (double)	n_dec 
									 (
										incfRec.fifo_qty, 
										inmr_rec.dec_pt
									);
			extend 			= twodec (incfRec.fifo_cost) * fifoValue;
			extend 			= out_cost (extend, inmr_rec.outer_size);
			totalItem [0]	+= extend;
			totalGroup [0] 	+= extend;
			totalGrand [0] 	+= extend;

			cc = FindIncf (0L,FALSE,"r");
		}

		if (onHand > 0.00)
		{
			fifoError = TRUE;
			cc = FindInei (ineiRec.hhbr_hash, ccmr_rec.est_no, "r");
			if (cc)
				file_err (cc, inei,"DBFIND");
		

			add_cost	= CheckIncf 
						  (
							incc_rec.hhwh_hash,
							 (inmr_rec.costing_flag [0] == 'F') ? TRUE : FALSE,
							needed,
							inmr_rec.dec_pt
						 );
					       
			extend = out_cost (add_cost / onHand, inmr_rec.outer_size);

			extend = twodec (extend);
			extend *= (double) onHand;
			extend = twodec (extend);
			totalItem [0] 	+= extend;
			totalGroup [0] 	+= extend;
			totalGrand [0] 	+= extend;
		}
		if (onHand < 0.00 && stockNegativeValue)
		{
			strcpy (ineiRec.est_no, ccmr_rec.est_no);
			ineiRec.hhbr_hash = inmr_rec.hhbr_hash;
			cc = find_rec (inei,&ineiRec,COMPARISON,"r");
			if (cc)
				file_err (cc, inei,"DBFIND");
		
			extend	=	out_cost (ineiRec.last_cost, inmr_rec.outer_size);
			extend	=	twodec (extend);
			extend	*=	(double) onHand;
			extend	=	twodec (extend);
			totalItem 	[0] += extend;
			totalGroup 	[0] += extend;
			totalGrand 	[0] += extend;
		}
		/*-------------------------------
		| fifo records found for item	|
		-------------------------------*/
		if (!fifoRecordFound && totalItem [0] != 0.00)
		{
			fprintf (ftmp, "| %-16.16s ",inmr_rec.item_no);
			fprintf (ftmp, "|%-38.38s",inmr_rec.description);
			i = ColumnIn (totalItem [0]);
			totalItem [i] 	+= totalItem [0];
			totalGroup [i] 	+= totalItem [0];
			totalGrand [i] 	+= totalItem [0];
			for (i = 1;i < 6;i++)
			{
				if (totalItem [i] == 0.00)
					fprintf (ftmp, "|%11.11s"," ");
				else
					fprintf (ftmp, "|%11.2f",totalItem [i]);
			}
			fprintf (ftmp, "|\n");
			for (i = 0;i < 6;i++)
				totalItem [i] = 0.00;
		}

		cc = find_rec (incc,&incc_rec,NEXT,"r");
	}
	fprintf (ftmp, "| %-16.16s "," ");
	fprintf (ftmp, "|%38.38s","*** GROUP TOTAL ***");

	for (i = 1;i < 6;i++)
	{
		if (totalGroup [i] == 0.00)
			fprintf (ftmp, "|%11.11s"," ");
		else
			fprintf (ftmp, "|%11.2f",totalGroup [i]);
	}
	fprintf (ftmp, "|\n");

	fprintf (ftmp, "| %-16.16s "," ");
	fprintf (ftmp, "|%38.38s","*** STOCK TOTAL ***");
	for (i = 1;i < 6;i++)
	{
		if (totalGrand [i] == 0.00)
			fprintf (ftmp, "|%11.11s"," ");
		else
			fprintf (ftmp, "|%11.2f",totalGrand [i]);
	}
	fprintf (ftmp, "|\n");
}

int
ColumnIn (
 double ffValue)
{
	double	fifoWorkCost	=	0.00;

	fifoWorkCost	=	ffValue;

	if (fifoWorkCost <= 200.00 && fifoWorkCost > 0.00)
		 return (EXIT_FAILURE);

	if (fifoWorkCost <= 500.00 && fifoWorkCost > 200.00)
		 return (2);

	if (fifoWorkCost <= 1000.00 && fifoWorkCost > 500.00)
		 return (3);

	if (fifoWorkCost <= 5000.00 && fifoWorkCost > 1000.00)
		 return (4);

	if (fifoWorkCost > 5000.0)
		 return (5);

    return (EXIT_SUCCESS);
}

void
PrintCategory (
	int 	printValue, 
	double *totalGroup)
{
	int		i;
	int		len = groupLength - 1;

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

	fprintf (ftmp, ".PD| %-116.116s|\n",err_str);

	if (printValue)
	{
		fprintf (ftmp, "| %-16.16s "," ");
		fprintf (ftmp, "|%38.38s","*** GROUP TOTAL ***");
		for (i = 1;i < 6;i++)
		{
			if (totalGroup [i] == 0.00)
				fprintf (ftmp, "|%11.11s"," ");
			else
				fprintf (ftmp, "|%11.2f",totalGroup [i]);
		}
		fprintf (ftmp, "|\n.PA\n");
	}
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

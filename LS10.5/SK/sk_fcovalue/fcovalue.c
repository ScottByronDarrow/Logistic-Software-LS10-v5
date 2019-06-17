/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Log: fcovalue.c,v $
| Revision 5.3  2001/08/09 09:18:27  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:44:53  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:19:04  scott
| Update - LS10.5
|
|  Program Name  : (sk_fcovalue.c)
|  Program Desc  : (Print Detailed Stock Valuation Report)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 03/02/88         |
|---------------------------------------------------------------------|
| $Id: fcovalue.c,v 5.3 2001/08/09 09:18:27 scott Exp $
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: fcovalue.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_fcovalue/fcovalue.c,v 5.3 2001/08/09 09:18:27 scott Exp $";

#include	<pslscr.h>
#include	<twodec.h>

#define	BY_MODE 	 (stockTakeMode [0] != ' ')
#define	MODE_OK		 (incc_rec.stat_flag [0] == stockTakeMode [0])

#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<ml_std_mess.h>
#include 	<ml_sk_mess.h>
#include 	<Costing.h>

#include	"schema"

	struct	commRecord	comm_rec;
	struct	ccmrRecord	ccmr_rec;
	struct	esmrRecord	esmr_rec;
	struct	excfRecord	excf_rec;
	struct	inccRecord	incc_rec;
	struct	inmrRecord	inmr_rec;
	struct	sttfRecord	sttf_rec;

	char 	*data	= "data";

	int		printerNumber = 1;
	int		stockNegativeValue	=	FALSE;

	FILE	*ftmp;

	char	oldGroup [13];
	char	newGroup [13];
	char	lower [13], 
			upper [13];

	char	stockTakeMode [2];

	int		firstTime = TRUE;
	double	totals [2];

	float	totalQuantities [2];

/*=======================
| Function Declarations |
=======================*/
void 	HeadingOutput 	(void);
void 	shutdown_prog 	(void);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	ProcessData  	(void);
void 	ProcessItem 	(long);
void 	PrintTotal 		(char *);
void 	PrintCategory 	(void);
int  	FindCcmr 		(long);
float 	GetClosingStock	(float, long);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{
	char	*sptr;
	register	int	i;

	/*---------------------------------------
	|	parameters							|
	|	1:	program name					|
	|	2:	printer number					|
	|	3:	lower bound   					|
	|	4:	upper bound   					|
	|	5:	Stock Take Mode   				|
	|	  	' ' - no stockTakeMode expected		    |
	---------------------------------------*/
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

	init_scr ();

	dsp_screen ("Now Processing : Company Full Stock Valuation Report", 
				comm_rec.co_no, comm_rec.co_name);

	IN_STAKE = (BY_MODE) ? TRUE : FALSE;
	
	/*=================================
	| Open pipe work file to pformat. |
 	=================================*/
	if ((ftmp = popen ("pformat","w")) == 0)
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);

	HeadingOutput ();

	for (i = 0; i < 2; i++)
	{
		totalQuantities [i] = 0.00;
		totals [i] = 0.00;
	}

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
	fprintf (ftmp, ".PI12\n");
	fprintf (ftmp, ".LP%d\n",printerNumber);
	fprintf (ftmp, ".SO\n");
	fprintf (ftmp, ".%d\n",10 + i);
	fprintf (ftmp, ".L110\n");
	fprintf (ftmp, ".B1\n");
	fprintf (ftmp, ".ECOMPANY DETAILED STOCK VALUATION REPORT\n");
	if (BY_MODE)
		fprintf (ftmp, ".ESTOCK TAKE SELECTION [%s]\n",stockTakeMode);
	fprintf (ftmp, ".E%s \n",clip (comm_rec.co_name));
	fprintf (ftmp, ".B1\n");
	fprintf (ftmp, ".EAS AT : %s\n",SystemTime ());

	fprintf (ftmp, ".R=================");
	fprintf (ftmp, "===========================================");
	fprintf (ftmp, "===============");
	fprintf (ftmp, "====================\n");

	fprintf (ftmp, "=================");
	fprintf (ftmp, "===========================================");
	fprintf (ftmp, "===============");
	fprintf (ftmp, "====================\n");


	fprintf (ftmp, "|  ITEM NUMBER   ");
	fprintf (ftmp, "|    I T E M      D E S C R I P T I O N    ");
	fprintf (ftmp, "|   QUANTITY   ");
	fprintf (ftmp, "|     EXTENDED     |\n");

	fprintf (ftmp, "|----------------");
	fprintf (ftmp, "|------------------------------------------");
	fprintf (ftmp, "|--------------");
	fprintf (ftmp, "|------------------|\n");

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

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (esmr, esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_hhcc_hash");
	open_rec (excf, excf_list, EXCF_NO_FIELDS, "excf_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no_3");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_hhbr_hash");
	open_rec (sttf, sttf_list, STTF_NO_FIELDS, "sttf_id_no");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (esmr);
	abc_fclose (ccmr);
	abc_fclose (excf);
	abc_fclose (inmr);
	abc_fclose (incc);
	abc_fclose (sttf);
	CloseCosting ();
	abc_dbclose ("data");
}

void
ProcessData  (
 void)
{
	/*-----------------------
	| read first inmr		|
	-----------------------*/
	strcpy (inmr_rec.co_no,comm_rec.co_no);
	sprintf (inmr_rec.inmr_class,"%-1.1s",lower);
	sprintf (inmr_rec.category,"%-11.11s",lower + 1);
	sprintf (inmr_rec.item_no,"%-16.16s"," ");
	cc = find_rec (inmr,&inmr_rec,GTEQ,"r");

	sprintf (oldGroup,"%1.1s%-11.11s", inmr_rec.inmr_class, inmr_rec.category);
	sprintf (newGroup,"%1.1s%-11.11s", inmr_rec.inmr_class, inmr_rec.category);
    
	/*----------------
	| loop thru inmr |
	----------------*/
	while (!cc && strcmp (newGroup,upper) <= 0 && 
		  		 !strcmp (inmr_rec.co_no,comm_rec.co_no))
	{
		if (strcmp (inmr_rec.supercession,"                "))
		{
			cc = find_rec (inmr,&inmr_rec,NEXT,"r");
			sprintf (newGroup,"%1.1s%-11.11s",inmr_rec.inmr_class,inmr_rec.category);
			continue;
		}

		dsp_process (" Item: ", inmr_rec.item_no);

		if (strcmp (newGroup,oldGroup) && totalQuantities [0] != 0.00)
			PrintTotal ("A");

		ProcessItem (inmr_rec.hhbr_hash);
		firstTime = FALSE;

		cc = find_rec (inmr,&inmr_rec,NEXT,"r");
		sprintf (newGroup,"%1.1s%-11.11s", inmr_rec.inmr_class, inmr_rec.category);
	}
	if (totalQuantities [0] != 0.00)
		PrintTotal ("A");

	PrintTotal ("C");
}

void
ProcessItem (
 long hhbr_hash)
{
	double	extend = 0.00,
			value = 0.00,
			itemTotal = 0.00;
	

	float	itemQuantity = 0.00,
			inccQuantity = 0.00;

	incc_rec.hhbr_hash = hhbr_hash;

	cc = find_rec (incc,&incc_rec,GTEQ,"r");
	while (!cc && incc_rec.hhbr_hash == hhbr_hash)
	{
		if (BY_MODE && !MODE_OK)
		{
			cc = find_rec (incc,&incc_rec,NEXT,"r");
			continue;
		}

		if (FindCcmr (incc_rec.hhcc_hash))
		{
			cc = find_rec (incc,&incc_rec,NEXT,"r");
			continue;
		}

		inccQuantity = (BY_MODE) ? GetClosingStock (incc_rec.closing_stock, 
			incc_rec.hhwh_hash) : incc_rec.closing_stock;

		if (inccQuantity <= 0.00)
		{
			if (stockNegativeValue)
				strcpy (inmr_rec.costing_flag, "L");
			else
			{
				cc = find_rec (incc,&incc_rec,NEXT,"r");
				continue;
			}
		}
		value	=	StockValue
				 	(
						inmr_rec.costing_flag,
						comm_rec.est_no,
						inmr_rec.hhbr_hash,
						incc_rec.hhwh_hash,
						inccQuantity,
						inmr_rec.dec_pt,
						TRUE
					);
		value	=	twodec (value);
		extend	=	value;
		extend	*=	 (double) n_dec (inccQuantity, inmr_rec.dec_pt);
		extend	=	out_cost (extend, inmr_rec.outer_size);

		itemQuantity 		+= inccQuantity;
		totalQuantities [0] += inccQuantity;
		totalQuantities [1] += inccQuantity;

		itemTotal 	+= extend;
		totals [0] 	+= extend;
		totals [1] 	+= extend;

		cc = find_rec (incc,&incc_rec,NEXT,"r");
	}
	if (itemQuantity != 0.00)
	{
		if (strcmp (newGroup,oldGroup) || firstTime)
		{
			PrintCategory ();
			strcpy (oldGroup,newGroup);
		}
		fprintf (ftmp, "|%-16.16s",inmr_rec.item_no);
		fprintf (ftmp, "| %-40.40s ",inmr_rec.description);
		fprintf (ftmp, "| %12.2f ",itemQuantity);
		fprintf (ftmp, "| %16.2f |%s\n",itemTotal,
						 (fifoError) ? "**" : " ");
	}
	return;
}

int
FindCcmr (
	long	hhccHash)
{
	/*---------------------------------------------
	| Find stock take record for date validation. |
	---------------------------------------------*/
	FindInsc (hhccHash, stockTakeMode, BY_MODE);

	if (ccmr_rec.hhcc_hash == hhccHash)
		return (EXIT_SUCCESS);

	ccmr_rec.hhcc_hash	=	hhccHash;
	return (find_rec (ccmr,&ccmr_rec,COMPARISON,"r"));
}

void
PrintTotal (
 char *tot_type)
{
	int	i;

	if (tot_type [0] == 'A')
	{
		fprintf (ftmp, "|----------------");
		fprintf (ftmp, "|------------------------------------------");
		fprintf (ftmp, "|--------------");
		fprintf (ftmp, "|------------------|\n");
	}

	fprintf (ftmp, "|                ");
	if (tot_type [0] == 'A')
	{
		fprintf (ftmp, "|    CATEGORY TOTAL.                       ");
		i = 0;
	}
	else
	{
		fprintf (ftmp, "|    COMPANY  TOTAL.                       ");
		i = 1;
	}
	fprintf (ftmp, "| %12.2f ",totalQuantities [i]);
	fprintf (ftmp, "| %16.2f |\n",totals [i]);

	totalQuantities [i] = 0.00;
	totals [i] = 0.00;
	fprintf (ftmp, "|----------------");
	fprintf (ftmp, "|------------------------------------------");
	fprintf (ftmp, "|--------------");
	fprintf (ftmp, "|------------------|\n");
}

void
PrintCategory (
 void)
{
	strcpy (excf_rec.co_no, comm_rec.co_no);
	strcpy (excf_rec.cat_no,inmr_rec.category);
	cc = find_rec (excf, &excf_rec, COMPARISON, "r");
	if (cc)
	      strcpy (excf_rec.cat_desc, "No Category description found.");

	fprintf (ftmp, "|  %-11.11s   | %-40.40s |              |                  |\n",
			excf_rec.cat_no,
			excf_rec.cat_desc);
}

/*===========================
| Get closing stock actual. |
===========================*/
float 
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
		counted += sttf_rec.qty;
		cc = find_rec (sttf, &sttf_rec, NEXT, "r");
	}

	return (on_hand - (on_hand - counted));
}


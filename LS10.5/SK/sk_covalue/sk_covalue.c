/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_covalue.c,v 5.4 2001/09/11 23:17:03 scott Exp $
|  Program Name  : (sk_covalue.c  )                                   |
|  Program Desc  : (Print Summary Stock Valuation Report        )     |
|                  (by Company.                                 )     |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 03/02/88         |
|---------------------------------------------------------------------|
| $Log: sk_covalue.c,v $
| Revision 5.4  2001/09/11 23:17:03  scott
| Updated from Scott machine - 12th Sep 2001
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_covalue.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_covalue/sk_covalue.c,v 5.4 2001/09/11 23:17:03 scott Exp $";

#include	<pslscr.h>		
#include	<twodec.h>
#define		BY_MODE 	 (stockTakeMode [0] != ' ')
#define		MODE_OK		 (incc_rec.stat_flag [0] == stockTakeMode [0])

#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<ml_std_mess.h>
#include 	<ml_sk_mess.h>
#include 	<Costing.h>

#define		CF(x)	comma_fmt (x, "NNNN,NNN,NNN,NNN.NN")

#include	"schema"

	struct	commRecord	comm_rec;
	struct	ccmrRecord	ccmr_rec;
	struct	esmrRecord	esmr_rec;
	struct	excfRecord	excf_rec;
	struct	inccRecord	incc_rec;
	struct	inmrRecord	inmr_rec;
	struct	sttfRecord	sttf_rec;

	char 	*data	= "data";

	int		maxBranch,
			printerNumber = 1;

	int		stockNegativeValue	=	FALSE;

	FILE	*ftmp;

	char	lower [13], 
			upper [13];

struct WhRecord {
	long	hhccHash;
	char	branchNumber [3];
	long	warehouseDate;
	struct	WhRecord	*lptr;
	struct	WhRecord	*rptr;
};

struct	WhRecord	*whHead;
struct	WhRecord	*whTemp;

struct	{
	char	branchNumber [3];
	double	total;
	double	grandTotal;
} branchTotal [100];


	char	stockTakeMode [2];

/*=======================
| Function Declarations |
=======================*/
void	HeadingOutput 		 (void);
void	shutdown_prog 		 (void);
void	OpenDB 				 (void);
void	CloseDB 			 (void);
void	ReadMisc 			 (void);
void	WhInsert 			 (struct WhRecord *);
void	ProcessData 		 (void);
void	WhProcess 			 (struct WhRecord *, long);
void	SumBranch 			 (long, long, char *);
double	WhCost 				 (long, long, char *);
void	PrintTotal 			 (char *group);
struct	WhRecord *WhAlloc 	 (void);
float	GetClosingStock  	 (float, long);

    
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

	/*===================================================
	| Read common terminal record & cost centre master. |
	===================================================*/
	ReadMisc ();

	init_scr ();

	dsp_screen ("Now Processing : Company Summary Stock Valuation Report", 
					comm_rec.co_no, comm_rec.co_name);

	IN_STAKE = (BY_MODE) ? TRUE : FALSE;

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
	fprintf (ftmp, ".PI12\n");
	fprintf (ftmp, ".LP%d\n",printerNumber);
	fprintf (ftmp, ".SO\n");
	fprintf (ftmp, ".%d\n",9 + i);
	fprintf (ftmp, ".L130\n");
	fprintf (ftmp, ".B1\n");
	fprintf (ftmp, ".ECOMPANY SUMMARY STOCK VALUATION REPORT\n");
	if (BY_MODE)
		fprintf (ftmp, ".ESTOCK TAKE SELECTION [%s]\n",stockTakeMode);
	fprintf (ftmp, ".E%s \n",clip (comm_rec.co_name));
	fprintf (ftmp, ".B1\n");
	fprintf (ftmp, ".E AS AT : %s\n",SystemTime ());

	fprintf (ftmp, ".R===============");
	fprintf (ftmp, "===========================================");
	fprintf (ftmp, "=====");
	fprintf (ftmp, "===========================================");
	fprintf (ftmp, "======================\n");

	fprintf (ftmp, "===============");
	fprintf (ftmp, "===========================================");
	fprintf (ftmp, "=====");
	fprintf (ftmp, "===========================================");
	fprintf (ftmp, "======================\n");

	fprintf (ftmp, "|    GROUP     ");
	fprintf (ftmp, "|    G R O U P    D E S C R I P T I O N    ");
	fprintf (ftmp, "| BR ");
	fprintf (ftmp, "|    B R A N C H   D E S C R I P T I O N   ");
	fprintf (ftmp, "|      EXTENDED      |\n");

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
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	open_rec (excf, excf_list, EXCF_NO_FIELDS, "excf_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no_3");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no");
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

/*===================================== 
| Get info from commom database file .|
=====================================*/
void
ReadMisc (
 void)
{
	char	oldBranch [3];

	whHead = (struct WhRecord *) 0;
	maxBranch = 0;

	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	strcpy (ccmr_rec.co_no,comm_rec.co_no);
	strcpy (ccmr_rec.est_no,"  ");
	strcpy (ccmr_rec.cc_no,"  ");
	cc = find_rec (ccmr,&ccmr_rec,GTEQ,"r");
	if (cc)
		sys_err ("Error in ccmr During (DBFIND)", cc, PNAME);

	strcpy (oldBranch,ccmr_rec.est_no);
	strcpy (branchTotal [maxBranch].branchNumber,ccmr_rec.est_no);
	branchTotal [maxBranch].total = 0.00;
	branchTotal [maxBranch++].grandTotal = 0.00;

	while (!cc && !strcmp (ccmr_rec.co_no,comm_rec.co_no))
	{
		if (strcmp (oldBranch,ccmr_rec.est_no))
		{
			strcpy (branchTotal [maxBranch].branchNumber,ccmr_rec.est_no);
			branchTotal [maxBranch].total = 0.00;
			branchTotal [maxBranch++].grandTotal = 0.00;
			strcpy (oldBranch,ccmr_rec.est_no);
		}

		whTemp = WhAlloc ();
		whTemp->hhccHash = ccmr_rec.hhcc_hash;

		FindInsc (ccmr_rec.hhcc_hash, stockTakeMode, BY_MODE);

		whTemp->warehouseDate = STAKE_COFF;

		sprintf (whTemp->branchNumber,"%2.2s",ccmr_rec.est_no);

		WhInsert (whHead);

		cc = find_rec (ccmr,&ccmr_rec,NEXT,"r");
	}

	abc_fclose (ccmr);
}

void		
WhInsert (
 struct WhRecord *whTptr)
{
	/*-----------------------
	| Put at start of tree	|
	-----------------------*/
	if (whHead == (struct WhRecord *) 0)
	{
		whHead = whTemp;
		whTemp->lptr = (struct WhRecord *) 0;
		whTemp->rptr = (struct WhRecord *) 0;
	}
	else
	{
		/*-----------------------
		| Check Left Sub Tree	|
		-----------------------*/
		if (whTemp->hhccHash < whTptr->hhccHash)
		{
			if (whTptr->lptr != (struct WhRecord *)0)
				WhInsert (whTptr->lptr);
			else
			{
				whTptr->lptr = whTemp;
				whTemp->lptr = (struct WhRecord *) 0;
				whTemp->rptr = (struct WhRecord *) 0;
			}
		}
		else
		{
			if (whTptr->rptr != (struct WhRecord *)0)
				WhInsert (whTptr->rptr);
			else
			{
				whTptr->rptr = whTemp;
				whTemp->lptr = (struct WhRecord *) 0;
				whTemp->rptr = (struct WhRecord *) 0;
			}
		}
	}
}

/*
cc_print (whTptr)
struct	WhRecord	*whTptr;
{
	if (whTptr != (struct WhRecord *)0)
	{
		cc_print (whTptr->lptr);
		print_at (0,0,"hhccHash = %03ld branchNumber = [%2s]\n",whTptr->hhccHash,whTptr->branchNumber);
		cc_print (whTptr->rptr);
	}
}
*/

void
ProcessData (
 void)
{
	char	oldGroup [13];
	char	newGroup [13];

	/*-------------------
	| read first inmr	|
	-------------------*/
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
			cc = find_rec (inmr, &inmr_rec, NEXT, "r");
			sprintf (newGroup,"%1.1s%-11.11s",
				inmr_rec.inmr_class,
				inmr_rec.category);

			continue;
		}

		dsp_process (" Item: ", inmr_rec.item_no);

		if (strcmp (newGroup,oldGroup))
		{
			PrintTotal (oldGroup);
			strcpy (oldGroup,newGroup);
		}

		WhProcess (whHead,inmr_rec.hhbr_hash);

		cc = find_rec (inmr,&inmr_rec,NEXT,"r");
		sprintf (newGroup,"%1.1s%-11.11s", inmr_rec.inmr_class, inmr_rec.category);
	}
	PrintTotal (oldGroup);
	PrintTotal ((char *)0);
}

void
WhProcess (
 struct WhRecord *whTptr, 
 long hhbrHash)
{
	if (whTptr != (struct WhRecord *)0)
	{
		/*---------------------------------------------
		| Find stock take record for date validation. |
		---------------------------------------------*/
		STAKE_COFF = whTptr->warehouseDate;

		WhProcess (whTptr->lptr,hhbrHash);
		SumBranch (whTptr->hhccHash,hhbrHash,whTptr->branchNumber);
		WhProcess (whTptr->rptr,hhbrHash);
	}
}

void
SumBranch (
 long hhccHash, 
 long hhbrHash, 
 char *branchNumber)
{
	int	i;
	double	cost;

	for (i = 0;i < maxBranch;i++)
	{
		if (!strcmp (branchNumber,branchTotal [i].branchNumber))
		{
			cost = WhCost (hhccHash,hhbrHash, branchTotal [i].branchNumber);
			branchTotal [i].total 		+= cost;
			branchTotal [i].grandTotal 	+= cost;
			break;
		}
	}
}

double	
WhCost (
	long	hhccHash, 
	long	hhbrHash, 
	char	*branchNumber)
{
	double	extend;
	double	value;
	float	inccQuantity;

	incc_rec.hhbr_hash = hhbrHash;
	incc_rec.hhcc_hash = hhccHash;

	cc = find_rec (incc,&incc_rec,COMPARISON,"r");

	if (cc)
		return (0.00);

	if (BY_MODE && !MODE_OK)
		return (0.00);

	inccQuantity = (BY_MODE) ? GetClosingStock (incc_rec.closing_stock, 
											incc_rec.hhwh_hash) 
						: incc_rec.closing_stock;
	
	if (inccQuantity <= 0.00)
	{
		if (stockNegativeValue)
			strcpy (inmr_rec.costing_flag, "L");
		else
			return (0.00);
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
	return (extend);
}

void
PrintTotal (
 char *group)
{
	int	i;
	double	grandTotals = 0.00;
	double	branchTotals = 0.00;

	if (group != (char *)0)
	{
		strcpy (excf_rec.co_no, comm_rec.co_no);
		sprintf (excf_rec.cat_no,"%-11.11s",group + 1);
		cc = find_rec (excf, &excf_rec, COMPARISON, "r");
		if (cc)
		      	sprintf (excf_rec.cat_desc, "%40.40s",
					"No Category description found.");
	}

	fprintf (ftmp, "|--------------");
	fprintf (ftmp, "|------------------------------------------");
	fprintf (ftmp, "|----");
	fprintf (ftmp, "|------------------------------------------");
	fprintf (ftmp, "|--------------------|\n");

	for (i = 0;i < maxBranch;i++)
	{
		strcpy (esmr_rec.co_no,comm_rec.co_no);
		strcpy (esmr_rec.est_no,branchTotal [i].branchNumber);
		cc = find_rec (esmr,&esmr_rec,COMPARISON,"r");
		if (!cc)
		{
			if (group == (char *)0)
			{
				fprintf (ftmp, "| %-12.12s ", (i == 0) ? "BRANCH TOTAL" : " ");
				fprintf (ftmp, "| %-40.40s "," ");
				fprintf (ftmp, "| %-2.2s ",esmr_rec.est_no);
				fprintf (ftmp, "| %-40.40s ",esmr_rec.est_name);
				fprintf (ftmp, "|%19.19s |\n",CF (branchTotal [i].grandTotal));
				grandTotals += branchTotal [i].grandTotal;
			}
			else
			{
				fprintf (ftmp, "| %-12.12s ", (i == 0) ? group : " ");
				fprintf (ftmp, "| %-40.40s ", (i == 0) ? excf_rec.cat_desc : " ");
				fprintf (ftmp, "| %-2.2s ",esmr_rec.est_no);
				fprintf (ftmp, "| %-40.40s ",esmr_rec.est_name);
				fprintf (ftmp, "|%19.19s |\n",CF (branchTotal [i].total));
				branchTotals += branchTotal [i].total;
			}

			branchTotal [i].total = 0.00;
		}
	}

	if (group == (char *)0)
	{
	    if (i > 1)
	    {
		fprintf (ftmp, "|==============");
		fprintf (ftmp, "|==========================================");
		fprintf (ftmp, "|====");
		fprintf (ftmp, "|==========================================");
		fprintf (ftmp, "|====================|\n");
	
		fprintf (ftmp, "| GRAND TOTAL. ");
		fprintf (ftmp, "| %-40.40s ", " ");
		fprintf (ftmp, "|    ");
		fprintf (ftmp, "| %-40.40s "," ");
		fprintf (ftmp, "|%19.19s |\n",CF (grandTotals));
	    }
	    return;
	}
	if (i > 1)
	{
		fprintf (ftmp, "| GROUP TOTAL. ");
		fprintf (ftmp, "| %-40.40s ",excf_rec.cat_desc);
		fprintf (ftmp, "|    ");
		fprintf (ftmp, "| %-40.40s "," ");
		fprintf (ftmp, "|%19.19s |\n", CF (branchTotals));
	}

	fprintf (ftmp, ".LRP%d\n",maxBranch + 1);
	fflush (ftmp);
}

struct	WhRecord *
WhAlloc (
 void)
{
	return ((struct WhRecord *) malloc (sizeof (struct WhRecord)));
}

/*===========================
| Get closing stock actual. |
===========================*/
float 
GetClosingStock  
(
 	float	onHand, 
 	long	hhwhHash)
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


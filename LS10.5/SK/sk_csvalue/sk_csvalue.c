/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_csvalue.c,v 5.2 2001/08/09 09:18:20 scott Exp $
|  Program Name  : (sk_csvalue.c & sk_bsvalue.c)
|  Program Desc  : (Print Summary Stock Valuation Report)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 03/02/88         |
|---------------------------------------------------------------------|
| $Log: sk_csvalue.c,v $
| Revision 5.2  2001/08/09 09:18:20  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/07/25 02:19:00  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_csvalue.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_csvalue/sk_csvalue.c,v 5.2 2001/08/09 09:18:20 scott Exp $";

#include	<pslscr.h>
#include	<twodec.h>
#include	<ml_sk_mess.h>
#include	<Costing.h>

#define		BY_MODE 	 (stockTakeMode [0] != ' ')
#define		MODE_OK		 (incc_rec.stat_flag [0] == stockTakeMode [0])
#define		BY_BR	  	 (rep_by [0] == 'B')

#include 	<dsp_screen.h>
#include 	<dsp_process2.h>

#include	"schema"

	struct	commRecord	comm_rec;
	struct	ccmrRecord	ccmr_rec;
	struct	excfRecord	excf_rec;
	struct	inccRecord	incc_rec;
	struct	inmrRecord	inmr_rec;
	struct	sttfRecord	sttf_rec;

	char 	*data	= "data";

	int		maxWarehouse,
			printerNumber = 1;

	int		stockNegativeValue	=	FALSE;

	FILE	*ftmp;

	char	lower [13], 
			upper [13];
	
struct whRecord {
	long	hhccHash;
	long	hhccDate;
	int		hhccMode;
	char	whNo [3];
	struct	whRecord	*lptr;
	struct	whRecord	*rptr;
};

struct	whRecord	*whHead;
struct	whRecord	*whTemp;

struct	{
	char	whNo [3];
	double	total;
	double	grandTotal;
} whTotal [100];

	char	stockTakeMode [2];
	char	rep_by [2];

/*=======================
| Function Declarations |
=======================*/
void	HeadingOutput		(void);
void	shutdown_prog 		(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
void	ReadMisc			(void);
void 	WarehouseInsert 	(struct whRecord *);
void 	WarehousePrint 		(struct whRecord *);
void 	ProcessData 		(void);
void 	WarehouseProcess 	(struct whRecord *, long);
void 	SumBranch 			(long, long, char *, long, int);
double	WarehouseCost 		(long, long, long, int);
void 	PrintTotal 			(char *);
void 	printGrand 			(void);
struct 	whRecord *wh_alloc 	(void);
double 	GetClosingStock 	(float, long);


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
	if (argc < 5)
	{
		sprintf (err_str,"%s\n", mlSkMess018);
		print_at (0,0, err_str, argv [0]);
		return (EXIT_FAILURE);
	}
	
	strcpy (rep_by, (!strncmp (argv [0],"sk_csvalue",10)) ? "C" : "B");

	printerNumber = atoi (argv [1]);
	sprintf (lower,"%-12.12s",argv [2]);
	sprintf (upper,"%-12.12s",argv [3]);
	if (argc == 5)
		sprintf (stockTakeMode,"%-1.1s",argv [4]);
	else
		strcpy (stockTakeMode," ");

	sptr = chk_env ("SK_VAL_NEGATIVE");
	stockNegativeValue = (sptr == (char *)0) ? FALSE : atoi (sptr);

	/*======================
	| Open database files. |
	======================*/
	OpenDB ();


	init_scr ();

	if (BY_BR)
		dsp_screen ("Now Processing : Branch Summary Stock Valuation Report", comm_rec.co_no, comm_rec.co_name);
	else
		dsp_screen ("Now Processing : Warehouse Summary Stock Valuation Report", comm_rec.co_no, comm_rec.co_name);

	/*=================================
	| Open pipe work file to pformat. |
 	=================================*/
	if ((ftmp = popen ("pformat","w")) == 0)
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
	int	i = 9;

	if (BY_MODE)
		i++;

	if (!BY_BR)
		i++;

	fprintf (ftmp, ".START%s<%s>\n", DateToString (comm_rec.inv_date), PNAME);
	fprintf (ftmp, ".PI12\n");
	fprintf (ftmp, ".LP%d\n",printerNumber);
	fprintf (ftmp, ".SO\n");
	fprintf (ftmp, ".%d\n", i);
	fprintf (ftmp, ".L127\n");
	fprintf (ftmp, ".B1\n");
	if (BY_BR)
		fprintf (ftmp, ".EBRANCH SUMMARY STOCK VALUATION REPORT\n");
	else
		fprintf (ftmp, ".EWAREHOUSE SUMMARY STOCK VALUATION REPORT\n");
	if (BY_MODE)
		fprintf (ftmp, ".ESTOCK TAKE SELECTION [%s]\n",stockTakeMode);
	fprintf (ftmp, ".E%s \n",clip (comm_rec.co_name));
	fprintf (ftmp, ".B1\n");
	fprintf (ftmp, ".E AS AT : %s\n",SystemTime ());

	fprintf (ftmp, ".R===============");
	fprintf (ftmp, "===========================================");
	fprintf (ftmp, "=====");
	fprintf (ftmp, "===========================================");
	fprintf (ftmp, "====================\n");

	fprintf (ftmp, "===============");
	fprintf (ftmp, "===========================================");
	fprintf (ftmp, "=====");
	fprintf (ftmp, "===========================================");
	fprintf (ftmp, "====================\n");

	fprintf (ftmp, "|    GROUP     ");
	fprintf (ftmp, "|    G R O U P    D E S C R I P T I O N    ");
	fprintf (ftmp, "| WH ");
	fprintf (ftmp, "| W A R E H O U S E  D E S C R I P T I O N ");
	fprintf (ftmp, "|     EXTENDED     |\n");

	if (!BY_BR)
	{
		fprintf (ftmp, "|--------------");
		fprintf (ftmp, "|------------------------------------------");
		fprintf (ftmp, "|----");
		fprintf (ftmp, "|------------------------------------------");
		fprintf (ftmp, "|------------------|\n");
	}
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

	/*===================================================
	| Read common terminal record & cost centre master. |
	===================================================*/
	ReadMisc ();
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
	char	oldWh [3];

	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");

	whHead = (struct whRecord *) 0;
	maxWarehouse = 0;

	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no,comm_rec.est_no);
	if (!BY_BR)
		strcpy (ccmr_rec.cc_no,comm_rec.cc_no);
	else
		strcpy (ccmr_rec.cc_no,"  ");

	cc = find_rec (ccmr,&ccmr_rec,GTEQ,"r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");

	strcpy (oldWh,ccmr_rec.cc_no);
	strcpy (whTotal [maxWarehouse].whNo, ccmr_rec.cc_no);
	whTotal [maxWarehouse].total = 0.00;
	whTotal [maxWarehouse++].grandTotal = 0.00;

	while (!cc && !strcmp (ccmr_rec.co_no,comm_rec.co_no) && 
		      !strcmp (ccmr_rec.est_no,comm_rec.est_no)) 
	{
		if (!BY_BR && strcmp (ccmr_rec.cc_no,comm_rec.cc_no))
		{
			cc = find_rec (ccmr,&ccmr_rec,NEXT,"r");
			continue;
		}
		if (strcmp (oldWh,ccmr_rec.cc_no))
		{
			strcpy (whTotal [maxWarehouse].whNo,ccmr_rec.cc_no);
			whTotal [maxWarehouse].total = 0.00;
			whTotal [maxWarehouse++].grandTotal = 0.00;
			strcpy (oldWh,ccmr_rec.cc_no);
		}

		/*---------------------------------------------
		| Find stock take record for date validation. |
		---------------------------------------------*/
		FindInsc (ccmr_rec.hhcc_hash, stockTakeMode, BY_MODE);

		whTemp = wh_alloc ();
		whTemp->hhccHash = ccmr_rec.hhcc_hash;
		whTemp->hhccDate = STAKE_COFF;
		whTemp->hhccMode = IN_STAKE;
		sprintf (whTemp->whNo,"%2.2s",ccmr_rec.cc_no);

		WarehouseInsert (whHead);

		cc = find_rec (ccmr,&ccmr_rec,NEXT,"r");
	}
}

void		
WarehouseInsert (
 struct whRecord *wh_tptr)
{
	/*-----------------------
	| Put at start of tree	|
	-----------------------*/
	if (whHead == (struct whRecord *) 0)
	{
		whHead = whTemp;
		whTemp->lptr = (struct whRecord *) 0;
		whTemp->rptr = (struct whRecord *) 0;
	}
	else
	{
		/*-----------------------
		| Check Left Sub Tree	|
		-----------------------*/
		if (whTemp->hhccHash < wh_tptr->hhccHash)
		{
			if (wh_tptr->lptr != (struct whRecord *)0)
				WarehouseInsert (wh_tptr->lptr);
			else
			{
				wh_tptr->lptr = whTemp;
				whTemp->lptr = (struct whRecord *) 0;
				whTemp->rptr = (struct whRecord *) 0;
			}
		}
		else
		{
			if (wh_tptr->rptr != (struct whRecord *)0)
				WarehouseInsert (wh_tptr->rptr);
			else
			{
				wh_tptr->rptr = whTemp;
				whTemp->lptr = (struct whRecord *) 0;
				whTemp->rptr = (struct whRecord *) 0;
			}
		}
	}
}

void
WarehousePrint (
 struct whRecord *wh_tptr)
{
	if (wh_tptr != (struct whRecord *)0)
	{
		WarehousePrint (wh_tptr->lptr);
		WarehousePrint (wh_tptr->rptr);
	}
}

void
ProcessData (
 void)
{
	char	oldGroup [13];
	char	newGroup [13];

	/*-----------------------
	| read first inmr	|
	-----------------------*/
	strcpy (inmr_rec.co_no,comm_rec.co_no);
	sprintf (inmr_rec.inmr_class,	"%-1.1s",lower);
	sprintf (inmr_rec.category,		"%-11.11s",lower + 1);
	sprintf (inmr_rec.item_no,		"%-16.16s"," ");
	cc = find_rec (inmr,&inmr_rec,GTEQ,"r");

	sprintf (oldGroup,"%1.1s%-11.11s", inmr_rec.inmr_class, inmr_rec.category);
	sprintf (newGroup,"%1.1s%-11.11s", inmr_rec.inmr_class, inmr_rec.category);
    
	/*----------------
	| loop thru inmr |
	----------------*/
	while (	!cc && strcmp (newGroup,upper) <= 0 && 
			!strcmp (inmr_rec.co_no,comm_rec.co_no))
	{
		if (strcmp (inmr_rec.supercession,"                "))
		{
			cc = find_rec (inmr,&inmr_rec,NEXT,"r");
			sprintf (newGroup,"%1.1s%-11.11s", inmr_rec.inmr_class, inmr_rec.category);
			continue;
		}

		dsp_process (" Item: ", inmr_rec.item_no);

		if (strcmp (newGroup,oldGroup))
		{
			PrintTotal (oldGroup);
			strcpy (oldGroup,newGroup);
		}

		WarehouseProcess (whHead,inmr_rec.hhbr_hash);

		cc = find_rec (inmr,&inmr_rec,NEXT,"r");
		sprintf (newGroup,"%1.1s%-11.11s", inmr_rec.inmr_class, inmr_rec.category);
	}
	PrintTotal (oldGroup);
	PrintTotal ((char *)0);
	if (BY_BR)
		printGrand ();
}

void
WarehouseProcess (
 struct whRecord *wh_tptr, 
 long hhbrHash)
{
	if (wh_tptr != (struct whRecord *)0)
	{
		WarehouseProcess 
		(
			wh_tptr->lptr, 	
			hhbrHash
		);
		SumBranch 
		(
			wh_tptr->hhccHash, 
			hhbrHash, 
			wh_tptr->whNo,
			wh_tptr->hhccDate,
			wh_tptr->hhccMode
		);
		WarehouseProcess 
		(
			wh_tptr->rptr,	
			hhbrHash
		);
	}
}

void
SumBranch (
	long	hhccHash, 
	long	hhbrHash, 
	char	*whNo, 
	long	hhccDate, 
	int		hhccMode)
{
	int	i;
	double	cost;

	for (i = 0;i < maxWarehouse;i++)
	{
		if (!strcmp (whNo,whTotal [i].whNo))
		{
			cost =	WarehouseCost 
					(
						hhccHash, 
						hhbrHash, 
				        hhccDate, 
						hhccMode
					);
			whTotal [i].total 		+= cost;
			whTotal [i].grandTotal 	+= cost;
			break;
		}
	}
}

double	
WarehouseCost (
	long	hhccHash, 
	long	hhbrHash, 
	long	hhccDate, 
	int		hhccMode)
{
	double	extend;
	double	value;
	float	closingStock = 0.00;

	incc_rec.hhbr_hash = hhbrHash;
	incc_rec.hhcc_hash = hhccHash;

	cc = find_rec (incc,&incc_rec,COMPARISON,"r");
	if (cc)
		return (0.00);

	if (BY_MODE && !MODE_OK)
		return (0.00);

	closingStock = (BY_MODE) ?
			GetClosingStock (
				n_dec (incc_rec.closing_stock, inmr_rec.dec_pt),
				incc_rec.hhwh_hash) : n_dec (incc_rec.closing_stock,
				inmr_rec.dec_pt);

	closingStock = n_dec (closingStock, inmr_rec.dec_pt);

	STAKE_COFF = hhccDate;
	IN_STAKE   = hhccMode;

	if (closingStock <= 0.00)
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
					closingStock,
					inmr_rec.dec_pt,
					TRUE
				);
	value	=	twodec (value);
	extend	=	value;
	extend	*=	 (double) n_dec (closingStock, inmr_rec.dec_pt);
	extend	=	out_cost (extend, inmr_rec.outer_size);
	return (extend);
}

void
PrintTotal (
 char *group)
{
	int	i;
	double	tot_tot = 0.00,
		tot_whs = 0.00;

	if (group != (char *)0)
	{
		strcpy (excf_rec.co_no, comm_rec.co_no);
		sprintf (excf_rec.cat_no,"%-11.11s",group + 1);
		cc = find_rec (excf, &excf_rec, COMPARISON, "r");
		if (cc)
		      	sprintf (excf_rec.cat_desc, "%40.40s",
					"No Category description found.");
	}

	if (BY_BR)
	{
		fprintf (ftmp, "|--------------");
		fprintf (ftmp, "|------------------------------------------");
		fprintf (ftmp, "|----");
		fprintf (ftmp, "|------------------------------------------");
		fprintf (ftmp, "|------------------|\n");
	}

	for (i = 0;i < maxWarehouse;i++)
	{
		strcpy (ccmr_rec.co_no,comm_rec.co_no);
		strcpy (ccmr_rec.est_no,comm_rec.est_no);
		strcpy (ccmr_rec.cc_no,whTotal [i].whNo);
		cc = find_rec (ccmr,&ccmr_rec,COMPARISON,"r");
		if (!cc)
		{
			if (group == (char *)0)
			{
				fprintf (ftmp, "| %-12.12s ", (i == 0) ? "WAREHOUSE   " : " ");
				fprintf (ftmp, "| %-40.40s ","TOTAL ");
				fprintf (ftmp, "| %-2.2s ",ccmr_rec.cc_no);
				fprintf (ftmp, "| %-40.40s ",ccmr_rec.name);
				fprintf (ftmp, "| %16.2f |\n",whTotal [i].grandTotal);
				tot_tot += whTotal [ i ].grandTotal;
			}
			else
			{
				fprintf (ftmp, "| %-12.12s ", (i == 0) ? group : " ");
				fprintf (ftmp, "| %-40.40s ", (i == 0) ? excf_rec.cat_desc : " ");
				fprintf (ftmp, "| %-2.2s ",ccmr_rec.cc_no);
				fprintf (ftmp, "| %-40.40s ",ccmr_rec.name);
				fprintf (ftmp, "| %16.2f |\n",whTotal [i].total);
				tot_whs += whTotal [ i ].total;
			}

			whTotal [i].total = 0.00;
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
		fprintf (ftmp, "|==================|\n");
	
		fprintf (ftmp, "| GRAND TOTAL. ");
		fprintf (ftmp, "| %-40.40s ", " ");
		fprintf (ftmp, "|    ");
		fprintf (ftmp, "| %-40.40s "," ");
		fprintf (ftmp, "| %16.2f |\n",tot_tot);
	    }
	    return;
	}
	if (i > 1)
	{
		fprintf (ftmp, "| GROUP TOTAL. ");
		fprintf (ftmp, "| %-40.40s ",excf_rec.cat_desc);
		fprintf (ftmp, "|    ");
		fprintf (ftmp, "| %-40.40s "," ");
		fprintf (ftmp, "| %16.2f |\n",tot_whs);
	}

	fprintf (ftmp, ".LRP%d\n",maxWarehouse + 1);
	fflush (ftmp);
}

void
printGrand (
 void)
{
	int	i;
	double	br_total = 0.00;

	for (i = 0;i < maxWarehouse;i++)
		br_total += whTotal [i].grandTotal;

	fprintf (ftmp, "| %-12.12s ","BRANCH TOTAL");
	fprintf (ftmp, "| %-40.40s "," ");
	fprintf (ftmp, "| %-2.2s ","  ");
	fprintf (ftmp, "| %-40.40s "," ");
	fprintf (ftmp, "| %16.2f |\n",br_total);
}

struct	whRecord *
wh_alloc (
 void)
{
	return ((struct whRecord *) malloc (sizeof (struct whRecord)));
}

/*===========================
| Get closing stock actual. |
===========================*/
double 
GetClosingStock (
	float	onHand, 
	long	hhwhHash)
{
	float	counted = 0.00;

	sttf_rec.hhwh_hash = hhwhHash;
	sprintf (sttf_rec.location, "%-10.10s", " ");
	cc = find_rec (sttf, &sttf_rec, GTEQ, "r");
	while (!cc && sttf_rec.hhwh_hash == hhwhHash)
	{
		counted += n_dec (sttf_rec.qty, inmr_rec.dec_pt);
		cc = find_rec (sttf, &sttf_rec, NEXT, "r");
	}
	return (onHand - (onHand - counted));
}

/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_rofi.c,v 5.3 2001/08/09 09:19:45 scott Exp $
|  Program Name  : ( sk_rofi.c ) 
|  Program Desc  : ( Return Of Investment Report )
|---------------------------------------------------------------------|
|  Author        : Campbell Mander.| Date Written  : 25/09/91         |
| $Log: sk_rofi.c,v $
| Revision 5.3  2001/08/09 09:19:45  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:45:41  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:19:24  scott
| Update - LS10.5
|
| Revision 4.0  2001/03/09 02:38:38  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2001/02/19 08:01:34  scott
| Updated to use app.schema
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_rofi.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_rofi/sk_rofi.c,v 5.3 2001/08/09 09:19:45 scott Exp $";

#include 	<pslscr.h>
#include 	<Costing.h>
#include 	<ml_std_mess.h>
#include 	<ml_sk_mess.h>
#include 	<get_lpno.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<twodec.h>

#define	BY_CO		 (local_rec.reportBy [0] == 'C')
#define	BY_BR		 (local_rec.reportBy [0] == 'B')
#define	BY_WH		 (local_rec.reportBy [0] == 'W')

#define	ITEM	0
#define	GROUP	1
#define	GRAND	2

#define	VAL(x)	 (incc_c_val [x])
#define	PRF(x)	 (incc_c_prf [x])

#include	"schema"

struct commRecord	comm_rec;
struct inmrRecord	inmr_rec;
struct excfRecord	excf_rec;
struct inccRecord	incc_rec;
struct ccmrRecord	ccmr_rec;

	float	*incc_c			=	&incc_rec.c_1;
	Money	*incc_c_val		=	&incc_rec.c_val_1;
	Money	*incc_c_prf		=	&incc_rec.c_prf_1;

	double	stockTurn [3]		= 	{0.00,0.00,0.00},
			stockValuation [3]	= 	{0.00,0.00,0.00},
			stockCost [3]		= 	{0.00,0.00,0.00},
			stockProfit			=	0.00,
			stockValue			=	0.00,
			groupProfit			=	0.00,
			groupValue			=	0.00,
			grandProfit			=	0.00,
			grandValue			=	0.00;

	float	margin				=	0.00,
			stockOnHand [3]		=	{0.00,0.00,0.00};
	
	long	hhbrHash;

	struct	{
		long	hhccHash;
		char	branchNumber [3];
	} valueHhcc [256];

	char	*srt_offset [256],
			dataString [200],
			currentGroup [13],
			previousGroup [13],
			currentBrNo [3];

	FILE	*fin,
			*fout,
			*fsort;

	int		noHhcc 			= 0,
			dataFound		= 0,
			detailed		= 0,
			currentMonth	=	0;

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char 	dummy [11];
	char	systemDate [11];
	long	lsystemDate;
	char	reportBy [2];
	char 	startClass [2];
	char 	startCat [12];
	char 	endClass [2];
	char 	endCat [12];
	int  	printerNumber;
} local_rec;

#include	<Costing.h>

/*======================= 
| Function Declarations |
=======================*/
char 	*IntSortRead 		(FILE *);
double 	CalcCost 			(void);
double 	WarehouseValue 		(void);
int  	CalcAgeing 			(int);
int  	heading 			(int);
int  	spec_valid 			(int);
void 	CalcData 			(void);
void 	CloseDB 			(void);
void 	GetWhValues 		(void);
void 	GroupHead 			(void);
void 	GroupTotal 			(void);
void 	InitValues 			(void);
void 	OpenDB 				(void);
void 	PrintData 			(void);
void 	PrintLine 			(void);
void 	PrintTotal 			(void);
void 	ProcessSorted 		(void);
void 	ReadData 			(void);
void 	StoreData 			(void);
void 	head 				(void);
void 	shutdown_prog 		(void);

/*=========================== 
| Main Processing Routine . |
===========================*/
int
main (
 int argc, 
 char * argv [])
{
	if (argc != 5) 
	{
		print_at (0,0, mlSkMess638, argv [0]);
		return (EXIT_FAILURE);
	}
	
	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));
	local_rec.lsystemDate = TodaysDate ();

	/*==============================
	| Read common terminal record. |
	==============================*/
	OpenDB ();

	DateToDMY (comm_rec.inv_date, NULL, &currentMonth, NULL);

	if (strncmp (argv [0], "sk_rofi_summ", 12))
		detailed = TRUE;
	else
		detailed = FALSE;

	local_rec.printerNumber = atoi (argv [1]);
	sprintf (local_rec.startClass,  "%-1.1s",   argv [2]);
	sprintf (local_rec.startCat,    "%-11.11s", argv [2] + 1);
	sprintf (local_rec.endClass, 	"%-1.1s",   argv [3]);
	sprintf (local_rec.endCat,   	"%-11.11s", argv [3] + 1);
	sprintf (local_rec.reportBy,    "%-1.1s",   argv [4]);

	InitValues ();
	ReadData ();

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*========================	
| Program exit sequence. |
========================*/
void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

void
OpenDB (
 void)
{
	abc_dbopen ("data");
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no_3");
	open_rec (excf, excf_list, EXCF_NO_FIELDS, "excf_id_no");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_hhcc_hash");
}

void
CloseDB (
 void)
{
	abc_fclose (inmr);
	abc_fclose (excf);
	abc_fclose (incc);
	abc_fclose (insf);
	abc_fclose (ccmr);
	SearchFindClose ();
	CloseCosting ();
	abc_dbclose ("data");
}

int
spec_valid (
 int field)
{
    return (EXIT_SUCCESS);        
}

int
heading (
	int	scn)
{
    return (EXIT_SUCCESS);        
}

void
InitValues (void)
{
	int	i;

	for (i = 0; i < 3; i++)
	{
		stockOnHand [i] 	= 0.00;
		stockValuation [i] 	= 0.00;
		stockCost [i] 		= 0.00;
	}
}

void
ReadData (
 void)
{
	int	i;

	GetWhValues ();

	dsp_screen ("Stock Holding Report.", comm_rec.co_no, comm_rec.co_name);

	fsort = sort_open ("rofi");

	dataFound = FALSE;

	margin 			= 0.00;
	stockProfit 	= 0.00;
	stockValue  	= 0.00;

	strcpy (inmr_rec.co_no, comm_rec.co_no);
	sprintf (inmr_rec.inmr_class,   "%-1.1s",   local_rec.startClass);
	sprintf (inmr_rec.category, 	"%-11.11s", local_rec.startCat);
	sprintf (inmr_rec.item_no,  	"%-16.16s", " ");

	cc = find_rec (inmr, &inmr_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (inmr_rec.co_no, comm_rec.co_no) &&
	       strcmp (inmr_rec.inmr_class, local_rec.endClass) <= 0)
	{
		if (!strcmp (inmr_rec.inmr_class, local_rec.endClass) &&
		    strcmp (inmr_rec.category, local_rec.endCat) > 0)
			break;

		for (i = 0; i < noHhcc; i++)
		{
			incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
			incc_rec.hhcc_hash = valueHhcc [i].hhccHash;
			cc = find_rec (incc, &incc_rec, COMPARISON, "r");
			if (!cc)
			{
				sprintf (currentBrNo, "%2.2s", valueHhcc [i].branchNumber);
				CalcData ();
			}
		}

		dataFound = TRUE;

		if (stockValue != 0.00)
			margin = (float) (stockProfit / stockValue * 100.00);
		else
			margin = 0.00;

		StoreData ();

		stockOnHand [ITEM] 		= 0.00;
		stockValuation [ITEM] 	= 0.00;
		stockCost [ITEM]    	= 0.00;
		margin        			= 0.00;
		stockProfit     		= 0.00;
		stockValue      		= 0.00;

		cc = find_rec (inmr, &inmr_rec, NEXT, "r");
	}

	InitValues ();

	if (dataFound)
		ProcessSorted ();
}

void
GetWhValues (
 void)
{
	noHhcc = 0;

	cc = find_hash (ccmr, &ccmr_rec, GTEQ, "r", 0L);
	while (!cc)
	{
		if (BY_CO && strcmp (ccmr_rec.co_no, comm_rec.co_no))
		{
			cc = find_hash (ccmr, &ccmr_rec, NEXT, "r", 0L);
			continue;
		}

		if (BY_BR && 
	            (strcmp (ccmr_rec.co_no, comm_rec.co_no) ||
		     strcmp (ccmr_rec.est_no, comm_rec.est_no)))
		{
			cc = find_hash (ccmr, &ccmr_rec, NEXT, "r", 0L);
			continue;
		}

		if (BY_WH && 
	            (strcmp (ccmr_rec.co_no, comm_rec.co_no) ||
		     strcmp (ccmr_rec.est_no, comm_rec.est_no) ||
		     strcmp (ccmr_rec.cc_no, comm_rec.cc_no)))
		{
			cc = find_hash (ccmr, &ccmr_rec, NEXT, "r", 0L);
			continue;
		}

		strcpy (valueHhcc [noHhcc].branchNumber, ccmr_rec.est_no);
		valueHhcc [noHhcc++].hhccHash = ccmr_rec.hhcc_hash;

		cc = find_hash (ccmr, &ccmr_rec, NEXT, "r", 0L);
	}
}

/*-----------------------------
| Calculate figures from incc |
-----------------------------*/
void
CalcData (
 void)
{
	double	tmp_stk_val;
	double	tmp_cost;

	stockOnHand [ITEM] += incc_rec.closing_stock;

	tmp_stk_val 	= WarehouseValue ();
	stockValuation [ITEM] += tmp_stk_val;

	tmp_cost = CalcCost ();
	stockCost [ITEM] += tmp_cost;
}

double	
WarehouseValue (void)
{
	double	avge_cost;
	double	extend;
	double	value;
	float	incc_qty;
	int		ff_error = FALSE;

	if (incc_rec.closing_stock <= 0.00)
		return (0.00);
	
	incc_qty = incc_rec.closing_stock;

	switch (inmr_rec.costing_flag [0])
	{
	case 'S':
		avge_cost = FindInsfValue (incc_rec.hhwh_hash, TRUE);
		value = out_cost (avge_cost, inmr_rec.outer_size);
		break;

	case 'F':
		avge_cost = FindIncfValue 
					(
						incc_rec.hhwh_hash, 
						incc_qty, 
						TRUE, 
						TRUE,
						inmr_rec.dec_pt
					);

		if (fifoError)
		{
			avge_cost = FindIncfValue 
						(
							incc_rec.hhwh_hash, 
							incc_qty - fifoQtyShort, 
							TRUE, 
							TRUE,
							inmr_rec.dec_pt
						);

			cc = FindInei (inmr_rec.hhbr_hash, currentBrNo, "r");
			if (cc)
				ineiRec.last_cost = 0.00;

			avge_cost += CheckIncf 
						(
							incc_rec.hhwh_hash, 
							TRUE, 
							incc_qty,
							inmr_rec.dec_pt
						);
			avge_cost /= incc_qty;

			ff_error = TRUE;
		}
		if (avge_cost <= 0.00)
			avge_cost = FindIneiCosts ("A", currentBrNo, inmr_rec.hhbr_hash);
		
		value = out_cost (avge_cost, inmr_rec.outer_size);

		break;

	case 'I':
		avge_cost = FindIncfValue 
					(
						incc_rec.hhwh_hash, 
						incc_qty, 
						TRUE, 
						FALSE,
						inmr_rec.dec_pt
					);
		if (fifoError)
		{
			avge_cost = FindIncfValue 
						(
							incc_rec.hhwh_hash, 
							incc_qty - fifoQtyShort, 
							TRUE, 
							FALSE,
							inmr_rec.dec_pt
						);

			cc = FindInei (inmr_rec.hhbr_hash, currentBrNo, "r");
			if (cc)
				ineiRec.last_cost = 0.00;

			avge_cost += CheckIncf 
						(
							incc_rec.hhwh_hash, 
							FALSE, 
							incc_qty,
							inmr_rec.dec_pt
						);
			avge_cost /= incc_qty;

			ff_error = TRUE;
		}
		if (avge_cost <= 0.00)
			avge_cost = FindIneiCosts ("A", currentBrNo, inmr_rec.hhbr_hash);

		value = out_cost (avge_cost, inmr_rec.outer_size);
		break;

	case 'A':
	case 'L':
	case 'P':
	case 'T':
		avge_cost 	= 	FindIneiCosts 
						(
							inmr_rec.costing_flag,
							currentBrNo, 
							inmr_rec.hhbr_hash
						);
		value 		= out_cost (avge_cost, inmr_rec.outer_size);
		break;

	default:
		return (0.00);
		break;
	}
	value = twodec (value);

	extend = (double) incc_qty;
	extend *= value;
	extend = twodec (extend);

	return (extend);
}

/*---------------------------------
| Return average cost of the item |
| over the last 12 months based on|
|amount of history available      | 
---------------------------------*/
double	
CalcCost (
 void)
{
	int	mths_old;
	int	i;
	int	calc_fact;
	double	tmp_cost;
	
	tmp_cost = 0.00;
	calc_fact = 0;
	mths_old = CalcAgeing (currentMonth);

	if (mths_old >= currentMonth)
	{
		for (i = 0; i < currentMonth; i++)
		{
			tmp_cost += DOLLARS (VAL (i) - PRF (i));
			stockProfit += DOLLARS (PRF (i));
			stockValue  += DOLLARS (VAL (i));
			calc_fact++;
		}

		i = currentMonth + (12 - mths_old);
		for (; i < 12; i++)
		{
			tmp_cost 	+= DOLLARS (VAL (i) - PRF (i));
			stockProfit += DOLLARS (PRF (i));
			stockValue  += DOLLARS (VAL (i));
			calc_fact++;
		}
	}
	else
	{
		for (i = (currentMonth - mths_old); i < currentMonth; i++)
		{
			tmp_cost 	+= DOLLARS (VAL (i) - PRF (i));
			stockProfit += DOLLARS (PRF (i));
			stockValue  += DOLLARS (VAL (i));
			calc_fact++;
		}
	}

	if (calc_fact < 12)
		tmp_cost *= (12.00 / (double) calc_fact);

	return (tmp_cost);
}

/*=======================================================================
| Calculate and return number of months ago the item was first stocked. |
=======================================================================*/
int
CalcAgeing (
	int	currentMonth)
{
	int		FST_STK_FOUND = FALSE;
	int		i;
	int		yrs;
	int		mths;
	int		tmp_yr;
	int		tmp_mth;
	int		year;
	int		mnth;
	int		mnths_ago;
	long	fst_stk;
	int		tmp_dmy [3];
	int		fs_dmy [3];

	mnths_ago = 11;
	/*-----------------------------------------
	| Check incc record to find first stocked |
	-----------------------------------------*/
	for (i = currentMonth; i < 12; i++)
	{
		if (incc_c_val [i] != 0.00 || incc_c_prf [i] != 0.00)
		{
			FST_STK_FOUND = TRUE;
			break;
		}
		mnths_ago--;
	}

	if (!FST_STK_FOUND)
	{
		for (i = 0; i < currentMonth; i++)
		{
			if (incc_c_val [i] != 0.00 || incc_c_prf [i] != 0.00)
			{
				FST_STK_FOUND = TRUE;
				break;
			}
			mnths_ago--;
		}
	}

	if (mnths_ago <= 0)
	{
		/*----------------------------------------------
		| No data in any of the 12  months history	   |
		----------------------------------------------*/
		return (EXIT_FAILURE);
	}
	else
	{
		/*--------------------------
		| Check if incc first stk  |
		| date is older than held  |
		| history                  |
		--------------------------*/
		yrs = mnths_ago / 12;
		mths = mnths_ago % 12;

		DateToDMY (comm_rec.inv_date, &tmp_dmy [0], &tmp_dmy [1], &tmp_dmy [2]);
		tmp_yr  = tmp_dmy [2];
		tmp_mth = tmp_dmy [1]; 

		tmp_yr -= yrs;
		tmp_mth -= mths;
		if (tmp_mth <= 0)
		{
			tmp_yr--;
			tmp_mth = 12 - (tmp_mth * -1);
		}
		
		tmp_dmy [0]	=	1;
		tmp_dmy [1]	=	tmp_mth;
		tmp_dmy [2]	=	tmp_yr;

		fst_stk = DMYToDate (tmp_dmy [0], tmp_dmy [1], tmp_dmy [2]);

		if (incc_rec.first_stocked < fst_stk)
		{
			DateToDMY (incc_rec.first_stocked, &fs_dmy [0], &fs_dmy [1], &fs_dmy [2]);
			year = fs_dmy [2];
			mnth = fs_dmy [1];

			DateToDMY (comm_rec.inv_date, &fs_dmy [0], &fs_dmy [1], &fs_dmy [2]);
			tmp_yr = fs_dmy [2];
			tmp_mth = fs_dmy [1];
	
			tmp_yr -= year;
			tmp_mth -= mnth;
			if (tmp_mth <= 0)
			{
				tmp_yr--;
				tmp_mth = 12 - (tmp_mth * -1);
			}
			mnths_ago = (12 * tmp_yr) + tmp_mth;
		}

		if (mnths_ago > 11)
			mnths_ago = 11;

		return (mnths_ago + 1);
	}
}

/*-------------------------
| Store data in sort file |
-------------------------*/
void
StoreData (
 void)
{
	
	dsp_process ("Item No : ",inmr_rec.item_no);

	sprintf (dataString,
		"%s%c%s%c%s%c%f%c%f%c%f%c%f%c%ld%c%f%c%f\n",
		inmr_rec.inmr_class,	1,	/* srt_offset =  0 */
		inmr_rec.category,		1,	/* srt_offset =  1 */
		inmr_rec.item_no,		1,	/* srt_offset =  2 */
		stockOnHand [ITEM],		1,	/* srt_offset =  3 */
		stockValuation [ITEM],	1,	/* srt_offset =  4 */
		stockCost [ITEM],		1,	/* srt_offset =  5 */
		margin,					1,	/* srt_offset =  6 */
		inmr_rec.hhbr_hash,		1,	/* srt_offset =  7 */
		stockProfit,			1,	/* srt_offset =  8 */
		stockValue);				/* srt_offset =  9 */

	sort_save (fsort, dataString);
}

void
head (
 void)
{
	if ((fout = popen ("pformat","w")) == NULL)
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);

	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (fout, ".LP%d\n",local_rec.printerNumber);
	if (BY_CO)
		fprintf (fout, ".12\n");
	else
		if (BY_BR)
			fprintf (fout, ".13\n");
		else
			fprintf (fout, ".14\n");

	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L158\n");
	fprintf (fout, 
		".ERETURN OF INVESTMENT REPORT - %s\n",
		 (detailed) ? "DETAILED" : "SUMMARY");

	fprintf (fout,".B1\n");
	fprintf (fout, ".ECOMPANY: %s\n", clip (comm_rec.co_name));

	if (BY_BR || BY_WH)
		fprintf (fout, ".EBRANCH: %s\n", clip (comm_rec.est_name));

	if (BY_WH)
		fprintf (fout, ".EWAREHOUSE: %s\n", clip (comm_rec.cc_name));

	fprintf (fout, ".EAS AT %s\n", SystemTime ());
	fprintf (fout,".B1\n");

	fprintf (fout, ".R=================");
	fprintf (fout, "=========================================");
	fprintf (fout, "=================");
	fprintf (fout, "=================");
	fprintf (fout, "=================");
	fprintf (fout, "=================");
	fprintf (fout, "=========");
	fprintf (fout, "==================\n");

	fprintf (fout, "=================");
	fprintf (fout, "=========================================");
	fprintf (fout, "=================");
	fprintf (fout, "=================");
	fprintf (fout, "=================");
	fprintf (fout, "=================");
	fprintf (fout, "=========");
	fprintf (fout, "==================\n");

	fprintf (fout, "|    GROUP /     ");
	fprintf (fout, "|            DESCRIPTION                 ");
	fprintf (fout, "|    STOCK ON    ");
	fprintf (fout, "|      STOCK     ");
	fprintf (fout, "|     COST OF    ");
	fprintf (fout, "|       STOCK    ");
	fprintf (fout, "| MARGIN ");
	fprintf (fout, "|      INDEX     |\n");

	fprintf (fout, "|  ITEM NUMBER   ");
	fprintf (fout, "|                                        ");
	fprintf (fout, "|      HAND      ");
	fprintf (fout, "|      VALUE     ");
	fprintf (fout, "|      SALES     ");
	fprintf (fout, "|        TURN    ");
	fprintf (fout, "|   %%    ");
	fprintf (fout, "|                |\n");

	PrintLine ();
	fflush (fout);
}

void
PrintLine (
 void)
{
	fprintf (fout, "|----------------");
	fprintf (fout, "|----------------------------------------");
	fprintf (fout, "|----------------");
	fprintf (fout, "|----------------");
	fprintf (fout, "|----------------");
	fprintf (fout, "|----------------");
	fprintf (fout, "|--------");
	fprintf (fout, "|----------------|\n");

	fflush (fout);

}

/*-------------------
| Process sort file |
-------------------*/
void
ProcessSorted (
 void)
{
	char	*sptr;
	int		first_time = TRUE;

	abc_selfield (inmr, "inmr_hhbr_hash");

	head ();
	
	fsort 	= sort_sort (fsort,"rofi");
	sptr 	= IntSortRead (fsort);
	while (sptr != (char *) 0)
	{
		sprintf (currentGroup, "%-1.1s%-11.11s", srt_offset [0], srt_offset [1]);
		stockOnHand [ITEM]  	= 	(float) (atof (srt_offset [3]));
		stockValuation [ITEM]  	= 	atof (srt_offset [4]);
		stockCost [ITEM]  		= 	atof (srt_offset [5]);
		margin   				= 	(float) (atof (srt_offset [6]));
		stockProfit 			= 	atof (srt_offset [8]);
		stockValue 				= 	atof (srt_offset [9]);
		hhbrHash 				= 	atol (srt_offset [7]);

		if (first_time || strcmp (previousGroup, currentGroup))
		{
			if (!first_time)
				GroupTotal ();

			if (detailed)
			{
				GroupHead ();

				if (!first_time)
					fprintf (fout, ".PA\n");
			}

			strcpy (previousGroup, currentGroup);
			first_time = FALSE;
		}

		inmr_rec.hhbr_hash	=	hhbrHash;
		cc = find_rec (inmr,&inmr_rec,COMPARISON,"r");
		if (cc)
		{
			sptr = IntSortRead (fsort);
			continue;
		}
		dsp_process ("Item No : ",inmr_rec.item_no);

		if (detailed)
			PrintData ();
		
		groupProfit				+= stockProfit;
		groupValue 				+= stockValue;
		grandProfit				+= stockProfit;
		grandValue 				+= stockValue;
		stockOnHand 	[GROUP]	+= stockOnHand 		[ITEM];
		stockValuation 	[GROUP]	+= stockValuation 	[ITEM];
		stockCost 		[GROUP] += stockCost 		[ITEM];

		stockOnHand 	[ITEM] 	= 0.00;
		stockValuation 	[ITEM] 	= 0.00;
		stockCost 		[ITEM] 	= 0.00;

		sptr = IntSortRead (fsort);
	}

	PrintTotal ();
	fprintf (fout,".EOF\n");
	pclose (fout);

	sort_delete (fsort,"rofi");
}

void
PrintData (
 void)
{
	if (stockValuation [ITEM] != 0.00)
		stockTurn [ITEM] = stockCost [ITEM] / stockValuation [ITEM];
	else
		stockTurn [ITEM] = 0.00;

	fprintf (fout, "|%-16.16s", inmr_rec.item_no);
	fprintf (fout, "|%40.40s",  inmr_rec.description);
	fprintf (fout, "|%15.2f ", stockOnHand [ITEM]);
	fprintf (fout, "|%15.2f ", stockValuation [ITEM]);
	fprintf (fout, "|%15.2f ", stockCost [ITEM]);
	fprintf (fout, "|%15.2f ", stockTurn [ITEM]);
	fprintf (fout, "| %6.2f ",  margin);
	fprintf (fout, "|%15.2f |\n", (stockTurn [ITEM] * margin));

	fflush (fout);
}

void
GroupTotal (
 void)
{
	double	grp_margin;

	if (stockValuation [GROUP] != 0.00)
		stockTurn [GROUP] = stockCost [GROUP] / stockValuation [GROUP];
	else
		stockTurn [GROUP] = 0.00;

	if (groupValue != 0.00)
		grp_margin = groupProfit / groupValue * 100.00;
	else
		grp_margin = 0.00;

	if (detailed)
	{
		fprintf (fout, ".LRP2\n");
		PrintLine ();

		fprintf 
		(
			fout, 
			"| TOTAL FOR GROUP :  %-1.1s %-11.11s                        |%15.2f |%15.2f |%15.2f |%15.2f | %6.2f |%15.2f |\n",
			previousGroup,
			previousGroup + 1,
			stockOnHand [GROUP],
			stockValuation [GROUP],
			stockCost [GROUP],
			stockTurn [GROUP],
			grp_margin,
			(stockTurn [GROUP] * grp_margin)
		);
		fflush (fout);
	}
	else
	{
		strcpy (excf_rec.co_no, comm_rec.co_no);
		sprintf (excf_rec.cat_no, "%-11.11s", previousGroup + 1);
		cc = find_rec (excf, &excf_rec, COMPARISON, "r");
		if (cc)
			sprintf (excf_rec.cat_desc, 
				"%-40.40s", 
				"Category Not Found On File");

		fprintf (fout, "!%-1.1s %-11.11s   ", previousGroup, previousGroup + 1);

		fprintf (fout, "!%40.40s",     excf_rec.cat_desc);
		fprintf (fout, "!%15.2f ",    stockOnHand [GROUP]);
		fprintf (fout, "!%15.2f ",    stockValuation [GROUP]);
		fprintf (fout, "!%15.2f ",    stockCost [GROUP]);
		fprintf (fout, "!%15.2f ",    stockTurn [GROUP]);
		fprintf (fout, "! %6.2f ",     grp_margin);
		fprintf (fout, "!%15.2f |\n", (stockTurn [GROUP] * grp_margin));
	
		fflush (fout);
	}

	stockOnHand [GRAND] 	+= stockOnHand [GROUP];
	stockValuation [GRAND] 	+= stockValuation [GROUP];
	stockCost [GRAND]  	+= stockCost [GROUP];

	stockOnHand [GROUP] 	= 0.00;
	stockValuation [GROUP] 	= 0.00;
	stockCost [GROUP]  	= 0.00;

	groupProfit 		= 0.00;
	groupValue 		= 0.00;
}

void
GroupHead (
 void)
{
	strcpy (excf_rec.co_no, comm_rec.co_no);
	sprintf (excf_rec.cat_no, "%-11.11s", currentGroup + 1);
	cc = find_rec (excf, &excf_rec, COMPARISON, "r");
	if (cc)
		sprintf (excf_rec.cat_desc, "%-40.40s", "Category Not Found On File");

	fprintf (fout, ".PD|%1.1s %11.11s   ", currentGroup, currentGroup + 1);
	fprintf (fout, "|%-40.40s", excf_rec.cat_desc);
	fprintf (fout, "|                ");
	fprintf (fout, "|                ");
	fprintf (fout, "|                ");
	fprintf (fout, "|                ");
	fprintf (fout, "|        ");
	fprintf (fout, "|                |\n");
}

void
PrintTotal (void)
{
	double	grand_margin;

	GroupTotal ();

	if (stockValuation [GRAND] != 0.00)
		stockTurn [GRAND] = stockCost [GRAND] / stockValuation [GRAND];
	else
		stockTurn [GRAND] = 0.00;

	if (grandValue != 0.00)
		grand_margin = grandProfit / grandValue * 100.00;
	else
		grand_margin = 0.00;

	PrintLine ();
	fprintf (fout, "!%-16.16s", " ");
	fprintf (fout, "     GRAND TOTALS                        ");
	fprintf (fout, "!%15.2f ", stockOnHand [GRAND]);
	fprintf (fout, "!%15.2f ", stockValuation [GRAND]);
	fprintf (fout, "!%15.2f ", stockCost [GRAND]);
	fprintf (fout, "!%15.2f ", stockTurn [GRAND]);
	fprintf (fout, "! %6.2f ", grand_margin);
	fprintf (fout, "!%15.2f |\n", (stockTurn [GRAND] * grand_margin));

	fflush (fout);
}

/*-----------------------
| Save offsets for each |
| numerical field.      |
-----------------------*/
char	*
IntSortRead (
 FILE *srt_fil)
{
	char	*sptr;
	char	*tptr;
	int		fld_no = 1;

	sptr = sort_read (srt_fil);

	if (!sptr)
	{
		return (sptr);
	}

	srt_offset [0] = sptr;

	tptr = sptr;
	while (fld_no < 10)
	{
		tptr = strchr (tptr, 1);
		if (!tptr)
			break;
		*tptr = 0;
		tptr++;

		srt_offset [fld_no++] = sptr + (tptr - sptr);
	}

	return (sptr);
}



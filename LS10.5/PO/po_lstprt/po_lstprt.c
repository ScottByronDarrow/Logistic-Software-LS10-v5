/*=====================================================================
| Copyright (C) 1998 - 2002 LogisticSoftware |
|=====================================================================|
| $Id: po_lstprt.c,v 5.3 2002/07/24 06:12:29 scott Exp $
|  Program Name  : (po_lstprt.c)
|  Program Desc  : (Late list Report)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 09/08/86         |
|---------------------------------------------------------------------|
| $Log: po_lstprt.c,v $
| Revision 5.3  2002/07/24 06:12:29  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: po_lstprt.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PO/po_lstprt/po_lstprt.c,v 5.3 2002/07/24 06:12:29 scott Exp $";

#include	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<ml_std_mess.h>
#include 	<ml_po_mess.h>
#include 	<arralloc.h>

	/*
	 * Special fields and flags.
	 */
	int		printerNumber 	= 1;	
	long	startDate		= 0L;

	double	lateQuantity 	= 0.0,
			lateAmount 		= 0.0;

#include	"schema"

struct commRecord	comm_rec;
struct inmrRecord	inmr_rec;
struct sumrRecord	sumr_rec;
struct pohrRecord	pohr_rec;
struct polnRecord	poln_rec;
struct inumRecord	inum_rec;

	FILE	*pout;

/*
 *	Structure for dynamic array.
 */
struct SortStruct
{
	char	sortCode 	[27];
	char	sortStr 	[256];
}	*sortRec;
	DArray sortDetails;
	int	sortCnt = 0;


/*
 * Function Declarations 
 */
void 	shutdown_prog 	(void);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	ProcessFile 	(void);
void 	EndReport 		(void);
void 	ReportPrint 	(void);
int 	ProcessPoln 	(long);
int		SortFunc		(const	void *,	const void *);
int 	heading 		(void);

/*
 * Main Processing Routine. 
 */
int
main (
 int argc, 
 char * argv [])
{
	if (argc < 3) 
	{
		print_at (0,0,mlPoMess721,argv [0]);
        return (EXIT_FAILURE);
	}
	printerNumber 	= atoi (argv [1]);
	startDate 		= StringToDate (argv [2]);
	if (startDate < 0L)
	{
		print_at (0,0,mlPoMess721,argv [0]);
		print_at (1,0,mlStdMess111);
        return (EXIT_FAILURE);
	}

	set_tty (); 

	OpenDB ();

	dsp_screen (" Processing Late Lists.", comm_rec.co_no,comm_rec.co_name);

	heading ();
	
	/*---------------
	| Process Data. |
	---------------*/
	ProcessFile ();

	/*-------------------
	| Print Line Items. |
	-------------------*/
	ReportPrint ();

	/*----------------
	| Finish report. |
	----------------*/
	EndReport ();
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*
 * Program exit sequence. 
 */
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*
 * Open data base files. 
 */
void
OpenDB (void)
{
	abc_dbopen ("data");
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, "sumr_hhsu_hash");
	open_rec (pohr, pohr_list, POHR_NO_FIELDS, "pohr_id_no2");
	open_rec (poln, poln_list, POLN_NO_FIELDS, "poln_hhpo_hash");
	open_rec (inum, inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
}

/*
 * Close data base files. 
 */
void
CloseDB (void)
{
	abc_fclose (inmr);
	abc_fclose (sumr);
	abc_fclose (pohr);
	abc_fclose (poln);
	abc_fclose (inum);
	abc_dbclose ("data");
}

/*
 * Process whole pohr file for company. printing any records which are  
 * earlier than startDate.  Returns: 0 if ok,non-zero if not ok.       
 */
void
ProcessFile (void)
{
	/*
	 * Allocate the initial array.
	 */
	ArrAlloc (&sortDetails, &sortRec, sizeof (struct SortStruct), 10);
	sortCnt = 0;

	strcpy (pohr_rec.co_no,comm_rec.co_no);
	strcpy (pohr_rec.br_no,"  ");
	strcpy (pohr_rec.pur_ord_no,"               ");

	/*
	 * Read whole pohr file. 
	 */
	cc = find_rec (pohr,&pohr_rec,GTEQ,"r");
	while (!cc && !strcmp (pohr_rec.co_no,comm_rec.co_no))
	{
		if (pohr_rec.status [0] != 'D')
			cc = ProcessPoln (pohr_rec.hhpo_hash);

		cc = find_rec (pohr,&pohr_rec,NEXT,"r");
	}	
}

int
ProcessPoln (
	long	hhpoHash)
{
	float	StdCnvFct 	= 1.00;
	float	PurCnvFct 	= 1.00;
	float	CnvFct		= 1.00;

	double	wk_qty = 0.00,
			balance = 0.00;

	sumr_rec.hhsu_hash	=	pohr_rec.hhsu_hash;
	if (find_rec (sumr,&sumr_rec,COMPARISON,"r"))
	{
		strcpy (sumr_rec.crd_no,"000000");
		strcpy (sumr_rec.acronym,"UNKNOWN");
	}

	poln_rec.hhpo_hash	=	hhpoHash;
	cc = find_rec (poln, &poln_rec, GTEQ, "r");
	while (!cc && poln_rec.hhpo_hash == hhpoHash)
	{	
		if (poln_rec.qty_ord - poln_rec.qty_rec <= 0.00)
		{
			cc = find_rec (poln, &poln_rec, NEXT, "r");
			continue;
		}
		if (poln_rec.due_date >= startDate)
		{
			cc = find_rec (poln, &poln_rec, NEXT, "r");
			continue;
		}
		/*
		 * Get part number. 
		 */
		inmr_rec.hhbr_hash	=	poln_rec.hhbr_hash;
		if (find_rec (inmr, &inmr_rec, COMPARISON, "r"))
		{
			sprintf (inmr_rec.description,"%-40.40s"," ");
			strcpy (inmr_rec.item_no,"Unknown part no.");
		}
		inum_rec.hhum_hash	=	inmr_rec.std_uom;
		cc = find_rec (inum, &inum_rec, COMPARISON, "r");
		StdCnvFct 	= (float) ((cc) ? 1.00 : inum_rec.cnv_fct);

		inum_rec.hhum_hash	=	poln_rec.hhum_hash;
		cc = find_rec (inum, &inum_rec, COMPARISON, "r");
		PurCnvFct 	= (float) ((cc) ? 1.00 : inum_rec.cnv_fct);
		CnvFct	=	StdCnvFct / PurCnvFct;

		dsp_process ("Item : ",inmr_rec.item_no);
		wk_qty = (double) (poln_rec.qty_ord - poln_rec.qty_rec);
		balance = wk_qty * out_cost (poln_rec.land_cst,inmr_rec.outer_size);

		/*
		 * Check the array size before adding new element.
		 */
		if (!ArrChkLimit (&sortDetails, sortRec, sortCnt))
			sys_err ("ArrChkLimit (sortRec)", ENOMEM, PNAME);

		/*
		 * Load values into array element sortCnt.
		 */
		sprintf (sortRec [sortCnt].sortCode, "%010ld%-16.16s",
				poln_rec.due_date, inmr_rec.item_no);
						
		sprintf 
		(
			sortRec [sortCnt].sortStr,
			"|%10.10s|%16.16s|%-40.40s| %4.4s |%7.2f|%10.2f|  %-6.6s  | %-9.9s|%-15.15s|",
			
			DateToString (poln_rec.due_date),
			inmr_rec.item_no,
			inmr_rec.description,
			inum_rec.uom,
			wk_qty * CnvFct,
			balance,
			sumr_rec.crd_no,
			sumr_rec.acronym,
			pohr_rec.pur_ord_no
		);
		/*
		 * Increment array counter.
		 */
		sortCnt++;
		lateQuantity += wk_qty;
		lateAmount += balance;
		cc = find_rec (poln, &poln_rec, NEXT, "r");
	}
	return (EXIT_SUCCESS);
}
  
/*
 * Routine to open the pipe to standard print and send initial data. 
 */
int
heading (void)
{
	if ((pout = popen ("pformat","w")) == 0) 
		sys_err ("Error in pformat During (POPEN) ",errno,PNAME);

	fprintf (pout, ".START%s<%s>\n", DateToString (comm_rec.crd_date), PNAME);
	fprintf (pout,".LP%d\n",printerNumber);
	fprintf (pout,".PI12\n");
	fprintf (pout,".13\n");
	fprintf (pout,".L158\n");
	fprintf (pout,".B1\n");
	fprintf (pout,".ESUPPLIER LATE LIST REPORT\n");
	fprintf (pout,".CSorted by Due Date By Item Number.\n");
	fprintf (pout,".E%s AS AT %s\n",clip (comm_rec.co_short),SystemTime ());
	fprintf (pout,".B1\n");
	fprintf (pout,".ESTART DATE: %s\n",DateToString (startDate));
	fprintf (pout,".B1\n");

	fprintf (pout, ".R===========");
	fprintf (pout, "=================");
	fprintf (pout, "=========================================");
	fprintf (pout, "=======");
	fprintf (pout, "========");
	fprintf (pout, "===========");
	fprintf (pout, "===========");
	fprintf (pout, "===========");
	fprintf (pout, "=================\n");

	fprintf (pout, "===========");
	fprintf (pout, "=================");
	fprintf (pout, "=========================================");
	fprintf (pout, "=======");
	fprintf (pout, "========");
	fprintf (pout, "===========");
	fprintf (pout, "===========");
	fprintf (pout, "===========");
	fprintf (pout, "=================\n");

	fprintf (pout, "|   DATE   ");
	fprintf (pout, "|     PART       ");
	fprintf (pout, "|      DESCRIPTION                       ");
	fprintf (pout, "| UOM. ");
	fprintf (pout, "| QUAN. ");
	fprintf (pout, "|  AMOUNT  ");
	fprintf (pout, "| SUPPLIER ");
	fprintf (pout, "| SUPPLIER ");
	fprintf (pout, "|      P.O.     |\n");

	fprintf (pout, "|   DUE    ");
	fprintf (pout, "|    NUMBER      ");
	fprintf (pout, "|                                        ");
	fprintf (pout, "|      ");
	fprintf (pout, "|       ");
	fprintf (pout, "|OF SHIPMNT");
	fprintf (pout, "|  NUMBER  ");
	fprintf (pout, "| ACRONYM  ");
	fprintf (pout, "|    NUMBER     |\n");

	fprintf (pout, "|----------");
	fprintf (pout, "|----------------");
	fprintf (pout, "|----------------------------------------");
	fprintf (pout, "|------");
	fprintf (pout, "|-------");
	fprintf (pout, "|----------");
	fprintf (pout, "|----------");
	fprintf (pout, "|----------");
	fprintf (pout, "|---------------|\n");
	fflush (pout);

    return (EXIT_SUCCESS);
}

/*
 * Routine to end report. Prints bottom line totals. 
 */
void
EndReport (void)
{
	fprintf (pout,"|          |%-16.16s|%40.40s|      |%7.0f|%10.2f|          |          |               |\n","TOTAL LATE"," ",lateQuantity,lateAmount);
	fprintf (pout,".EOF\n");
	pclose (pout);
}

void
ReportPrint (void)
{
	char	workDate [11];
	long	oldDate = 0L,
			newDate = 0L;

	int		firstTime = 1,
			i;

	/*
	 * Sort the array in item description order.
	 */
	qsort (sortRec, sortCnt, sizeof (struct SortStruct), SortFunc);

	for (i = 0; i < sortCnt; i++)
	{
		sprintf (workDate, "%10.10s", sortRec [i].sortCode);
		newDate = atol (workDate);
		if (firstTime)
			oldDate = newDate;

		firstTime = 0;

		sprintf (err_str, "%-16.16s", sortRec [i].sortCode + 10);
		dsp_process ("Item : ",err_str);
	
		if (newDate != oldDate)
		{
			fprintf (pout, "|----------");
			fprintf (pout, "|----------------");
			fprintf (pout, "|----------------------------------------");
			fprintf (pout, "|------");
			fprintf (pout, "|-------");
			fprintf (pout, "|----------");
			fprintf (pout, "|----------");
			fprintf (pout, "|----------");
			fprintf (pout, "|---------------|\n");
			oldDate = newDate;
		}
		fprintf (pout,"%s\n",sortRec [i].sortStr);
	}
	/*
	 *	Free up the array memory
	 */
	ArrDelete (&sortDetails);
}
int 
SortFunc (
 const void *a1, 
 const void *b1)
{
	int	result;
	const struct SortStruct a = * (const struct SortStruct *) a1;
	const struct SortStruct b = * (const struct SortStruct *) b1;

	result = strcmp (a.sortCode, b.sortCode);

	return (result);
}

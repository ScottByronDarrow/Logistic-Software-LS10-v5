/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: lead_upd.c,v 5.3 2002/07/24 04:34:03 scott Exp $
|  Program Name  : (po_lead_up.c  )                                   |
|  Program Desc  : (Purchase order lead time update.            )     |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 23/08/93         |
|---------------------------------------------------------------------|
| $Log: lead_upd.c,v $
| Revision 5.3  2002/07/24 04:34:03  scott
| Updated to add new sort routines and remove old disk based routines.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: lead_upd.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PO/po_lead_upd/lead_upd.c,v 5.3 2002/07/24 04:34:03 scott Exp $";

#include <pslscr.h>
#include <dsp_screen.h>
#include <dsp_process.h>
#include <ml_std_mess.h>
#include <ml_po_mess.h>
#include <arralloc.h>

	int		printerNumber = 1;	/* Line printer number			*/
	int		noInis = FALSE;

	int		sortOpen = FALSE;

#include	"schema"

struct commRecord	comm_rec;
struct inmrRecord	inmr_rec;
struct sumrRecord	sumr_rec;
struct suphRecord	suph_rec;
struct inisRecord	inis_rec;

	char 	*data	=	"data";

	char	data_str [70];
	
	FILE	*pout;
	FILE	*fsort;

	int		sortByNumber 	= FALSE,
			numberRecords 	= 0;

	float	envVarPoLeadThresh	=	0.0,
			leadTimes			=	0.0,
			leadTotal			=	0.0;

	long	previousHhsu		=	0L;

	char	envVarPoShipDefault [2];

/*
 *	Structure for dynamic array.
 */
struct SortStruct
{
	char	sortCode [26];
	char	customerNo	[sizeof sumr_rec.crd_no]; 
	char	acronym		[sizeof sumr_rec.acronym];
	char	itemNo		[sizeof inmr_rec.item_no];
	char	shipMethod	[sizeof suph_rec.ship_method];
	char	branchNo	[sizeof sumr_rec.est_no];
	long	calulateDays;
	long	hhsuHash; 
	long	hhbrHash; 
}	*sortRec;
	DArray sortDetails;
	int	sortCnt = 0;

/*
 * Function Declarations 
 */
void 	shutdown_prog 	(void);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
void 	ProcessSumr 	(void);
void 	ProcessSuph 	(void);
void 	ProcessData 	(void);
void 	ProcessBefore 	(char *, long, long, char *);
void 	ProcessAfter 	(char *);
void 	endReport 		(void);
int 	heading 		(void);
int		SortFunc		(const	void *,	const void *);

/*
 * Main Processing Routine. 
 */
int
main (
 int argc, 
 char * argv [])
{
	char	*sptr;

	if (argc < 3) 
	{
		print_at (0,0,mlPoMess728,argv [0]);
        return (EXIT_FAILURE);
	}

	sptr = chk_env ("PO_LEAD_THRESH");
	envVarPoLeadThresh = (float) ( (sptr == (char *)0) ? 0.00 : atof (sptr));

	sptr = chk_env ("PO_SHIP_DEFAULT");
	sprintf (envVarPoShipDefault, "%-1.1s", (sptr == (char *)0) ? "S" : sptr);

	printerNumber = atoi (argv [1]);

	sortByNumber = (argv [2] [0] == 'N') ? TRUE : FALSE;

	OpenDB ();

	dsp_screen ("Supplier lead time Print/Update .",
									comm_rec.co_no, comm_rec.co_name);
	heading ();

	ProcessSumr ();

	endReport ();

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
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (inmr ,inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (sumr ,sumr_list, SUMR_NO_FIELDS, "sumr_id_no");
	open_rec (inis ,inis_list, INIS_NO_FIELDS, "inis_id_no");
	open_rec (suph ,suph_list, SUPH_NO_FIELDS, "suph_id_no2");
}

/*
 * Close data base files. 
 */
void
CloseDB (void)
{
	abc_fclose (inmr);
	abc_fclose (sumr);
	abc_fclose (inis);
	abc_fclose (suph);
	abc_dbclose (data);
}

/*
 * Process whole sumr file. 
 */
void
ProcessSumr (void)
{
	/*
	 * Process supplier master file. 
	 */
	strcpy (sumr_rec.co_no, comm_rec.co_no);
	strcpy (sumr_rec.est_no, "  ");
	strcpy (sumr_rec.crd_no, "      ");
	cc = find_rec (sumr, &sumr_rec, GTEQ, "r"); 
	while (!cc && !strcmp (sumr_rec.co_no, comm_rec.co_no))
	{
		dsp_process ("Supplier", (sortByNumber) 
								? sumr_rec.crd_no : sumr_rec.acronym);

		/*
		 * Check for active Supplier Purchase history records and  
         * update them from a status "A" to a status of "U".       
		 */
		suph_rec.hhsu_hash = sumr_rec.hhsu_hash;
	 	strcpy (suph_rec.status, "A");
		cc = find_rec (suph, &suph_rec, GTEQ, "r");
		while (!cc && suph_rec.hhsu_hash == sumr_rec.hhsu_hash &&
	 			       suph_rec.status [0] == 'A')
		{
			if (suph_rec.ship_method [0] == ' ')
				strcpy (suph_rec.ship_method, envVarPoShipDefault);

			ProcessSuph ();

			strcpy (suph_rec.status, "U");
			cc = abc_update (suph, &suph_rec);
			if (cc)
				file_err (cc, "suph", "DBUPDATE");

			suph_rec.hhsu_hash = sumr_rec.hhsu_hash;
	 		strcpy (suph_rec.status, "A");
			cc = find_rec (suph, &suph_rec, GTEQ, "r");
		}
		cc = find_rec (sumr, &sumr_rec, NEXT, "r"); 
	}
	if (sortOpen)
	{
		abc_selfield (sumr, "sumr_hhsu_hash");
		ProcessData ();
		/*
		 *	Free up the array memory
		 */
		ArrDelete (&sortDetails);
	}
	return;
}

/*
 * Process suph file. 
 */
void
ProcessSuph (void)
{

	long	calculateDays = 0L;

	if (!sortOpen)
	{	
		/*
		 * Allocate the initial array.
		 */
		ArrAlloc (&sortDetails, &sortRec, sizeof (struct SortStruct), 10);
		sortCnt = 0;
		sortOpen = TRUE;
	}
	/*
     *       Fields within sort string.                  Offset 
	 *
	 * (1)  Supplier name or acronym                         0   
	 * (2)  Item number.                                    10   
	 * (3)  Shipment method                                 27   
	 * (4)  Lead times from suph_ord_date - suph_rec_date.  29   
	 * (5)  Branch number to get correct inis record.       38   
	 * (6)  hhsu_hash                                       41   
	 * (7)  hhbr_hash                                       52   
	 *
	 */
	
	inmr_rec.hhbr_hash = suph_rec.hhbr_hash;
	cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
	if (cc)
		return;
	
	calculateDays = suph_rec.rec_date - suph_rec.ord_date;
											
	/*
	 * Load values into array element sortCnt.
	 */
	sprintf 
	(
		sortRec [sortCnt].sortCode,	
		"%-9.9s%-16.16s",
		(sortByNumber) ? sumr_rec.crd_no : sumr_rec.acronym,
		inmr_rec.item_no
	);
	strcpy (sortRec [sortCnt].customerNo,	sumr_rec.crd_no);
	strcpy (sortRec [sortCnt].acronym,		sumr_rec.acronym);
	strcpy (sortRec [sortCnt].itemNo,		inmr_rec.item_no);
	strcpy (sortRec [sortCnt].shipMethod,	suph_rec.ship_method);
	strcpy (sortRec [sortCnt].branchNo,		suph_rec.br_no);
	sortRec [sortCnt].calulateDays 	= calculateDays;
	sortRec [sortCnt].hhsuHash 		= sumr_rec.hhsu_hash;
	sortRec [sortCnt].hhbrHash 		= inmr_rec.hhbr_hash;
	/*
	 * Increment array counter.
	 */
	sortCnt++;
}

/*
 * Mail processing routine to read and process work file. 
 */
void
ProcessData (void)
{
	char	itemNumber 		[17],
			supplierNumber 	[9],
			shipMethod 		[2],
			branchNumber 	[3],
			prevShipMethod 	[2];

	long	prevHhbrHash = 0L,
			hhsuHash	 = 0L,
			hhbrHash	 = 0L;

	int		firstTime = TRUE,
			i; 

	/*
	 * Sort the array in item description order.
	 */
	qsort (sortRec, sortCnt, sizeof (struct SortStruct), SortFunc);
	
	for (i = 0; i < sortCnt; i++)
	{
		strcpy (supplierNumber,  sortRec [i].acronym);
		strcpy (itemNumber,    	 sortRec [i].itemNo);
		strcpy (shipMethod,		 sortRec [i].shipMethod);
		strcpy (branchNumber,  	 sortRec [i].branchNo);
	
		leadTimes = (float) sortRec [i].calulateDays;
		hhsuHash  = sortRec [i].hhsuHash;
		hhbrHash  = sortRec [i].hhbrHash;

		if (firstTime ||  
				hhbrHash != prevHhbrHash || 
				strcmp (prevShipMethod, shipMethod))
		{
			if (!firstTime)
				ProcessAfter (prevShipMethod);

			ProcessBefore (branchNumber, hhsuHash, hhbrHash, shipMethod);

			strcpy (prevShipMethod, shipMethod);
			prevHhbrHash = hhbrHash;
		}
		firstTime = FALSE;
		leadTotal = leadTotal + leadTimes;
		numberRecords++;
	}
	ProcessAfter (prevShipMethod);
}

/*
 * Before group of code. 
 */
void
ProcessBefore (
	char	*branchNumber,
	long	hhsuHash,
	long	hhbrHash,
	char	*shipMethod)
{
	noInis	=	TRUE;

	inis_rec.hhbr_hash = hhbrHash;
	inis_rec.hhsu_hash = hhsuHash;
	strcpy (inis_rec.co_no, comm_rec.co_no);
	strcpy (inis_rec.br_no, branchNumber);
	strcpy (inis_rec.wh_no, "  ");
	cc = find_rec (inis, &inis_rec, GTEQ, "r");
	if (!cc && inis_rec.hhbr_hash == hhbrHash &&
			   inis_rec.hhsu_hash == hhsuHash)
	{
		noInis	=	FALSE;
	}
	else
	{
		inis_rec.hhbr_hash = hhbrHash;
		inis_rec.hhsu_hash = hhsuHash;
		strcpy (inis_rec.co_no, comm_rec.co_no);
		strcpy (inis_rec.br_no, "  ");
		strcpy (inis_rec.wh_no, "  ");
		cc = find_rec (inis, &inis_rec, GTEQ, "r");
		if (!cc && inis_rec.hhbr_hash == hhbrHash &&
			   	   inis_rec.hhsu_hash == hhsuHash)
		{
			noInis	=	FALSE;
		}
	}
	/*
	 * If supplier changes then print new heading for supplier. 
	 */
	if (previousHhsu != hhsuHash)
	{
		sumr_rec.hhsu_hash	=	hhsuHash;
		cc = find_rec (sumr, &sumr_rec, EQUAL, "r");
		if (cc)
			file_err (cc, "sumr", "DBFIND");

		fprintf (pout, "| %-9.9s", (sortByNumber) 
										? sumr_rec.crd_no 
										: sumr_rec.acronym);
		fprintf (pout, "| %-40.40s", sumr_rec.crd_name);

		previousHhsu = hhsuHash;
	}
	else
	{
		fprintf (pout, "| %9.9s",  " ");
		fprintf (pout, "| %40.40s"," ");
	}
	inmr_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, "inmr", "DBFIND");

	dsp_process ("Item", inmr_rec.item_no);

	fprintf (pout, "| %-16.16s ", inmr_rec.item_no);
	fprintf (pout, "| %-40.40s", inmr_rec.description);

	if (shipMethod [0] == 'A')
		fprintf (pout, "| AIR  ");
	else if (shipMethod [0] == 'S')
		fprintf (pout, "| SEA  ");
	else if (shipMethod [0] == 'R')
		fprintf (pout, "| RAIL ");
	else if (shipMethod [0] == 'L')
		fprintf (pout, "| LAND ");
	else
		fprintf (pout, "| ???? ");

	fflush (pout);

}
/*
 * After group of code. 
 */
void
ProcessAfter (
	char	*prevShipMethod)
{
	float	diff;
	float	new_lead = (float) 0;
	float	old_lead;

	if (numberRecords > 0)
		new_lead = leadTotal / (float) numberRecords;

	if (noInis)
		fprintf (pout, "|  N/A  | %5.1f | NO SUPP.|\n", new_lead);
	else
	{
		if (prevShipMethod [0] == 'L')
				old_lead = inis_rec.lnd_time;

		else if (prevShipMethod [0] == 'S')
				old_lead = inis_rec.sea_time;

		else if (prevShipMethod [0] == 'A')
				old_lead = inis_rec.air_time;
		else
				old_lead = 0.00;

		if (old_lead == 0.00)
			diff = 0.00;
		else
			diff = (float) (fabs ( ( (new_lead - old_lead) / old_lead) * 100));

		if (diff < envVarPoLeadThresh && envVarPoLeadThresh > 0.00 && new_lead != old_lead)
		{
			if (inis_rec.dflt_lead [0] == prevShipMethod [0])
				inis_rec.lead_time = new_lead;

			if (prevShipMethod [0] == 'A')
				inis_rec.air_time = new_lead;

			if (prevShipMethod [0] == 'S')
				inis_rec.sea_time = new_lead;

			if (prevShipMethod [0] == 'L')
				inis_rec.lnd_time = new_lead;

			cc = abc_update (inis, &inis_rec);
			if (cc)
				file_err (cc, inis, "DBUPDATE");

			fprintf (pout, "|%6.1f ", old_lead);
			fprintf (pout, "|%6.1f ", new_lead);
			fprintf (pout, "|%8.2f |  %s\n", diff, (new_lead < 0.00) ? "-ve" : " ");
		}
		else
		{
			fprintf (pout, "|%6.1f ", old_lead);
			fprintf (pout, "|%6.1f ", new_lead);
			if (new_lead == old_lead)
				fprintf (pout, "|  SAME.  |\n");
			else
				fprintf (pout, "|%8.2f |*\n", diff);
		}
	}
	abc_unlock (inis);

	leadTotal = 0L;
	numberRecords = 0;
}
	
/*
 * Routine to open the pipe to standard print and send initial data. 
 */
int
heading (
 void)
{
	if ( (pout = popen ("pformat","w")) == 0)
		file_err (errno, "pformat", "POPEN");

	fprintf (pout, ".START%s<%s>\n", DateToString (comm_rec.inv_date), PNAME);
	fprintf (pout, ".LP%d\n", printerNumber);
	fprintf (pout, ".PI12\n");
	fprintf (pout, ".12\n");
	fprintf (pout, ".L158\n");
	fprintf (pout, ".B1\n");
	fprintf (pout, ".ESUPPLIER LEAD TIME UPDATE REPORT\n");
	fprintf (pout, ".B1\n");
	fprintf (pout, ".E%s AS AT %s\n",clip (comm_rec.co_name),SystemTime ());
	fprintf (pout, ".B1\n");
	fprintf (pout, ".CNOTE : (1) Lead time Threshold : %% %6.2f /  (2) '*' indicates supplier record not updated.\n", envVarPoLeadThresh);

	fprintf (pout, ".R===========");
	fprintf (pout, "==========================================");
	fprintf (pout, "===================");
	fprintf (pout, "==========================================");
	fprintf (pout, "==================================\n");

	fprintf (pout, "===========");
	fprintf (pout, "==========================================");
	fprintf (pout, "===================");
	fprintf (pout, "==========================================");
	fprintf (pout, "==================================\n");

	fprintf (pout, "| SUPPLIER ");
	fprintf (pout, "|                SUPPLIER                 ");
	fprintf (pout, "|       ITEM       ");
	fprintf (pout, "|                   ITEM                  ");
	fprintf (pout, "|     L E A D    T I M E S       |\n");

	if (sortByNumber)
		fprintf (pout, "|  NUMBER  ");
	else
		fprintf (pout, "| ACRONYM  ");
	fprintf (pout, "|                  NAME                   ");
	fprintf (pout, "|      NUMBER      ");
	fprintf (pout, "|                DESCRIPTION              ");
	fprintf (pout, "|METHOD|  OLD  |  NEW  | %% DIFF. |\n");

	fprintf (pout, "|----------");
	fprintf (pout, "|-----------------------------------------");
	fprintf (pout, "|------------------");
	fprintf (pout, "|-----------------------------------------");
	fprintf (pout, "|------|-------|-------|---------|\n");

	fflush (pout);
    return (EXIT_SUCCESS);
}

/*
 * Routine to end report. Prints bottom line totals. 
 */
void
endReport (void)
{
	fprintf (pout,".EOF\n");
	pclose (pout);
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

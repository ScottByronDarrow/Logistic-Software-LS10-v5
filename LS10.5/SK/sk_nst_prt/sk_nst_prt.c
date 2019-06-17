/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_nst_prt.c,v 5.3 2002/03/08 02:53:28 scott Exp $
|  Program Desc  : (Print Stock Take Sheets (New).       	     )    |	
|---------------------------------------------------------------------|
|  Author        : Roger Gibbison  | Date Written  : 21/03/89         |
|---------------------------------------------------------------------|
| $Log: sk_nst_prt.c,v $
| Revision 5.3  2002/03/08 02:53:28  scott
| Updated to remove disk based sorting.
|
| Revision 5.2  2001/08/09 09:19:27  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:45:29  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:16:55  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:38:13  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.2  2000/12/22 03:27:04  ramon
| Updated to correct the errors when compiled in LS10-GUI.
|
| Revision 3.1  2000/11/20 07:40:20  scott
| New features related to 3PL environment
| New features related to Number Plates
| All covered in release 3 notes
|
| Revision 3.0  2000/10/10 12:20:48  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.2  2000/07/28 08:34:51  scott
| S/C 3101 - Change related to extra description.
| Updated to add environment variable SK_ST_DESC_SHOW to print extra description
| if required, simply  taking it out was not ideal.
| Also fixes problem with stock take sheets related to printing multiple lines
| when SAME locations exists for SAME item with different UOM.
|
| Revision 2.1  2000/07/21 17:44:39  ambhet
| SC#3101 - Modified to remove the function wherein it prints the extra description.
|
| Revision 2.0  2000/07/15 09:11:30  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.16  1999/11/03 07:32:16  scott
| Updated to fix small compile errors due to -Wall flag being set.
|
| Revision 1.15  1999/10/08 05:32:43  scott
| First Pass checkin by Scott.
|
| Revision 1.14  1999/06/20 05:20:25  scott
| Updated to add log for cvs + remove old read_comm + fixed all warnings.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_nst_prt.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_nst_prt/sk_nst_prt.c,v 5.3 2002/03/08 02:53:28 scott Exp $";

#define	NO_SCRGEN
#include 	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include	<twodec.h>
#include	<ml_std_mess.h>
#include	<ml_sk_mess.h>
#include 	<arralloc.h>

#define	GROUP			0
#define	ITEM			1
#define	ABC				2
#define	LOCN			3
#define	DF_PAGE_LEN		48

#define	LCL_DESC	p_types [printoutType]._desc
#define	LCL_INMR	p_types [printoutType]._inmr

#define	BY_ITEM		 (printoutType == ITEM)
#define	BY_LOCN		 (printoutType == LOCN)

#define	SERIAL		 (inmr_rec.serial_item [0] == 'Y')
#define	MODE_OK		 (incc_rec.stat_flag [0] == mode [0])
#define	NON_FREEZE	 (incc_rec.stat_flag [0] == '0' && envSkStPfrz)

#include	"schema"

struct commRecord	comm_rec;
struct ccmrRecord	ccmr_rec;
struct inexRecord	inex_rec;
struct inmrRecord	inmr_rec;
struct inmrRecord	inmr2_rec;
struct inccRecord	incc_rec;
struct inumRecord	inum_rec;
struct excfRecord	excf_rec;
struct inscRecord	insc_rec;
struct inwuRecord	inwu_rec;
struct inloRecord	inlo_rec;
struct inloRecord	inlo2_rec;

	char 	*inmr2 = "inmr2", 
			*inlo2 = "inlo2", 
			*data = "data";

	char	lower [17], 
			upper [17], 
			mode [2],
			*envSkIvalClass,
 			*result;

	int		printerNumber, 
			inexPrinted 	= FALSE, 
			printoutType	= 0,
			envSkStExpUom 	= FALSE,
			envSkStPfrz 	= FALSE,
			envSkStPageLen	= 48,
			envMultLoc 		= FALSE, 
			envSkStShowQty 	= FALSE, 
			envSkStSpace 	= 0,
			envSkStDescShow	= 0; 

	FILE	*fout;

	long	reportLineCounter = 0L,
			reportPageCounter = 0L;

	struct	{
		char	*_desc;
		char	*_inmr;
	} p_types [] = {
		{"GROUP", 		"inmr_id_no_3"	}, 
		{"ITEM NUMBER", "inmr_id_no"	}, 
		{"ABC CODE", 	"inmr_id_no"	}, 
		{"LOCATION", 	"inmr_id_no"	}, 
	};
/*
 *	Structure for dynamic array.
 */
struct SortStruct
{
	char	sortCode [28];
	char	locNo		[sizeof inlo_rec.location]; 
	char	itemNo		[sizeof inmr_rec.item_no];
	float	qty;
	long	hhbrHash; 
	long	hhwhHash;
}	*sortRec;
	DArray sortDetails;
	int	sortCnt = 0;

int		SortFunc			(const	void *,	const void *);

	char	defaultQtyMask [15];
	char	reportQtyMask [30];
	char	OldLoc [11],
			NewLoc [11];
	int		FirstLoc = TRUE;

/*
 * Function Declarations 
 */
int		ValidAbc 		 	 (void);
int		ValidGroup 		 	 (void);
int		ValidItem 		 	 (void);
int		ValidLocation 	 	 (char *);
void	CheckStakePage 	 	 (void);
void	CloseDB 		 	 (void);
void	Heading 		 	 (void);
void	OpenDB			 	 (void);
void	PrintInex 		 	 (void);
void	PrintLine 		 	 (int);
void	PrintSheet 		 	 (char *);
void 	PrintLocationSheet 	 (void);
void	Process 		 	 (void);
void	ProcessByGroup	 	 (void);
void	ProcessByItem 	 	 (void);
void	ProcessByStdLoc 	 (void);
void	ProcessLocation 	 (void);
void	ReadMiscFiles	 	 (void);
void	SaveLine 		 	 (int);
void	SaveLocationLine 	 (void);
void	shutdown_prog	 	 (void);

/*
 * Main Processing Routine 
 */
int
main (
 int argc, 
 char * argv [])
{
	char	*sptr;
	int		after,
			before;

	if (argc != 6)
	{
		print_at (0,0,mlSkMess574, argv [0]);
		return (EXIT_FAILURE);
	}

	sptr = chk_env ("SK_QTY_MASK");
	if (sptr == (char *)0)
		strcpy (defaultQtyMask, "NNNNNNN.NNNNNN");
	else
		strcpy (defaultQtyMask, sptr);
	before = strlen (defaultQtyMask);
	sptr = strrchr (defaultQtyMask, '.');
	if (sptr)
		after = (int) ((sptr + strlen (sptr) - 1) - sptr);
	else
		after = 0;
	if (after == 0)
		sprintf (reportQtyMask, "%%%df", before);
	else
		sprintf (reportQtyMask, "%%%d.%df", before, after);

	sptr = chk_env ("SK_ST_PFRZ");
	if (sptr != (char *)0)
		envSkStPfrz = atoi (sptr);

	sptr = chk_env ("SK_ST_DESC_SHOW");
	envSkStDescShow = (sptr == (char *)0) ? 0 : atoi (sptr);
			
	sptr = chk_env ("SK_ST_EXP_UOM");
	envSkStExpUom = (sptr == (char *)0) ? 0 : atoi (sptr);

	sptr = chk_env ("SK_ST_PAGE_LEN");
	envSkStPageLen = (sptr == (char *)0) ? DF_PAGE_LEN : atoi (sptr);

	sptr = chk_env ("SK_ST_SPACE");
	envSkStSpace = (sptr == (char *)0) ? 0 : atoi (sptr);
	if (envSkStSpace < 0 || envSkStSpace > 10)
		envSkStSpace = 0;
		
	printerNumber = atoi (argv [1]);

	sptr = chk_env ("SK_IVAL_CLASS");
	envSkIvalClass = (sptr == (char *)0) ? strdup ("ZLPN") : strdup (sptr);
	upshift (envSkIvalClass); 

	switch (argv [4] [0])
	{
	case	'G':
	case	'g':
		sprintf (lower, "%-12.12s", argv [2]);
		sprintf (upper, "%-12.12s", argv [3]);
		printoutType = GROUP;
		break;

	case	'I':
	case	'i':
		sprintf (lower, "%-16.16s", argv [2]);
		sprintf (upper, "%-16.16s", argv [3]);
		printoutType = ITEM;
		break;

	case	'A':
	case	'a':
		sprintf (lower, "%-1.1s", argv [2]);
		sprintf (upper, "%-1.1s", argv [3]);
		printoutType = ABC;
		break;

	case	'L':
	case	'l':
		sprintf (lower, "%-10.10s", argv [2]);
		sprintf (upper, "%-10.10s", argv [3]);
		printoutType = LOCN;
		break;

	default:
		printf ("<printoutType> - G (roup\n\r");
		printf ("             - I (tem Number\n\r");
		printf ("             - A (BC Code\n\r");
		printf ("             - L (ocation\n\r");
		return (EXIT_FAILURE);
	}

	sprintf (mode, "%-1.1s", argv [5]);

	sptr = chk_env ("MULT_LOC");
	if (sptr != (char *)0)
		envMultLoc = atoi (sptr);

	sptr = chk_env ("SK_ST_SHOW_QTY");
	if (sptr != (char *)0)
		envSkStShowQty = atoi (sptr);

	OpenDB ();
	
	abc_selfield (inlo, "inlo_id_loc");

	Process ();

	shutdown_prog ();
	return (EXIT_SUCCESS);
}

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
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	ReadMiscFiles ();

	abc_alias (inmr2, inmr);
	abc_alias (inlo2, inlo);

	open_rec (inlo,	inlo_list, INLO_NO_FIELDS, "inlo_mst_id");
	open_rec (inlo2,inlo_list, INLO_NO_FIELDS, "inlo_location");
	open_rec (inex, inex_list, INEX_NO_FIELDS, "inex_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, LCL_INMR);
	open_rec (inmr2,inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (inwu, inwu_list, INWU_NO_FIELDS, "inwu_id_no");
	open_rec (inum, inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	open_rec (excf, excf_list, EXCF_NO_FIELDS, "excf_id_no");
}

void
CloseDB (
 void)
{
	insc_rec.page_no = reportPageCounter + 1;
	cc = abc_update (insc, &insc_rec);
	if (cc)
		sys_err ("Error in insc During (DBFIND)", cc, PNAME);
	
	abc_unlock (insc);
		
	abc_fclose (inmr);
	abc_fclose (incc);
	abc_fclose (inum);
	abc_fclose (inwu);
	abc_fclose (inex);
	abc_fclose (excf);
	abc_fclose (insc);
	abc_fclose (inlo2);

	abc_dbclose (data);
}

void
ReadMiscFiles (
 void)
{
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");

	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, comm_rec.est_no);
	strcpy (ccmr_rec.cc_no, comm_rec.cc_no);
	cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, (char *)ccmr, "DBFIND");

	abc_fclose (ccmr);

	open_rec (insc, insc_list, INSC_NO_FIELDS, "insc_id_no");
	insc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	strcpy (insc_rec.stake_code, mode);
	cc = find_rec (insc, &insc_rec, COMPARISON, "u");
	if (cc)
		file_err (cc, "insc", "DBFIND");

	reportPageCounter = insc_rec.page_no + 1L;
}

void
Heading (void)
{
	if ((fout = popen ("pformat", "w")) == 0)
		sys_err ("Error in pformat during (POPEN)", errno, PNAME);

	strcpy (err_str, insc_rec.description);

	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (fout, ".PL200\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".LP%d\n", printerNumber);
	fprintf (fout, ".14\n");
	fprintf (fout, ".PI16\n");
	fprintf (fout, ".L156\n");
	fprintf (fout, ".PG%ld\n", reportPageCounter);
	fprintf (fout, ".ESTOCK TAKE SHEET BY %s\n", LCL_DESC);
	fprintf (fout, ".ESelection : %s %s\n", insc_rec.stake_code, clip (err_str));
	fprintf (fout, ".E%s %s\n", comm_rec.co_no, clip (comm_rec.co_name));
	fprintf (fout, ".E%s %s\n", comm_rec.est_no, clip (comm_rec.est_name));
	fprintf (fout, ".E%s %s\n", comm_rec.cc_no, clip (comm_rec.cc_name));
	fprintf (fout, ".ECreated At %s %s   / %s\n", insc_rec.start_time, 
					     DateToString (insc_rec.start_date),
						 (envSkStExpUom) ? "NOTE UOM's ARE SHOWN AS ONE LINE PER UOM" : " ");
	fprintf (fout, ".EFrom %s To %s\n", lower, upper);

	fprintf (fout, ".R===================");
	fprintf (fout, "=============");
	fprintf (fout, "===========================================");
	fprintf (fout, "=======");
	if (envSkStShowQty)
		fprintf (fout, "=================");
	fprintf (fout, "=========================================");
	fprintf (fout, "============\n");

	fprintf (fout, "===================");
	fprintf (fout, "=============");
	fprintf (fout, "===========================================");
	fprintf (fout, "=======");
	if (envSkStShowQty)
		fprintf (fout, "=================");
	fprintf (fout, "=========================================");
	fprintf (fout, "============\n");

	fprintf (fout, (BY_LOCN) ? "|  LOCATION  " : "|   ITEM  NUMBER   ");
	fprintf (fout, (BY_LOCN) ? "|   ITEM  NUMBER   " : "|  LOCATION  ");
	fprintf (fout, "|  D E S C R I P T I O N                   ");
	fprintf (fout, "| UOM. ");
	if (envSkStShowQty)
		fprintf (fout, "|  QTY ON HAND   ");
	fprintf (fout, "|QUANTITY COUNTED 1 | QUANTITY COUNTED 2 ");
	fprintf (fout, "| CHECKED  |\n");

	fprintf (fout, (BY_LOCN) ? "|------------" : "|------------------");
	fprintf (fout, (BY_LOCN) ? "|------------------" : "|------------");
	fprintf (fout, "|------------------------------------------");
	fprintf (fout, "|------");
	if (envSkStShowQty)
		fprintf (fout, "|----------------");
	fprintf (fout, "|-------------------|--------------------");
	fprintf (fout, "|----------|\n");

	fflush (fout);
}

void
Process (
 void)
{
	sprintf (err_str, "Printing Stock Take Sheets by %s", LCL_DESC);
	dsp_screen (err_str, comm_rec.co_no, comm_rec.co_name);

	Heading ();

	switch (printoutType)
	{
	case	GROUP:
		ProcessByGroup ();
		break;

	case	ITEM:
	case	ABC:
		ProcessByItem ();
		break;

	case	LOCN:
		if (envMultLoc)
			ProcessLocation ();
		else
			ProcessByStdLoc ();
		break;
	}
	fprintf (fout, ".EOF\n");
	fclose (fout);
}

void
ProcessByGroup (void)
{
	char	category [12];

	sprintf (category, "%-11.11s", " ");

	/*
	 * Allocate the initial array.
	 */
	ArrAlloc (&sortDetails, &sortRec, sizeof (struct SortStruct), 100);
	sortCnt = 0;

	strcpy (inmr_rec.co_no, comm_rec.co_no);
	sprintf (inmr_rec.inmr_class, "%-1.1s", lower);
	sprintf (inmr_rec.category, "%-11.11s", lower + 1);
	sprintf (inmr_rec.item_no, "%-16.16s", " ");
	cc = find_rec (inmr, &inmr_rec, GTEQ, "r");
	while (!cc && ValidGroup ())
	{
		if ((result = strstr (envSkIvalClass, inmr_rec.inmr_class)))
		{
			cc = find_rec (inmr, &inmr_rec, NEXT, "r");
			continue;
		}

		if (!SERIAL)
		{
			incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
			incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
			cc = find_rec (incc, &incc_rec, COMPARISON, "r");
			if (!cc && (MODE_OK || NON_FREEZE))
			{
				if (strcmp (category, inmr_rec.category))
				{
					if (strcmp (category, "           "))
						PrintSheet (category);

					/*
					 * Allocate the initial array.
					 */
					ArrAlloc (&sortDetails, &sortRec, sizeof (struct SortStruct), 100);
					sortCnt = 0;

					strcpy (category, inmr_rec.category);
				}
				SaveLine (0);
			}
		}
		cc = find_rec (inmr, &inmr_rec, NEXT, "r");
	}
	PrintSheet (category);
}

void
ProcessByItem (void)
{
	/*
	 * Allocate the initial array.
	 */
	ArrAlloc (&sortDetails, &sortRec, sizeof (struct SortStruct), 100);
	sortCnt = 0;

	strcpy (inmr_rec.co_no, comm_rec.co_no);
	sprintf (inmr_rec.item_no, "%-16.16s", (BY_ITEM) ? lower : " ");
	cc = find_rec (inmr, &inmr_rec, GTEQ, "r");
	while (!cc && !strcmp (inmr_rec.co_no, comm_rec.co_no) && ValidItem ())
	{
		if ((result = strstr (envSkIvalClass, inmr_rec.inmr_class)))
		{
			cc = find_rec (inmr, &inmr_rec, NEXT, "r");
			continue;
		}

		if (ValidAbc () && !SERIAL)
		{
			incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
			incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
			cc = find_rec (incc, &incc_rec, COMPARISON, "r");
			if (!cc && (MODE_OK || NON_FREEZE))
				SaveLine (0);
		}
		cc = find_rec (inmr, &inmr_rec, NEXT, "r");
	}
	PrintSheet ("");
}

void
ProcessByStdLoc (
 void)
{
	abc_selfield (incc, "incc_id_no_3");
	abc_selfield (inmr, "inmr_hhbr_hash");

	/*
	 * Allocate the initial array.
	 */
	ArrAlloc (&sortDetails, &sortRec, sizeof (struct SortStruct), 100);
	sortCnt = 0;

	incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	sprintf (incc_rec.location, "%-10.10s", lower);
	sprintf (incc_rec.sort, "%-28.28s", " ");
	cc = find_rec (incc, &incc_rec, GTEQ, "r");
	while (!cc && ValidLocation (incc_rec.location) && 
                      incc_rec.hhcc_hash == ccmr_rec.hhcc_hash)
	{
		if ((result = strstr (envSkIvalClass, inmr_rec.inmr_class)))
		{
			cc = find_rec (incc, &incc_rec, NEXT, "r");
			continue;
		}

		if (MODE_OK || NON_FREEZE)
		{
			inmr_rec.hhbr_hash	=	incc_rec.hhbr_hash;
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
			if (!SERIAL)
				SaveLine (1);

		}
		cc = find_rec (incc, &incc_rec, NEXT, "r");
	}
	PrintSheet ("");
}

void
ProcessLocation (void)
{
	FirstLoc = TRUE;

	/*
	 * Allocate the initial array.
	 */
	ArrAlloc (&sortDetails, &sortRec, sizeof (struct SortStruct), 100);
	sortCnt = 0;

	abc_selfield (inlo, "inlo_id_loc");
	abc_selfield (incc, "incc_hhwh_hash");
	abc_selfield (inmr, "inmr_hhbr_hash");

	sprintf (inlo2_rec.location,  lower);
	cc = find_rec (inlo2, &inlo2_rec, GTEQ, "r");

	while (!cc && (strncmp (inlo2_rec.location, upper, 10) <= 0))
	{
		incc_rec.hhwh_hash = inlo2_rec.hhwh_hash;
		cc = find_rec (incc, &incc_rec, COMPARISON, "r");
		if (cc || (!MODE_OK && !NON_FREEZE) || 	
			incc_rec.hhcc_hash != ccmr_rec.hhcc_hash)
		{
			cc = find_rec (inlo2, &inlo2_rec, NEXT, "r");
			continue;
		}
		inmr_rec.hhbr_hash	=	incc_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		if (cc || strcmp (inmr_rec.co_no, comm_rec.co_no) ||
			 (result = strstr (envSkIvalClass, inmr_rec.inmr_class)) || SERIAL)
		{
			cc = find_rec (inlo2, &inlo2_rec, NEXT, "r");
			continue;
		}
		SaveLocationLine ();
		cc = find_rec (inlo2, &inlo2_rec, NEXT, "r");
	}
	abc_selfield (inlo, "inlo_mst_loc");
	PrintLocationSheet ();
}

void
SaveLine (
	int		byLocation)
{
	char	LastLocation [11],
			ThisLocation [11];
	
	strcpy (ThisLocation, "          ");
	strcpy (LastLocation, "          ");

	if (envMultLoc)
	{
		inlo_rec.hhwh_hash 	= incc_rec.hhwh_hash;
		strcpy (inlo_rec.loc_type, " ");
		strcpy (inlo_rec.location, "          ");
		cc = find_rec (inlo, &inlo_rec, GTEQ, "r");
		while (!cc && inlo_rec.hhwh_hash == incc_rec.hhwh_hash)
		{
			if (BY_LOCN)
			{
				if (strcmp (inlo_rec.location, inlo2_rec.location))
				{
					cc = find_rec (inlo, &inlo_rec, NEXT, "r");
					continue;
				}
				strcpy (ThisLocation , inlo_rec.location);
				if (!strcmp (ThisLocation, LastLocation))
				{
					strcpy (LastLocation , inlo_rec.location);
					cc = find_rec (inlo, &inlo_rec, NEXT, "r");
					continue;
				}
				strcpy (LastLocation , inlo_rec.location);
			}
			else
			{
				strcpy (ThisLocation , inlo_rec.location);
				if (!strcmp (ThisLocation, LastLocation))
				{
					strcpy (LastLocation , inlo_rec.location);
					cc = find_rec (inlo, &inlo_rec, NEXT, "r");
					continue;
				}
				strcpy (LastLocation , inlo_rec.location);
			}
			/*
			 * Check the array size before adding new element.
			 */
			if (!ArrChkLimit (&sortDetails, sortRec, sortCnt))
				sys_err ("ArrChkLimit (sortRec)", ENOMEM, PNAME);

			/*
			 * Load values into array element sortCnt.
			 */
			strcpy (sortRec [sortCnt].locNo, inlo_rec.location);
			strcpy (sortRec [sortCnt].itemNo, inmr_rec.item_no);
			if (byLocation)
			{
				sprintf 
				(
					sortRec [sortCnt].sortCode, 
					"%-10.10s%-16.16s",
					inlo_rec.location,
					inmr_rec.item_no
				);
			}
			else
			{
				sprintf 
				(
					sortRec [sortCnt].sortCode, 
					"%-16.16s%-10.10s",
					inmr_rec.item_no,
					inlo_rec.location
				);
			}
			sortRec [sortCnt].hhbrHash	=	inmr_rec.hhbr_hash;
			if (envSkStExpUom)
				sortRec [sortCnt].hhwhHash	=	incc_rec.hhwh_hash;
			else
				sortRec [sortCnt].hhwhHash	=	0L;

			sortRec [sortCnt].qty	= 	inlo_rec.qty;
            
			/*
			 * Increment array counter.
			 */
			sortCnt++;

			cc = find_rec (inlo, &inlo_rec, NEXT, "r");
		}
	}
	else
	{
		if (!ValidLocation (incc_rec.location))
			return;

		/*
		 * Check the array size before adding new element.
		 */
		if (!ArrChkLimit (&sortDetails, sortRec, sortCnt))
			sys_err ("ArrChkLimit (sortRec)", ENOMEM, PNAME);

		strcpy (sortRec [sortCnt].locNo, inlo_rec.location);
		strcpy (sortRec [sortCnt].itemNo, inmr_rec.item_no);
		if (byLocation)
		{
			sprintf 
			(
				sortRec [sortCnt].sortCode, 
				"%-10.10s%-16.16s",
				inlo_rec.location,
				inmr_rec.item_no
			);
		}
		else
		{
			sprintf 
			(
				sortRec [sortCnt].sortCode, 
				"%-16.16s%-10.10s",
				inmr_rec.item_no,
				inlo_rec.location
			);
		}
		sortRec [sortCnt].hhbrHash	=	inmr_rec.hhbr_hash;
		if (envSkStExpUom)
			sortRec [sortCnt].hhwhHash	= incc_rec.hhwh_hash;
		else
			sortRec [sortCnt].hhwhHash	= 0L;
														
		sortRec [sortCnt].qty	=	incc_rec.closing_stock;

		/*
		 * Increment array counter.
		 */
		sortCnt++;
	}
	if (BY_LOCN)
		dsp_process ("Location : ", inlo2_rec.location);
	else
		dsp_process ("Item : ", inmr_rec.item_no);
}
void
SaveLocationLine (void)
{
	/*
	 * Check the array size before adding new element.
	 */
	if (!ArrChkLimit (&sortDetails, sortRec, sortCnt))
		sys_err ("ArrChkLimit (sortRec)", ENOMEM, PNAME);

	sprintf 
	(
		sortRec [sortCnt].sortCode, 
		"%-10.10s%-16.16s",
		inlo2_rec.location,
		inmr_rec.item_no
	);

	strcpy (sortRec [sortCnt].locNo, inlo2_rec.location);
	strcpy (sortRec [sortCnt].itemNo, inmr_rec.item_no);
	sortRec [sortCnt].hhbrHash	=	inmr_rec.hhbr_hash;
	sortRec [sortCnt].qty		=	inlo2_rec.qty;
	if (envSkStExpUom)
		sortRec [sortCnt].hhwhHash	=	incc_rec.hhwh_hash;
	else
		sortRec [sortCnt].hhwhHash	=	0L;

	/*
	 * Increment array counter.
	 */
	sortCnt++;

	dsp_process ("Location : ", inlo2_rec.location);
}

int
ValidGroup (void)
{
	if (strcmp (inmr_rec.co_no, comm_rec.co_no))
		return (EXIT_SUCCESS);

	if (inmr_rec.inmr_class [0] > upper [0])
		return (EXIT_SUCCESS);

	if (inmr_rec.inmr_class [0] < upper [0])
		return (EXIT_FAILURE);

	if (strcmp (inmr_rec.category, upper + 1) > 0)
		return (EXIT_SUCCESS);

	return (EXIT_FAILURE);
}

int
ValidItem (void)
{
	if (printoutType != ITEM)
		return (EXIT_FAILURE);

	if (strcmp (inmr_rec.item_no, upper) > 0)
		return (EXIT_SUCCESS);

	return (EXIT_FAILURE);
}

int
ValidAbc (void)
{
	if (printoutType != ABC)
		return (EXIT_FAILURE);

	if (strcmp (inmr_rec.abc_code, lower) < 0)
		return (EXIT_SUCCESS);

	if (strcmp (inmr_rec.abc_code, upper) > 0)
		return (EXIT_SUCCESS);

	return (EXIT_FAILURE);
}

int
ValidLocation (
	char	*location)
{
	if (printoutType != LOCN)
		return (EXIT_FAILURE);

	if (strcmp (location, lower) < 0)
		return (EXIT_SUCCESS);

	if (strcmp (location, upper) > 0)
		return (EXIT_SUCCESS);

	return (EXIT_FAILURE);
}

void
PrintSheet (
	char	*category)
{
	int		i;
	char	*cptr;
	static	int	categoryPrint;

	/*
	 * Sort the array in item description order.
	 */
	qsort (sortRec, sortCnt, sizeof (struct SortStruct), SortFunc);
	
	if (sortCnt && printoutType == GROUP)
	{
		strcpy (excf_rec.co_no, comm_rec.co_no);
		sprintf (excf_rec.cat_no, "%-11.11s", category);
		cc = find_rec (excf, &excf_rec, COMPARISON, "r");
		if (cc)
			strcpy (excf_rec.cat_desc, " ");

		cptr = expand (err_str, excf_rec.cat_desc);

		if (envSkStShowQty)
			fprintf (fout, ".PD| %11.11s %-137.137s|\n", category, cptr);
		else
			fprintf (fout, ".PD| %11.11s %-120.120s|\n", category, cptr);

		/*
		 * page break on category			
		 */
		if (categoryPrint)
		{
			reportLineCounter = 999L;
			CheckStakePage ();
		}
		categoryPrint = 1;
	}

	for (i = 0; i < sortCnt; i++)
	{
		inmr2_rec.hhbr_hash	=	sortRec [i].hhbrHash;
		cc = find_rec (inmr2, &inmr2_rec, COMPARISON, "r");
		if (!cc)
			PrintLine (i);
	}
	/*
	 *	Free up the array memory
	 */
	ArrDelete (&sortDetails);
}
void
PrintLocationSheet (void)
{
	int		i;
	long	hhbrHash	=	0L;
	char	thisWorkStr [31],
			lastWorkStr [31];

	strcpy (lastWorkStr, " ");
	strcpy (thisWorkStr, " ");

	/*
	 * Sort the array in item description order.
	 */
	qsort (sortRec, sortCnt, sizeof (struct SortStruct), SortFunc);
	
	for (i = 0; i < sortCnt; i++)
	{
		sprintf (thisWorkStr, "%-27.27s", sortRec [i].sortCode);
		hhbrHash = sortRec [i].hhbrHash;
		
		if (strcmp (thisWorkStr, lastWorkStr))
		{
			inmr2_rec.hhbr_hash	=	sortRec [i].hhbrHash;
			cc = find_rec (inmr2, &inmr2_rec, COMPARISON, "r");
			if (!cc)
				PrintLine (i);
		}
		strcpy (lastWorkStr, thisWorkStr);
	}
	/*
	 *	Free up the array memory
	 */
	ArrDelete (&sortDetails);
}
void
PrintLine (
	int		lineNo)
{
	int		i;
	long	hhwhHash		=	0L;
	int		ItemLocPrinted	=	TRUE;
	float	CnvFct			=	0.00;
	float	StdCnvFct		=	0.00;
	float	Closing			=	0.00;

	hhwhHash	=	sortRec [lineNo].hhwhHash;
	if (hhwhHash)
	{
		inum_rec.hhum_hash	=	inmr2_rec.std_uom;
		cc = find_rec (inum, &inum_rec, COMPARISON, "r");
		StdCnvFct = (float) ((cc) ? 1.00 : inum_rec.cnv_fct);
	}
	if (BY_LOCN)
	{
		if (FirstLoc)
		{
			strcpy (OldLoc, sortRec [lineNo].locNo);
			FirstLoc = FALSE;
		}

		strcpy (NewLoc, sortRec [lineNo].locNo);
		if (strcmp (OldLoc, NewLoc))
		{
			fprintf (fout, (BY_LOCN) ? "|------------" : "|------------------");
			fprintf (fout, (BY_LOCN) ? "|------------------" : "|------------");
			fprintf (fout, "|------------------------------------------");
			fprintf (fout, "|------");
			if (envSkStShowQty)
				fprintf (fout, "|----------------");
			fprintf (fout, "|-------------------|--------------------");
			fprintf (fout, "|----------|\n");
			CheckStakePage ();
		}
		strcpy (OldLoc, sortRec [lineNo].locNo);

		fprintf (fout, "| %-10.10s ", sortRec [lineNo].locNo);
		fprintf (fout, "| %-16.16s ", inmr2_rec.item_no);
	
	}
	else
	{
		fprintf (fout, "| %-16.16s ", inmr2_rec.item_no);
		fprintf (fout, "| %-10.10s ", sortRec [lineNo].locNo);
		strcpy (NewLoc, sortRec [lineNo].locNo);
	}

	if (strcmp (inmr2_rec.supercession, "                "))
	{
		fprintf (fout, "| SAME AS %16.16s  (%40.40s)                                      |          |\n", inmr2_rec.supercession, 
		        		   inmr2_rec.description);
		CheckStakePage ();
	}	
	else
	{
		if (hhwhHash)
		{
			inwu_rec.hhwh_hash	=	hhwhHash;
			inwu_rec.hhum_hash	=	0L;
			cc = find_rec (inwu, &inwu_rec, GTEQ, "r");
			if (cc || inwu_rec.hhwh_hash !=	hhwhHash)
			{
				fprintf (fout, "| %-40.40s ", inmr2_rec.description);
				fprintf (fout, "| %s ", inmr2_rec.sale_unit);
				if (envSkStShowQty)
				{
					sprintf 
					(
						err_str, 
						reportQtyMask, 
						n_dec (sortRec [lineNo].qty, inmr_rec.dec_pt)
					);
					fprintf (fout, "| %-14.14s ", err_str);
				}
				fprintf (fout, "|                   |                    ");
				fprintf (fout, "|%-10.10s|\n", " ");
				CheckStakePage ();
				fflush (fout);
			}
			if (envMultLoc)
			{
				while (!cc && inwu_rec.hhwh_hash == hhwhHash)
				{
					abc_selfield (inlo, "inlo_mst_loc");
					inlo_rec.hhwh_hash	=	hhwhHash;
					inlo_rec.hhum_hash	=	inwu_rec.hhum_hash;
					strcpy (inlo_rec.location, NewLoc);
					cc = find_rec (inlo, &inlo_rec, GTEQ, "r");
					if (!cc && inlo_rec.hhwh_hash == hhwhHash &&
						   		inlo_rec.hhum_hash == inwu_rec.hhum_hash &&
								!strcmp (inlo_rec.location, NewLoc))
					{
						Closing	=	inlo_rec.qty / inlo_rec.cnv_fct;
						if (ItemLocPrinted	== FALSE)
						{
							if (BY_LOCN)
							{
								fprintf (fout, "| %10.10s ", " ");
								fprintf (fout, "| %16.16s ", " ");
							}
							else
							{
								fprintf (fout, "| %16.16s ", " ");
								fprintf (fout, "| %10.10s ", " ");
							}
							fprintf (fout, "| %-40.40s ", " ");
						}
						else
							fprintf (fout, "| %-40.40s ",inmr2_rec.description);

						fprintf (fout, "| %s ", inlo_rec.uom);
						if (envSkStShowQty)
							fprintf (fout, "| %14.2f ", Closing);
						fprintf (fout, "|                   |                    ");
						fprintf (fout, "|%-10.10s|\n", " ");
						CheckStakePage ();
						fflush (fout);
						ItemLocPrinted	= FALSE;
					}
					cc = find_rec (inwu, &inwu_rec, NEXT, "r");
				}
			}
			else
			{
				while (!cc && inwu_rec.hhwh_hash == hhwhHash)
				{
					inum_rec.hhum_hash	=	inwu_rec.hhum_hash;
					cc = find_rec (inum, &inum_rec, COMPARISON, "r");
					if (cc)
					{
						cc = find_rec (inwu, &inwu_rec, NEXT, "r");
						continue;
					}

					CnvFct	=	inum_rec.cnv_fct / StdCnvFct;
					Closing	=	inwu_rec.closing_stock / CnvFct;
					if (ItemLocPrinted	== FALSE)
					{
						if (BY_LOCN)
						{
							fprintf (fout, "| %10.10s ", " ");
							fprintf (fout, "| %16.16s ", " ");
						}
						else
						{
							fprintf (fout, "| %16.16s ", " ");
							fprintf (fout, "| %10.10s ", " ");
						}
						fprintf (fout, "| %-40.40s ", " ");
					}
					else
						fprintf (fout, "| %-40.40s ", inmr2_rec.description);

					fprintf (fout, "| %s ", inum_rec.uom);
					if (envSkStShowQty)
						fprintf (fout, "| %14.2f ", Closing);
					fprintf (fout, "|                   |                    ");
					fprintf (fout, "|%-10.10s|\n", " ");
					CheckStakePage ();
					fflush (fout);
					ItemLocPrinted	= FALSE;
					cc = find_rec (inwu, &inwu_rec, NEXT, "r");
				}
			}
		}
		else
		{
			fprintf (fout, "| %-40.40s ", inmr2_rec.description);
			fprintf (fout, "| %s ", inmr2_rec.sale_unit);
			if (envSkStShowQty)
			{
				sprintf 
				(
					err_str, 
					reportQtyMask, 
					n_dec (sortRec [lineNo].qty, inmr_rec.dec_pt)
				);
				fprintf (fout, "| %-14.14s ", err_str);
			}
			fprintf (fout, "|                   |                    ");
			fprintf (fout, "|%-10.10s|\n", " ");
			CheckStakePage ();
			fflush (fout);
		}
	}
	if (envSkStDescShow)
		PrintInex (); 

	for (i = 0;i < envSkStSpace;i++)
	{
		if (inexPrinted)
			i = envSkStSpace;
		
		if (envSkStShowQty)
			fprintf (fout, "|%-150.150s|\n", " ");
		else
			fprintf (fout, "|%-133.133s|\n", " ");
		CheckStakePage ();
	}
	fflush (fout);
}

void
CheckStakePage (void)
{
	if (++reportLineCounter >= envSkStPageLen)
	{
		reportPageCounter++;	
		fprintf (fout, ".PA\n");
		reportLineCounter = 0L;
	}
}

void
PrintInex (void)
{
	inex_rec.hhbr_hash = inmr2_rec.hhbr_hash;
	inex_rec.line_no   = 0;
	inexPrinted = FALSE;

	cc = find_rec (inex, &inex_rec, GTEQ, "r");
	if (cc || inex_rec.hhbr_hash != inmr2_rec.hhbr_hash)
		return;

	while (!cc && inex_rec.hhbr_hash == inmr2_rec.hhbr_hash)
	{
		fprintf (fout, "| %-27.27s   ", " ");
		fprintf (fout, "| %-40.40s ", inex_rec.desc);
		fprintf (fout, "|      ");
		if (envSkStShowQty)
			fprintf (fout, "|                ");
		fprintf (fout, "|                   |                    ");
		fprintf (fout, "|          |\n");
		CheckStakePage ();
		cc = find_rec (inex, &inex_rec, NEXT, "r");
	}
	inexPrinted = TRUE;
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

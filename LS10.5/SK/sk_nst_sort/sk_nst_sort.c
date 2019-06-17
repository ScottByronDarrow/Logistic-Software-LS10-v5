/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
|  Program Name  : (sk_nst_sort.c)                                    |
|  Program Desc  : (Creat Stock Take Work File.          	     )    |	
| $Id: sk_nst_sort.c,v 5.3 2002/11/26 03:24:16 keytan Exp $
|---------------------------------------------------------------------|
|  Author        : Roger Gibbison  | Date Written  : 30/03/89         |
|---------------------------------------------------------------------|
|  Date Modified : (30/03/89)      | Modified  by  : Roger Gibbison.  |
|  Date Modified : (08/05/89)      | Modified  by  : Roger Gibbison.  |
|  Date Modified : (16/06/92)      | Modified  by  : Simon Dubey.     |
|  Date Modified : (30/09/97)      | Modified  by  : Elizabeth D. Paid|
|                :                                                    |
|                                                                     |
|  Comments      : Handle Function keys in getalpha                   |
|    (16/06/92)  : exclude values of SK_IVAL_CLASS from stock take    |
|                : SC DFH 7096                                        |
|                :                                                    |
|    (30/09/97)  : SEL     - Multilingual Conversio, changed printf to|
|                :           print_at                                 |
|                :                                                    |
|                                                                     |
| $Log: sk_nst_sort.c,v $
| Revision 5.3  2002/11/26 03:24:16  keytan
| Fixed ProcessByItem. Place teh processing of incc after find_rec incc.
|
| Revision 5.2  2002/07/16 02:43:46  scott
| Updated from service calls and general maintenance.
|
| Revision 5.1  2002/03/08 04:01:07  scott
| Updated to use memory based sorting.
|
| Revision 5.2  2001/08/09 09:19:29  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:45:30  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:16:57  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:38:17  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.2  2000/12/19 04:20:58  scott
| Updated to use index in open that is used in program
|
| Revision 3.1  2000/11/20 07:40:21  scott
| New features related to 3PL environment
| New features related to Number Plates
| All covered in release 3 notes
|
| Revision 3.0  2000/10/10 12:20:50  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.1  2000/08/01 02:50:01  scott
| General Maintenance - Added app.schema
|
| Revision 2.0  2000/07/15 09:11:32  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.13  2000/03/15 06:11:45  ramon
| Removed the call to fflush () during sort.
|
| Revision 1.12  1999/11/23 04:35:15  scott
| Updated to return correct status.
|
| Revision 1.11  1999/11/03 07:32:19  scott
| Updated to fix small compile errors due to -Wall flag being set.
|
| Revision 1.10  1999/10/13 02:42:08  nz
| Updated to ensure read_comm was in correct place.
|
| Revision 1.9  1999/10/08 05:32:44  scott
| First Pass checkin by Scott.
|
| Revision 1.8  1999/06/20 05:20:26  scott
| Updated to add log for cvs + remove old read_comm + fixed all warnings.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_nst_sort.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_nst_sort/sk_nst_sort.c,v 5.3 2002/11/26 03:24:16 keytan Exp $";

#define	NO_SCRGEN
#include	<pslscr.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<ml_sk_mess.h>
#include 	<arralloc.h>

#define	GROUP		0
#define	ITEM		1
#define	ABC			2
#define	LOCN		3
#define	BOTH		4	/* Location/Item number */

#define	LCL_DESC	itemTypes [sortType]._desc
#define	LCL_INMR	itemTypes [sortType]._inmr

#define	BY_ITEM		 (sortType == ITEM)
#define	BY_MLOC		 (sortType == LOCN && envMultLoc)
#define	BY_BOTH		 (sortType == BOTH)

#define	SERIAL		 (inmr_rec.serial_item [0] == 'Y')
#define	MODE_OK		 (incc_rec.stat_flag [0] == mode [0])
#define	NON_FREEZE	 (incc_rec.stat_flag [0] == '0' && envSkStPfrz)

#include	"schema"

struct commRecord	comm_rec;
struct ccmrRecord	ccmr_rec;
struct inmrRecord	inmr_rec;
struct inccRecord	incc_rec;
struct inloRecord	inlo_rec;
struct inloRecord	inlo2_rec;

	struct	{
		char	*_desc;
		char	*_inmr;
	} itemTypes [] = {
		{"Group", 		"inmr_id_no_3"},
		{"Item Number", "inmr_id_no"},
		{"ABC Code", 	"inmr_id_no"},
		{"Location", 	"inmr_id_no"},
 		{"Location/Item Number", "inmr_id_no"},
	};

	struct	{
		long	hhwhHash;
		char	location [11];
	} wkRec;

/*
 *	Structure for dynamic array.
 */
struct SortStruct
{
	char	sortCode [27];
	char	itemNo	[sizeof inmr_rec.item_no]; 
	char	locNo	[sizeof incc_rec.location]; 
	long	hhwhHash; 
}	*sortRec;
	DArray sortDetails;
	int	sortCnt = 0;

int		SortFunc			(const	void *,	const void *);

	int		printerNumber,
			envSkStPfrz = 0,
			sortType,
			envMultLoc 		= FALSE, 
			workFileNumber,
			processID;

	char	*envSkIvalClass,
			*result;

	char 	*data 	= "data",
			*inlo2	= "inlo2",
			lower [17],
			upper [17],
			mode [2];

/*
 * Function Declarations 
 */
int  	CheckForStock 		(long);
int  	ValicAbc 			(void);
int  	ValidItem 			(void);
void 	CloseDB 			(void);
void 	OpenDB 				(void);
void 	Process 			(void);
void 	ProcessBothStdLocn 	(void);
void 	ProcessByGroup 		(void);
void 	ProcessByItem 		(void);
void 	ProcessByStdLoc 	(void);
void 	ProcessLocation 	(void);
void 	ReadMisc 			(void);
void 	SaveLine 			(void);
void 	CheckValidLocs 		(long);
void 	shutdown_prog 		(void);

/*
 * Main Processing Routine 
 */
int
main (
 int argc, 
 char * argv [])
{
	char	*sptr;

	if (argc != 7)
	{
		print_at (0,0, ML (mlSkMess215),argv [0]);
		return (EXIT_FAILURE);
	}

	printerNumber	= atoi (argv [1]);
	sprintf (mode,"%-1.1s",argv [5]);
	processID		= atoi (argv [6]);

	sptr = chk_env ("SK_ST_PFRZ");
	if (sptr != (char *)0)
		envSkStPfrz = atoi (sptr);

	sptr = chk_env ("MULT_LOC");
	if (sptr != (char *)0)
		envMultLoc = atoi (sptr);


	switch (argv [4] [0])
	{
	case	'G':
	case	'g':
		sprintf (lower,"%-12.12s",argv [2]);
		sprintf (upper,"%-12.12s",argv [3]);
		sortType = GROUP;
		break;

	case	'I':
	case	'i':
		sprintf (lower,"%-16.16s",argv [2]);
		sprintf (upper,"%-16.16s",argv [3]);
		sortType = ITEM;
		break;

	case	'A':
	case	'a':
		sprintf (lower,"%-1.1s",argv [2]);
		sprintf (upper,"%-1.1s",argv [3]);
		sortType = ABC;
		break;

	case	'L':
	case	'l':
		sprintf (lower,"%-10.10s",argv [2]);
		sprintf (upper,"%-10.10s",argv [3]);
		sortType = LOCN;
		break;

 	case	'B':
 	case	'b':
 		sprintf (wkRec.location,"B%-9.9s",mode);
 		sprintf (lower,"%-10.10s",argv [2]);
 		sprintf (upper,"%-10.10s",argv [3]);
 		sortType = BOTH;
 		break;
 
	default:
		print_at (0,0, mlSkMess215, argv [0]);
		return (EXIT_FAILURE);
	}

	OpenDB ();

	switch (argv [4] [0])
	{
	case	'G':
	case	'g':
		sprintf (wkRec.location,"G%-9.9s",mode);
		break;

	case	'I':
	case	'i':
		sprintf (wkRec.location,"I%-9.9s",mode);
		break;

	case	'A':
	case	'a':
		sprintf (wkRec.location,"A%-9.9s",mode);
		break;

	case	'L':
	case	'l':
		if (envMultLoc)
			sprintf (wkRec.location,"L%-9.9s",mode);
		else
			sprintf (wkRec.location,"I%-9.9s",mode);

		break;

 	case	'B':
 	case	'b':
 		sprintf (wkRec.location,"B%-9.9s",mode);
 		break;
 
	default:
		print_at (0,0, mlSkMess215, argv [0]);
		return (EXIT_FAILURE);
	}

	sptr = chk_env ("SK_IVAL_CLASS");
	if (sptr)
		envSkIvalClass = strdup (sptr);
	else
		envSkIvalClass = strdup ("ZKPN");
	upshift (envSkIvalClass); 


	wkRec.hhwhHash = 0L;

	cc = RF_ADD (workFileNumber, (char *) &wkRec);
	if (cc)
		file_err (cc, "work", "WKADD");

	Process ();

	shutdown_prog ();
    return (EXIT_SUCCESS);
}

void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*
 * Open database files. 
 */
void
OpenDB (void)
{
	char	*sptr = getenv ("PROG_PATH");
	char	filename [101];

	abc_dbopen (data);

	ReadMisc ();

	abc_alias (inlo2, inlo);

	open_rec (inmr, inmr_list, INMR_NO_FIELDS, LCL_INMR);
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (inlo, inlo_list, INLO_NO_FIELDS, "inlo_id_loc");
	open_rec (inlo2,inlo_list, INLO_NO_FIELDS, "inlo_location");
	
	sprintf (filename,"%s/WORK/sksort%05d", (sptr == (char *) 0) 
							? "/usr/LS10.5" : sptr,processID);
	cc = RF_OPEN (filename,sizeof (wkRec),"w",&workFileNumber);
	if (cc)
		file_err (cc, "work", "WKOPEN");
}

void
CloseDB (void)
{
	abc_fclose (inmr);
	abc_fclose (incc);
	abc_fclose (inlo);
	abc_fclose (inlo2);
	
	abc_dbclose (data);
	RF_CLOSE (workFileNumber);
}

void
ReadMisc (void)
{
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (ccmr,ccmr_list,CCMR_NO_FIELDS,"ccmr_id_no");

	strcpy (ccmr_rec.co_no,comm_rec.co_no);
	strcpy (ccmr_rec.est_no,comm_rec.est_no);
	strcpy (ccmr_rec.cc_no,comm_rec.cc_no);
	cc = find_rec (ccmr,&ccmr_rec,COMPARISON,"r");
	if (cc)
		file_err (cc, "ccmr", "DBFIND");

	abc_fclose (ccmr);
}

void
Process (void)
{
	sprintf (err_str,"Creating Stock Take File by %s",LCL_DESC);
	dsp_screen (err_str,comm_rec.co_no,comm_rec.co_name);

	switch (sortType)
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

 	case	BOTH:
		if (envMultLoc)
			ProcessLocation ();
		else
			ProcessBothStdLocn ();
		break;

	}
}

/*
 * Process stock by product group.
 */
void
ProcessByGroup (void)
{
	abc_selfield (incc, "incc_id_no_2");
	abc_selfield (inmr, "inmr_hhbr_hash");

	incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	sprintf (incc_rec.sort,"%-12.12s%-16.16s",lower, " ");
	cc = find_rec (incc,&incc_rec,GTEQ,"r");
	while (!cc && incc_rec.hhcc_hash == ccmr_rec.hhcc_hash)
	{
		if (strncmp (incc_rec.sort, upper,12) > 0)
			break;

		inmr_rec.hhbr_hash	=	incc_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		if ((result = strstr (envSkIvalClass, inmr_rec.inmr_class)))
		{
			cc = find_rec (incc, &incc_rec, NEXT, "r");
			continue;
		}
		/*
		 * Don't allow superceeded parts in sort file.
		 */
		if (strcmp (inmr_rec.supercession, "                "))
		{
			cc = find_rec (incc,&incc_rec,NEXT,"r");
			continue;
		}

		if (CheckForStock (incc_rec.hhwh_hash))
		{
			cc = find_rec (incc,&incc_rec,NEXT,"r");
			continue;
		}
		if (!cc && !SERIAL && (MODE_OK || NON_FREEZE))
		{
			wkRec.hhwhHash = incc_rec.hhwh_hash; 
			strcpy (wkRec.location,"          ");

			cc = RF_ADD (workFileNumber, (char *) &wkRec);
			if (cc)
				file_err (cc, "work", "DBADD");

			dsp_process ("Item : ",inmr_rec.item_no);
		}
		cc = find_rec (incc,&incc_rec,NEXT,"r");
	}
}

/*
 * Process items by Items. 
 */
void
ProcessByItem (void)
{
	strcpy (inmr_rec.co_no,comm_rec.co_no);
	sprintf (inmr_rec.item_no,"%-16.16s", (BY_ITEM) ? lower : " ");
	cc = find_rec (inmr,&inmr_rec,GTEQ,"r");
	while (!cc && !strcmp (inmr_rec.co_no,comm_rec.co_no) && ValidItem ())
	{
		/*
		 * Don't allow superceeded parts in sort file. 
		 */
		if (strcmp (inmr_rec.supercession, "                "))
		{
			cc = find_rec (inmr,&inmr_rec,NEXT,"r");
			continue;
		}

		if ((result = strstr (envSkIvalClass, inmr_rec.inmr_class)))
		{
			cc = find_rec (inmr,&inmr_rec,NEXT,"r");
			continue;
		}

		if (ValicAbc () && !SERIAL)
		{
			incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
			incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
			cc = find_rec (incc,&incc_rec,COMPARISON,"r");
			if (!cc && (MODE_OK || NON_FREEZE))
			{
				if (CheckForStock (incc_rec.hhwh_hash))
				{
					cc = find_rec (incc,&incc_rec,NEXT,"r");
					continue;
				}
				wkRec.hhwhHash = incc_rec.hhwh_hash; 
				strcpy (wkRec.location,"          ");

				cc = RF_ADD (workFileNumber, (char *) &wkRec);
				if (cc)
					file_err (cc, "work", "DBADD");

				dsp_process ("Item : ",inmr_rec.item_no);
			}
		}
		cc = find_rec (inmr,&inmr_rec,NEXT,"r");
	}
}
/*
 * Process items by Standard Locations. 
 */
void
ProcessByStdLoc (
 void)
{
	abc_selfield (incc, "incc_id_no_3");
	abc_selfield (inmr, "inmr_hhbr_hash");

	incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	sprintf (incc_rec.location, "%-10.10s", lower);
	sprintf (incc_rec.sort, "%-28.28s", " ");
	cc = find_rec (incc,&incc_rec,GTEQ,"r");
	while (!cc && incc_rec.hhcc_hash == ccmr_rec.hhcc_hash)
	{
		if (strcmp (incc_rec.location, upper) > 0)
			break;

		inmr_rec.hhbr_hash	=	incc_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");

		/*
		 * Don't allow superceeded parts in sort file. 
		 */
		if (strcmp (inmr_rec.supercession, "                "))
		{
			cc = find_rec (incc,&incc_rec,NEXT,"r");
			continue;
		}
		if ((result = strstr (envSkIvalClass, inmr_rec.inmr_class)))
		{
			cc = find_rec (incc,&incc_rec,NEXT,"r");
			continue;
		}
		if (CheckForStock (incc_rec.hhwh_hash))
		{
			cc = find_rec (incc,&incc_rec,NEXT,"r");
			continue;
		}
		if (!SERIAL && (MODE_OK || NON_FREEZE))
		{
			wkRec.hhwhHash = incc_rec.hhwh_hash; 
			strcpy (wkRec.location,"          ");

			cc = RF_ADD (workFileNumber, (char *) &wkRec);
			if (cc)
				file_err (cc, "work", "DBADD");

			dsp_process ("Item : ",inmr_rec.item_no);
		}
		cc = find_rec (incc,&incc_rec,NEXT,"r");
	}
}

/*
 * Process items by Standard Locations/Items. 
 */
void
ProcessBothStdLocn (void)
{
	abc_selfield (incc, "incc_id_no_3");
	abc_selfield (inmr, "inmr_hhbr_hash");

	/*
	 * Allocate the initial array.
	 */
	ArrAlloc (&sortDetails, &sortRec, sizeof (struct SortStruct), 10);
	sortCnt = 0;

	incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	sprintf (incc_rec.location, "%-10.10s", lower);
	sprintf (incc_rec.sort, "%-28.28s", " ");
	cc = find_rec (incc,&incc_rec,GTEQ,"r");
	while (!cc && incc_rec.hhcc_hash == ccmr_rec.hhcc_hash)
	{
		if (strcmp (incc_rec.location, upper) > 0)
			break;

		inmr_rec.hhbr_hash	=	incc_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");

		/*
		 * Don't allow superceeded parts in sort file. 
		 */
		if (strcmp (inmr_rec.supercession, "                "))
		{
			cc = find_rec (incc,&incc_rec,NEXT,"r");
			continue;
		}

		if ((result = strstr (envSkIvalClass, inmr_rec.inmr_class)))
		{
			cc = find_rec (incc,&incc_rec,NEXT,"r");
			continue;
		}

		if (CheckForStock (incc_rec.hhwh_hash))
		{
			cc = find_rec (incc,&incc_rec,NEXT,"r");
			continue;
		}
		if (!SERIAL && (MODE_OK || NON_FREEZE))
		{
			/*
			 * Check the array size before adding new element.
			 */
			if (!ArrChkLimit (&sortDetails, sortRec, sortCnt))
				sys_err ("ArrChkLimit (sortRec)", ENOMEM, PNAME);

			/*
			 * Load values into array element sortCnt.
			 */
			sprintf 
			(
				sortRec [sortCnt].sortCode, 
				"%-10.10s%-16.16s",
				incc_rec.location, 
				inmr_rec.item_no
			);
			strcpy (sortRec [sortCnt].itemNo,	inmr_rec.item_no);
			strcpy (sortRec [sortCnt].locNo, 	incc_rec.location);
			sortRec [sortCnt].hhwhHash = incc_rec.hhwh_hash;
			/*
			 * Increment array counter.
			 */
			sortCnt++;

			dsp_process ("Item : ", inmr_rec.item_no);
		}
		cc = find_rec (incc,&incc_rec,NEXT,"r");
	}
	SaveLine ();
}

/*
 * Process items by Multiple Locations. 
 */
void
ProcessLocation (void)
{
	/*
	 * Allocate the initial array.
	 */
	ArrAlloc (&sortDetails, &sortRec, sizeof (struct SortStruct), 10);
	sortCnt = 0;

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
		/*
		 * Don't allow superceeded parts in sort file. 
		 */
		if (strcmp (inmr_rec.supercession, "                "))
		{
			cc = find_rec (inlo2, &inlo2_rec, NEXT, "r");
			continue;
		}
		CheckValidLocs (incc_rec.hhwh_hash);
		cc = find_rec (inlo2, &inlo2_rec, NEXT, "r");
	}
	SaveLine ();
}

/*
 * Validate items 
 */
int
ValidItem (void)
{
	if (sortType != ITEM)
		return (EXIT_FAILURE);

	if (strcmp (inmr_rec.item_no,upper) > 0)
		return (EXIT_SUCCESS);

	return (EXIT_FAILURE);
}

/*
 * Validate ABC Codes. 
 */
int
ValicAbc (void)
{
	if (sortType != ABC)
		return (EXIT_FAILURE);

	if (strcmp (inmr_rec.abc_code,lower) < 0)
		return (EXIT_SUCCESS);

	if (strcmp (inmr_rec.abc_code,upper) > 0)
		return (EXIT_SUCCESS);

	return (EXIT_FAILURE);
}

/*
 * Validate and process locations. 
 */
void
CheckValidLocs (
	long	hhwhHash)
{
	int		firstTime = 1;

	abc_selfield (inlo, "inlo_id_loc");

	inlo_rec.hhwh_hash 		= hhwhHash;
	inlo_rec.loc_type [0]	=	' ';
	strcpy (inlo_rec.location,	"          ");
	cc = find_rec (inlo,&inlo_rec,GTEQ,"r");
	while (!cc && inlo_rec.hhwh_hash == hhwhHash)
	{
		if (strcmp (inlo_rec.location, inlo2_rec.location))
		{
			cc = find_rec (inlo, &inlo_rec, NEXT, "r");
			continue;
		}
		/*
		 * Check the array size before adding new element.
		 */
		if (!ArrChkLimit (&sortDetails, sortRec, sortCnt))
			sys_err ("ArrChkLimit (sortRec)", ENOMEM, PNAME);

		/*
		 * Load values into array element sortCnt.
		 */
		sprintf 
		(
			sortRec [sortCnt].sortCode, 
			"%-10.10s%-16.16s",
			inlo_rec.location, 
			inmr_rec.item_no
		);
		strcpy (sortRec [sortCnt].itemNo,	inmr_rec.item_no);
		strcpy (sortRec [sortCnt].locNo, 	inlo_rec.location);
		sortRec [sortCnt].hhwhHash = incc_rec.hhwh_hash;
		/*
		 * Increment array counter.
		 */
		sortCnt++;

		if (firstTime)
			dsp_process ("Item : ",inmr_rec.item_no);

		firstTime = 0;
		cc = find_rec (inlo,&inlo_rec,NEXT,"r");
	}
}

/*
 * Use sorted work file is by multiple locations. 
 */
void
SaveLine (void)
{
	char	LastLoc [11];
	long	LastHHWH;
	int		i;

	LastHHWH	=	0L;
	strcpy (LastLoc, "         ");
	/*
	 * Sort the array in item description order.
	 */
	qsort (sortRec, sortCnt, sizeof (struct SortStruct), SortFunc);

	for (i = 0; i < sortCnt; i++)
	{
		wkRec.hhwhHash	=	sortRec [i].hhwhHash;
		strcpy (wkRec.location, sortRec [i].locNo);
		
		if (strcmp (LastLoc, wkRec.location) || LastHHWH != wkRec.hhwhHash)
		{
			cc = RF_ADD (workFileNumber, (char *) &wkRec);
			if (cc)
				file_err (cc, "work", "WKADD");
		}
		LastHHWH	=	wkRec.hhwhHash;
		strcpy (LastLoc, wkRec.location);
	}
	/*
	 *	Free up the array memory
	 */
	ArrDelete (&sortDetails);
}

int
CheckForStock (
	long	hhwhHash)
{
	if (!envMultLoc)
		return (EXIT_SUCCESS);

	abc_selfield (inlo, "inlo_hhwh_hash");

	inlo_rec.hhwh_hash	=	hhwhHash;
	return (find_rec (inlo, &inlo_rec, COMPARISON, "r"));
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

/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: bm_undefprt.c,v 5.2 2002/07/17 09:56:56 scott Exp $
|  Program Name  : (bm_undefprt.c)                                 
|  Program Desc  : (Print Items Which Should Have A Bill But Haven't)
|---------------------------------------------------------------------|
|  Author        : Campbell Mander.| Date Written : 10/09/91          |
|---------------------------------------------------------------------|
| $Log: bm_undefprt.c,v $
| Revision 5.2  2002/07/17 09:56:56  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.1  2002/06/03 08:22:11  scott
| Updated to remove sort functions
| Updated to convert to app.schema
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: bm_undefprt.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/BM/bm_undefprt/bm_undefprt.c,v 5.2 2002/07/17 09:56:56 scott Exp $";

#include <pslscr.h>
#include <dsp_screen.h>
#include <dsp_process2.h>
#include <get_lpno.h>
#include <ml_bm_mess.h>
#include <ml_std_mess.h>
#include <DateToString.h>
#include <arralloc.h>

#define	BY_ITEM		 (local_rec.rangeType [0] == 'I')

FILE	*fout;

#include	"schema"

struct bmmsRecord	bmms_rec;
struct commRecord	comm_rec;
struct excfRecord	excf_rec;
struct inmrRecord	inmr_rec;
struct inumRecord	inum_rec;

	char	*data	= "data";

	char	run_type [2];

	char	currentGroup [13];
	char	previousGroup [13];

/*
 *	Structure for dynamic array.
 */
struct SortStruct
{
	char	sortCode [31];
	char	itemClass		 [sizeof inmr_rec.inmr_class]; 
	char	itemCategory	 [sizeof inmr_rec.category]; 
	char	itemSource		 [sizeof inmr_rec.source]; 
	char	itemNo			 [sizeof inmr_rec.item_no]; 
	char	itemDescription	 [sizeof inmr_rec.description]; 
	char	itemUom			 [sizeof inum_rec.uom]; 
	char	uomDescription	 [sizeof inum_rec.desc]; 
}	*sortRec;
	DArray sortDetails;
	int	sortCnt = 0;

int		SortFunc			 (const	void *,	const void *);

/*
 * Local & Screen Structures. 
 */
struct {
	long	lsystemDate;
	int		printerNo;
	char	dummy 			[11];
	char	systemDate 		[11];
	char	rangeType 		[2];
	char	startItem 		[17];
	char	startItemDesc 	[41];
	char	endItem 		[17];
	char	endItemDesc 	[41];
	char	startClass 		[2];
	char	startCat 		[12];
	char	startCatDesc 	[41];
	char	endClass 		[2];
	char	endCat 			[12];
	char	endCatDesc 		[41];
} local_rec;

extern	int		TruePosition;

static	struct	var	vars [] =
{
	{1, LIN, "printerNo",		 4, 2, INTTYPE,
		"NN", "          ",
		" ", "1", "Printer Number   ", " ",
		YES, NO,  JUSTRIGHT, "", "", (char *)&local_rec.printerNo},

	{1, LIN, "startItem",	 6, 2, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "",  "Start Item       ", " ",
		NO, NO,  JUSTLEFT, "", "", local_rec.startItem},
	{1, LIN, "startItemDesc",	 6, 36, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.startItemDesc},
	{1, LIN, "endItem",	 7, 2, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "",  "End Item         ", " ",
		NO, NO,  JUSTLEFT, "", "", local_rec.endItem},
	{1, LIN, "endItemDesc", 7, 36, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.endItemDesc},
	{1, LIN, "startClass",	 6, 2, CHARTYPE,
		"U", "          ",
		" ", "A", "Start Class      ", " ",
		ND, NO,  JUSTLEFT, "", "", local_rec.startClass},
	{1, LIN, "startCat",	 7, 2, CHARTYPE,
		"UUUUUUUUUUU", "          ",
		" ", "",  "Start Category   ", " ",
		ND, NO,  JUSTLEFT, "", "", local_rec.startCat},
	{1, LIN, "startCatDesc",	 7, 36, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		ND, NO,  JUSTLEFT, "", "", local_rec.startCatDesc},
	{1, LIN, "endClass",	 9, 2, CHARTYPE,
		"U", "          ",
		" ", "Z", "End Class        ", " ",
		ND, NO,  JUSTLEFT, "", "", local_rec.endClass},
	{1, LIN, "endCat",	10, 2, CHARTYPE,
		"UUUUUUUUUUU", "          ",
		" ", "",  "End Category     ", " ",
		ND, NO,  JUSTLEFT, "", "", local_rec.endCat},
	{1, LIN, "endCatDesc",	10, 36, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		ND, NO,  JUSTLEFT, "", "", local_rec.endCatDesc},
	{0, LIN, "",		 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*
 * Local Function Prototypes
 */
void 	shutdown_prog 	(void);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
int 	spec_valid 		(int);
void 	Process 		(void);
void 	ProcessSorted 	(void);
int 	CategoryHeading (void);
void 	HeadingOutput 	(void);
void 	SrchExcf 		(char *);
int 	heading 		(int);

/*
 * Main Processing Routine
 */
int
main (
	int		argc,
	char	*argv [])
{
	TruePosition	=	TRUE;

	if (argc != 2)
	{
		print_at (0,0, ML (mlBmMess700), argv [0]);
        return (EXIT_FAILURE);
	}

	sprintf (local_rec.rangeType, "%-1.1s", argv [1]);

	local_rec.lsystemDate = TodaysDate ();
	strcpy (local_rec.systemDate, DateToDDMMYY (local_rec.lsystemDate));

	SETUP_SCR (vars);

	if (!BY_ITEM)
	{
		FLD ("startItem") 		= ND;
		FLD ("startItemDesc") 	= ND;
		FLD ("endItem") 		= ND;
		FLD ("endItemDesc") 	= ND;
		FLD ("startClass") 		= YES;
		FLD ("endClass") 		= YES;
		FLD ("startCat") 		= NO;
		FLD ("startCatDesc") 	= NA;
		FLD ("endCat") 			= NO;
		FLD ("endCatDesc") 		= NA;
	}

	init_scr ();
	set_tty ();
	set_masks ();

	OpenDB ();

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	while (prog_exit == 0)
	{
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;
		init_vars (1);

		heading (1);
		entry (1);
		if (restart || prog_exit)
			continue;

		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
			continue;

		Process ();
		prog_exit = TRUE;
	}
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
 * Open data base files 
 */
void
OpenDB (void)
{
	abc_dbopen (data);

	open_rec (bmms,bmms_list,BMMS_NO_FIELDS, "bmms_hhbr_hash");
	open_rec (excf,excf_list,EXCF_NO_FIELDS, "excf_id_no");
	open_rec (inmr,inmr_list,INMR_NO_FIELDS, 
							(BY_ITEM) ? "inmr_id_no" : "inmr_id_no_3");
	open_rec (inum,inum_list,INUM_NO_FIELDS, "inum_hhum_hash");
}

/*
 * Close data base files 
 */
void
CloseDB (void)
{
	abc_fclose (bmms);
	abc_fclose (inmr);
	abc_fclose (inum);
	abc_fclose (excf);
	SearchFindClose ();
	abc_dbclose (data);
}

int
spec_valid (
	int		field)
{
	if (LCHECK ("printerNo"))
	{
		if (SRCH_KEY)
		{
			local_rec.printerNo = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("startItem"))
	{
		if (FLD ("startItem") == ND)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			sprintf (local_rec.startItem, "%-16.16s", " ");
			strcpy (local_rec.startItemDesc, ML ("First Item"));

			DSP_FLD ("startItem");
			DSP_FLD ("startItemDesc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}

		cc = FindInmr (comm_rec.co_no, local_rec.startItem, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.startItem);
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			print_mess (ML (mlStdMess001));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		SuperSynonymError ();

		strcpy (local_rec.startItemDesc,inmr_rec.description);
		DSP_FLD ("startItemDesc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("endItem"))
	{
		if (FLD ("endItem") == ND)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			strcpy (local_rec.endItem, "~~~~~~~~~~~~~~~~");
			strcpy (local_rec.endItemDesc, ML ("Last Item"));

			DSP_FLD ("endItem");
			DSP_FLD ("endItemDesc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}

		cc = FindInmr (comm_rec.co_no, local_rec.endItem, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.endItem);
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			print_mess (ML (mlStdMess001));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		SuperSynonymError ();

		strcpy (local_rec.endItemDesc,inmr_rec.description);
		DSP_FLD ("endItemDesc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("startClass"))
	{
		if (last_char == FN16)
		{
			prog_exit = TRUE;
			return (EXIT_SUCCESS);
		}

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate start group 
	 */
	if (LCHECK ("startCat"))
	{
		if (FLD ("startCat") == ND)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			sprintf (local_rec.startCat, "%-11.11s", " ");
			strcpy (local_rec.startCatDesc, ML ("First Category"));

			DSP_FLD ("startCat");
			DSP_FLD ("startCatDesc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchExcf (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (excf_rec.co_no,comm_rec.co_no);
		strcpy (excf_rec.cat_no,local_rec.startCat);

		if (!dflt_used)
		{
			cc = find_rec (excf,&excf_rec,COMPARISON,"r");
			if (cc)
			{
				errmess (ML (mlStdMess004));
				return (EXIT_FAILURE);
			}
		}
		else
		{
			sprintf (local_rec.startCat,"%-11.11s","           ");
			strcpy (excf_rec.cat_desc, ML ("BEGINNING OF RANGE"));
		}
		if (prog_status != ENTRY && strcmp (local_rec.startCat,local_rec.endCat) > 0)
		{
			errmess (ML (mlStdMess006));
			sleep (sleepTime);
			return (EXIT_SUCCESS);
		}
		strcpy (local_rec.startCatDesc, excf_rec.cat_desc);
		DSP_FLD ("startCatDesc");
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate end group 
	 */
	if (LCHECK ("endCat"))
	{
		if (FLD ("endCat") == ND)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			strcpy (local_rec.endCat, "~~~~~~~~~~~");
			strcpy (local_rec.endCatDesc, ML ("Last Category"));

			DSP_FLD ("endCat");
			DSP_FLD ("endCatDesc");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchExcf (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (excf_rec.co_no,comm_rec.co_no);
		strcpy (excf_rec.cat_no,local_rec.endCat);

		if (!dflt_used)
		{
			cc = find_rec (excf,&excf_rec,COMPARISON,"r");
			if (cc)
			{
				errmess (ML (mlStdMess004));
				return (EXIT_FAILURE);
			}
		}
		else
		{
			sprintf (local_rec.endCat,"%-11.11s","~~~~~~~~~~~");
			strcpy (excf_rec.cat_desc, ML ("END OF RANGE"));
		}
		if (strcmp (local_rec.startCat,local_rec.endCat) > 0)
		{
			errmess (ML (mlStdMess006));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.endCatDesc, excf_rec.cat_desc);
		DSP_FLD ("endCatDesc");
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

void
Process (void)
{
	int	sortOpen = FALSE;
	int	data_found = FALSE;

	dsp_screen ("Reading Bills For Items", comm_rec.co_no, comm_rec.co_name);
	strcpy (inmr_rec.co_no, comm_rec.co_no);

	if (BY_ITEM)
		sprintf (inmr_rec.item_no, "%-16.16s", local_rec.startItem);
	else
	{
		sprintf (inmr_rec.inmr_class,"%-1.1s",   local_rec.startClass);
		sprintf (inmr_rec.category, "%-11.11s", local_rec.startCat);
		sprintf (inmr_rec.item_no,  "%-16.16s", " ");
	}

	cc = find_rec (inmr, &inmr_rec, GTEQ, "r");
	while (!cc && !strcmp (inmr_rec.co_no, comm_rec.co_no) &&
		 ((BY_ITEM &&
		strcmp (inmr_rec.item_no,local_rec.endItem) <= 0) ||
		 (!BY_ITEM &&
		strcmp (inmr_rec.inmr_class,local_rec.endClass) <= 0)))
	{
		if (!strcmp (inmr_rec.inmr_class, local_rec.endClass) &&
		    strcmp (inmr_rec.category, local_rec.endCat) > 0)
			break;

		if (strcmp (inmr_rec.source, "MC") &&
		    strcmp (inmr_rec.source, "MP") &&
		    strcmp (inmr_rec.source, "BM") &&
		    strcmp (inmr_rec.source, "BP"))
		{
			cc = find_rec (inmr, &inmr_rec, NEXT, "r");
			continue;
		}

		bmms_rec.hhbr_hash = inmr_rec.hhbr_hash;
		cc = find_rec (bmms, &bmms_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (inmr, &inmr_rec, NEXT, "r");
			continue;
		}

		inum_rec.hhum_hash = inmr_rec.std_uom;
		cc = find_rec (inum, &inum_rec, COMPARISON, "r");
		if (cc)
		{
			sprintf (inum_rec.uom,  "%-4.4s", " ");
			sprintf (inum_rec.desc, "%-40.40s", " ");
		}

		if (!sortOpen)
		{
			/*
			 * Allocate the initial array.
			 */
			ArrAlloc (&sortDetails, &sortRec, sizeof (struct SortStruct), 10);
			sortCnt = 0;
			sortOpen = TRUE;
		}

		dsp_process ("Item: ", inmr_rec.item_no);

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
			"%-1.1s%-11.11s%-2.2s%-16.16s",
			inmr_rec.inmr_class,
			inmr_rec.category,
			inmr_rec.source,
			inmr_rec.item_no
		);
		strcpy (sortRec [sortCnt].itemClass, inmr_rec.inmr_class);
		strcpy (sortRec [sortCnt].itemCategory, inmr_rec.category);
		strcpy (sortRec [sortCnt].itemSource, inmr_rec.source);
		strcpy (sortRec [sortCnt].itemNo, inmr_rec.item_no);
		strcpy (sortRec [sortCnt].itemDescription, inmr_rec.description);
		strcpy (sortRec [sortCnt].itemUom, inum_rec.uom);
		strcpy (sortRec [sortCnt].uomDescription, inum_rec.desc);
		/*
		 * Increment array counter.
		 */
		sortCnt++;
		data_found = TRUE;

		cc = find_rec (inmr, &inmr_rec, NEXT, "r");
	}

	if (data_found)
	{
		ProcessSorted ();
		fprintf (fout, ".EOF\n");
		fclose (fout);
	}
}

void
ProcessSorted (void)
{
	int		i,
			firstTime = TRUE;

	/*
	 * Sort the array in item description order.
	 */
	qsort (sortRec, sortCnt, sizeof (struct SortStruct), SortFunc);

	for (i = 0; i < sortCnt; i++)
	{
		sprintf 
		(
			currentGroup, 
			"%-1.1s%-11.11s", 
			sortRec [i].itemClass, 
			sortRec [i].itemCategory
		);
		if (strcmp (currentGroup, previousGroup) || firstTime)
		{
			if (firstTime)
				HeadingOutput ();

			CategoryHeading ();

			if (!firstTime)
				fprintf (fout, ".PA\n");

			strcpy (previousGroup, currentGroup);
			firstTime = FALSE;
		}

		dsp_process ("Item: ", sortRec [i].itemNo);

		fprintf 
		(
			fout,
			"| %-16.16s | %-40.40s |   %-2.2s   | %-4.4s  %-40.40s |\n",
			sortRec [i].itemNo,
			sortRec [i].itemDescription, 
			sortRec [i].itemSource, 
			sortRec [i].itemUom, 
			sortRec [i].uomDescription
		);
	}
	/*
	 *	Free up the array memory
	 */
	ArrDelete (&sortDetails);
}

int
CategoryHeading (void)
{

	strcpy (excf_rec.co_no, comm_rec.co_no);
	sprintf (excf_rec.cat_no, "%-11.11s", currentGroup + 1);
	cc = find_rec (excf, &excf_rec, COMPARISON, "r");
	if (cc)
		sprintf (excf_rec.cat_desc, "%-40.40s", ML ("No Description"));

	expand (err_str, excf_rec.cat_desc);

	fprintf (fout,
		".PD|  %-1.1s   %-11.11s   %-80.80s                   |\n",
		currentGroup,
		currentGroup + 1,
		err_str);

	return (EXIT_SUCCESS);
}

/*
 * Initialize for Screen or Printer Output. 
 */
void
HeadingOutput (void)
{
	/*
	 * Open pipe to pformat 
	 */
	if ((fout = popen ("pformat","w")) == NULL)
		sys_err ("Error in pformat During (POPEN)", errno,PNAME);

	/*
	 * Initialize printer for output.  
	 */
	fprintf (fout,".START%s <%s>\n",DateToString (TodaysDate ()),PNAME);
	fprintf (fout,".LP%d\n",local_rec.printerNo);
	fprintf (fout,".10\n");
	fprintf (fout,".PI10\n");
	fprintf (fout,".L121\n");
	fprintf (fout,".EUNDEFINED BILL OF MATERIALS REPORT\n");
	fprintf (fout,".E%s - %s\n",comm_rec.co_no, clip (comm_rec.co_name));
	fprintf (fout,".B1\n");
	if (BY_ITEM)
		fprintf (fout,
			".CItem Range From : %-16.16s To %-16.16s\n",
			local_rec.startItem,
			local_rec.endItem);
	else
		fprintf (fout,
			".CGroup Range From : %-1.1s %-11.11s To %-1.1s %-11.11s\n",
			local_rec.startClass,
			local_rec.startCat,
			local_rec.endClass,
			local_rec.endCat);

	fprintf (fout,".R=====================================================");
	fprintf (fout,"=============================");
	fprintf (fout,"=======================================\n");

	fprintf (fout,"=============================");
	fprintf (fout,"=====================================================");
	fprintf (fout,"=======================================\n");

	fprintf (fout,"|   ITEM NUMBER    |  DESCRIPTION                      ");
	fprintf (fout,"       | SOURCE | HOLDING UNIT                                   |\n");

	fprintf (fout,"|------------------|-----------------------------------");
	fprintf (fout,"-------|--------|------------------------------------------------|\n");
}

/*
 * Search for Category master file. 
 */
void
SrchExcf (
	char	*key_val)
{
	_work_open (11,0,40);
	save_rec ("#Category.", "#Description");
	strcpy (excf_rec.co_no,comm_rec.co_no);
	sprintf (excf_rec.cat_no ,"%-11.11s",key_val);
	cc = find_rec (excf, &excf_rec, GTEQ, "r");
	while (!cc && !strncmp (excf_rec.cat_no,key_val,strlen (key_val)) && 
				  !strcmp (excf_rec.co_no, comm_rec.co_no))
	{
		cc = save_rec (excf_rec.cat_no, excf_rec.cat_desc);
		if (cc)
			break;

		cc = find_rec (excf, &excf_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (excf_rec.co_no,comm_rec.co_no);
	sprintf (excf_rec.cat_no,"%-11.11s",temp_str);
	cc = find_rec (excf, &excf_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, excf, "DBFIND");
}

int
heading (
	int		scn)
{
	if (!restart)
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();

		sprintf (err_str, " %s ", ML (mlBmMess017));
		rv_pr (err_str, 26,0,1);
		line_at (1,0,80);

		if (BY_ITEM)
			box (0,3,80,4);
		else
			box (0,3,80,7);

		line_at (5,1,79);

		if (!BY_ITEM)
			line_at (8,1,79);
		
		line_at (20,0,80);
		line_at (22,0,80);

		print_at (21,0, ML (mlStdMess038), comm_rec.co_no,comm_rec.co_name);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
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

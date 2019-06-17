/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: sup_perf.c,v 5.4 2002/07/26 05:31:17 scott Exp $
|  Program Name  : (po_sup_perf.c)
|  Program Desc  : (Print Delivery Performance by Supplier.)
|---------------------------------------------------------------------|
|  Author        : Dirk Heinsius   | Date Written  : 19/08/93         |
|---------------------------------------------------------------------|
| $Log: sup_perf.c,v $
| Revision 5.4  2002/07/26 05:31:17  scott
| Updated to add new sort routines and remove old disk based routines.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sup_perf.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PO/po_sup_perf/sup_perf.c,v 5.4 2002/07/26 05:31:17 scott Exp $";

#include 	<pslscr.h>
#include 	<get_lpno.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<ml_std_mess.h>
#include 	<ml_po_mess.h>
#include 	<arralloc.h>

#define		SORT_FILENAME	"posuprf"

#include	"schema"

struct commRecord	comm_rec;
struct sumrRecord	sumr_rec;
struct suphRecord	suph_rec;

	int		envVarCrCo 		= 0,
			envVarCrFind 	= 0,
			firstFlag 		= TRUE,
			dataFound 		= FALSE;

	char	branchNumber [3];

	long	lsystemDate;

	char	*data	= "data";
		
	FILE	*fout;

/*
 *	Structure for dynamic array.
 */
struct SortStruct
{
	char	sortCode [19];
	char	supplierNo	[sizeof sumr_rec.crd_no]; 
	char	csmNo		[sizeof suph_rec.csm_no];
	char	poNo		[sizeof suph_rec.po_no];
	char	grinNo		[sizeof suph_rec.grn_no];
	char	wkString1	[11];
	char	wkString2	[11];
	char	wkComments	[21];
}	*sortRec;
	DArray sortDetails;
	int	sortCnt = 0;

int		SortFunc			(const	void *,	const void *);

extern	int		TruePosition;
/*
 * Local & Screen Structures. 
 */
struct {
	char	startSupplierNo [7];
	char	startSupplierName [41];
	char	endSupplierNo [7];
	char	endSupplierName [41];
	long	startDate;
	char	startDateString [11];
	long	endDate;
	char	endDateString [11];
	int		printerNo;
	char 	back [2];
	char	backDesc [4];
	char	onight [2];
	char	onightDesc [4];
	char	dummy [11];
	char	systemDate [11];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "startSupplier",	 4, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", " ", "Start Supplier      ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.startSupplierNo},
	{1, LIN, "startSupplierName",	 4, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.startSupplierName},
	{1, LIN, "endSupplier",	 5, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", "~~~~~~", "End Supplier        ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.endSupplierNo},
	{1, LIN, "endSupplierName",	 5, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.endSupplierName},
	{1, LIN, "startDate",	 7, 2, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "00/00/00", "Start Receipt Date  ", " ",
		YES, NO,  JUSTLEFT, "", "", (char *)&local_rec.startDate},
	{1, LIN, "endDate",	 8, 2, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", local_rec.systemDate, "End Receipt Date    ", " ",
		YES, NO,  JUSTLEFT, "", "", (char *)&local_rec.endDate},
	{1, LIN, "printerNo",	 10, 2, INTTYPE,
		"NN", "          ",
		" ", "1", "Printer Number      ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.printerNo},
	{1, LIN, "back",	 11, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Background          ", " ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.back},
	{1, LIN, "backDesc",	 11, 25, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.backDesc},
	{1, LIN, "onight",	 12, 2, CHARTYPE,
		"U", "          ",
		" ", "N", "Overnight           ", " ",
		YES, NO,  JUSTLEFT, "YN", "", local_rec.onight},
	{1, LIN, "onightDesc",	 12, 25, CHARTYPE,
		"AAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.onightDesc},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};


#include <FindSumr.h>
/*
 * Function Declarations 
 */
void	shutdown_prog 		(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
int 	spec_valid 			(int);
void 	RunProgram 			(char *);
void 	HeadingOutput 		(void);
void 	PrintRuler 			(void);
void 	SortFile 			(void);
void 	ProcessFile 		(void);
void 	PrintSupplier 		(char *);
void 	PrintSuph 			(int);
int 	heading 			(int);


/*
 * Main Processing Routine 
 */
int
main (
 int argc, 
 char * argv [])
{
	if (argc != 1 && argc != 6)
	{
		print_at (23, 0, mlPoMess708, argv [0]);
		return (EXIT_FAILURE);
	}
	TruePosition	=	TRUE;

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));
	lsystemDate = TodaysDate ();

	SETUP_SCR (vars);

	envVarCrCo 		= atoi (get_env ("CR_CO"));
	envVarCrFind 	= atoi (get_env ("CR_FIND"));

	OpenDB ();

	strcpy (branchNumber, (envVarCrCo) ? comm_rec.est_no : " 0");

	if (argc == 6)
	{
		sprintf (local_rec.startSupplierNo, "%-6.6s", argv [1]);
		sprintf (local_rec.endSupplierNo, "%-6.6s", argv [2]);
		local_rec.startDate	= StringToDate (argv [3]);
		local_rec.endDate	= StringToDate (argv [4]);
		local_rec.printerNo = atoi (argv [5]);

		dsp_screen ("Processing : Printing Delivery Performance By Supplier.",
					comm_rec.co_no,comm_rec.co_name);

		if ((fout = popen ("pformat","w")) == (FILE *) NULL)
			file_err (errno, "pformat", "POPEN");

		/*
		 * Allocate the initial array.
		 */
		ArrAlloc (&sortDetails, &sortRec, sizeof (struct SortStruct), 10);
		sortCnt = 0;
		
		SortFile ();

		if (dataFound == TRUE)
		{
			HeadingOutput ();
			ProcessFile ();
			fprintf (fout,".EOF\n");
		}
		/*
		 *	Free up the array memory
		 */
		ArrDelete (&sortDetails);

		pclose (fout);
		shutdown_prog ();
        return (EXIT_SUCCESS);
	}

	/*
	 * Setup required parameters 
	 */
	init_scr ();				/*  sets terminal from termcap	*/
	set_tty ();              /*  get into raw mode			*/
	set_masks ();			/*  setup print using masks		*/

	while (prog_exit == 0)
	{
		/*
		 * Reset control flags 
		 */
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;
		init_vars (1);		/*  set default values		*/

		/*
		 * Entry screen 1 linear input 
		 */
		heading (1);
		entry (1);
		if (restart || prog_exit)
			continue;

		/*
		 * Edit screen 1 linear input 
		 */
		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
			continue;

		strcpy (err_str, ML (mlPoMess220));
		RunProgram (argv [0]);
		prog_exit = 1;
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
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (suph, suph_list, SUPH_NO_FIELDS, "suph_id_no2");
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, (!envVarCrFind) ? "sumr_id_no" 
							    : "sumr_id_no3");
}

/*
 * Close data base files 
 */
void
CloseDB (void)
{
	abc_fclose (sumr);
	abc_fclose (suph);
	abc_dbclose (data);
}

int
spec_valid (
	int	field)
{
	/*
	 * Validate Supplier 
	 */
	if (LCHECK ("startSupplier"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.startSupplierName, ML ("Start Supplier"));
			DSP_FLD ("startSupplierName");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}
		if ((strcmp (local_rec.startSupplierNo, local_rec.endSupplierNo) > 0) &&
		    (strcmp (local_rec.endSupplierNo, "      ") != 0))
		{
			print_mess (ML (mlStdMess017));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (sumr_rec.co_no, comm_rec.co_no);
		strcpy (sumr_rec.est_no, branchNumber);
		strcpy (sumr_rec.crd_no, pad_num (local_rec.startSupplierNo));
		cc = find_rec (sumr, &sumr_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess022));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.startSupplierName, sumr_rec.crd_name);
		DSP_FLD ("startSupplierName");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("endSupplier"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.endSupplierName, ML ("End Supplier"));
			DSP_FLD ("endSupplierName");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SumrSearch (comm_rec.co_no, branchNumber, temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (sumr_rec.co_no, comm_rec.co_no);
		strcpy (sumr_rec.est_no, branchNumber);
		strcpy (sumr_rec.crd_no, pad_num (local_rec.endSupplierNo));
		cc = find_rec (sumr, &sumr_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess022));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (strcmp (local_rec.startSupplierNo, local_rec.endSupplierNo) > 0)
		{
			print_mess (ML (mlStdMess018));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.endSupplierName, sumr_rec.crd_name);
		DSP_FLD ("endSupplierName");
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate start date. 
	 */
	if (LCHECK ("startDate"))
	{
		if (dflt_used)
		{
			local_rec.startDate = 0L;
			DSP_FLD ("startDate");
			return (EXIT_SUCCESS);
		}
		if (local_rec.startDate > local_rec.endDate && local_rec.endDate > 0L)
		{
			strcpy (local_rec.startDateString, DateToString (local_rec.startDate));
			strcpy (local_rec.endDateString, DateToString (local_rec.endDate));
			print_mess (ML (mlStdMess019));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate end date. 
	 */
	if (LCHECK ("endDate"))
	{
		if (dflt_used)
		{
			local_rec.endDate = lsystemDate;
			DSP_FLD ("endDate");
			return (EXIT_SUCCESS);
		}

		if (local_rec.endDate < local_rec.startDate)
		{
			strcpy (local_rec.startDateString, DateToString (local_rec.startDate));
			print_mess (ML (mlStdMess026));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("printerNo"))
	{
		if (SRCH_KEY)
		{
			local_rec.printerNo = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if (!valid_lp (local_rec.printerNo))
		{
			print_mess (ML (mlStdMess020));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("back"))
	{
		strcpy (local_rec.backDesc, (local_rec.back [0] == 'Y') ? 
		       ML ("Yes") : ML ("No "));
		DSP_FLD ("backDesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("onight"))
	{
		strcpy (local_rec.onightDesc, (local_rec.onight [0] == 'Y') ? 
		       ML ("Yes") : ML ("No "));
		DSP_FLD ("onightDesc");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

void
RunProgram (
	char *programName)
{
	strcpy (local_rec.startDateString, DateToString (local_rec.startDate));
	strcpy (local_rec.endDateString, DateToString (local_rec.endDate));

	shutdown_prog ();

	if (local_rec.onight [0] == 'Y')
	{
		sprintf 
		(
			err_str, 
			"ONIGHT \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%d\"",
			programName,
			local_rec.startSupplierNo,
			local_rec.endSupplierNo,
			local_rec.startDateString,
			local_rec.endDateString,
			local_rec.printerNo
		);
		SystemExec (err_str, TRUE);
	}
	else
	{
		sprintf 
		(
			err_str, 
			"%s \"%s\" \"%s\" \"%s\" \"%s\" \"%d\"",
			programName,
			local_rec.startSupplierNo,
			local_rec.endSupplierNo,
			local_rec.startDateString,
			local_rec.endDateString,
			local_rec.printerNo
		);
		SystemExec (err_str, (local_rec.back [0] == 'Y') ? TRUE : FALSE);
	}
}

/*
 * Start Output To Standard Print 
 */
void
HeadingOutput (void)
{
	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (fout, ".LP%d\n", local_rec.printerNo);
	fprintf (fout, ".PI12\n");

	fprintf (fout, ".14\n");
	fprintf (fout, ".L112\n");
	fprintf (fout, ".B1\n");
	fprintf (fout, ".ECompany : %s - %s\n",
				  comm_rec.co_no, clip (comm_rec.co_name));
	fprintf (fout, ".EBranch  : %s - %s\n",
				  comm_rec.est_no, clip (comm_rec.est_name));
	fprintf (fout, ".B1\n");
	fprintf (fout, ".EDELIVERY PERFORMANCE BY SUPPLIER\n");
	fprintf (fout, ".B1\n");
	fprintf (fout, ".EAS AT %-24.24s\n",SystemTime ());
	fprintf (fout, ".B1\n");

	fprintf (fout, ".R===============");
	fprintf (fout, "================");
	fprintf (fout, "================");
	fprintf (fout, "==================================");
	fprintf (fout, "=======================\n");

	fprintf (fout, "===============");
	fprintf (fout, "================");
	fprintf (fout, "================");
	fprintf (fout, "==================================");
	fprintf (fout, "=======================\n");

	fprintf (fout, "!   SHIPMENT   ");
	fprintf (fout, "!PURCHASE ORDER ");
	fprintf (fout, "! GOODS RECEIPT ");
	fprintf (fout, "!         DELIVERY DATE           ");
	fprintf (fout, "!                     !\n");

	fprintf (fout, "!    NUMBER    ");
	fprintf (fout, "!    NUMBER     ");
	fprintf (fout, "!    NUMBER     ");
	fprintf (fout, "!  PROMISED/DUE  ! ACTUAL/RECEIPT ");
	fprintf (fout, "!  COMMENT            !\n");
 
	PrintRuler ();

	fflush (fout);
}

void
PrintRuler (void)
{
	fprintf (fout, "!--------------");
	fprintf (fout, "!---------------");
	fprintf (fout, "!---------------");
	fprintf (fout, "!----------------!----------------");
	fprintf (fout, "!---------------------!\n");
	fflush (fout);
}

void
SortFile (void)
{
	char	workString1 [11],
			workString2 [11],
			workComments [21];

	strcpy (sumr_rec.co_no, comm_rec.co_no);
	if (envVarCrFind)
		strcpy (sumr_rec.est_no, "  ");
	else
		strcpy (sumr_rec.est_no, branchNumber);

	strcpy (sumr_rec.crd_no, local_rec.startSupplierNo);

	cc = find_rec (sumr, &sumr_rec, GTEQ, "r");

	while (!cc && 
		   !strcmp (sumr_rec.co_no, comm_rec.co_no) && 
		   strcmp (sumr_rec.crd_no, local_rec.startSupplierNo) >= 0 && 
		   strcmp (sumr_rec.crd_no, local_rec.endSupplierNo) <= 0) 
	{
		if (strcmp (sumr_rec.est_no, branchNumber) != 0)
		{
			if (!envVarCrFind)
			{
				cc = 1;
				continue;
			}
		}
		suph_rec.hhsu_hash = sumr_rec.hhsu_hash;
		strcpy (suph_rec.status," ");

		cc = find_rec (suph, &suph_rec, GTEQ, "r");

		while (!cc &&
			   !strcmp (suph_rec.br_no, comm_rec.est_no) && 
			    suph_rec.hhsu_hash == sumr_rec.hhsu_hash)
		{
			if ((suph_rec.rec_date >= local_rec.startDate) &&
				 (suph_rec.rec_date <= local_rec.endDate) &&
			    (suph_rec.due_date != 0L))
			{
			/*
			 * We now have an suph record matching our search         
			 * criteria so we convert it to a string format and      
			 * write it out to our sort file.                       
			 */
				dataFound = TRUE;
				strcpy (workString1, DateToString (suph_rec.due_date));
				strcpy (workString2, DateToString (suph_rec.rec_date));
				if (suph_rec.rec_qty == suph_rec.ord_qty)
					strcpy (workComments, "                    ");
				else
					strcpy (workComments, "Partial Receipt.    ");

				/*
				 * Check the array size before adding new element.
				 */
				if (!ArrChkLimit (&sortDetails, sortRec, sortCnt))
					sys_err ("ArrChkLimit (sortRec)", ENOMEM, PNAME);

				/*
				 * Load values into array element sortCnt.
				 */
				sprintf (sortRec [sortCnt].sortCode, "%-6.6s%-12.12s",
							sumr_rec.crd_no, suph_rec.csm_no);
				strcpy (sortRec [sortCnt].supplierNo, sumr_rec.crd_no);
				strcpy (sortRec [sortCnt].csmNo, suph_rec.csm_no);
				strcpy (sortRec [sortCnt].poNo,  suph_rec.po_no);
				strcpy (sortRec [sortCnt].grinNo,suph_rec.grn_no);
				strcpy (sortRec [sortCnt].wkString1, workString1);
				strcpy (sortRec [sortCnt].wkString2, workString2);
				strcpy (sortRec [sortCnt].wkComments,workComments);
				/*
				 * Increment array counter.
				 */
				sortCnt++;
			}
			cc = find_rec (suph, &suph_rec, NEXT, "r");
		}
		cc = find_rec (sumr, &sumr_rec, NEXT, "r");
	}
	/*
	 * Sort the array in item description order.
	 */
	qsort (sortRec, sortCnt, sizeof (struct SortStruct), SortFunc);
}

void
ProcessFile (void)
{
	char	lastSupplier [7],
			thisSupplier [7],
			lastLine [132],
			thisLine [132];

	int		i;

	strcpy (lastSupplier,  "      ");
	sprintf (lastLine, "%74.74s", " ");

	for (i = 0; i < sortCnt; i++)
	{
		sprintf (thisSupplier, "%6.6s", sortRec [i].supplierNo);

		dsp_process ("Supplier No. : ", thisSupplier);
		if (strcmp (lastSupplier, thisSupplier) != 0)
		{
			PrintSupplier (thisSupplier);
			strcpy (lastSupplier, thisSupplier);
		}
		sprintf 
		(
			thisLine, 
			"%s%s%s%s%s%s",
			sortRec [i].supplierNo,
			sortRec [i].csmNo,
			sortRec [i].poNo,
			sortRec [i].grinNo,
			sortRec [i].wkString1,
			sortRec [i].wkString2
		);
		if (strcmp (lastLine, thisLine) != 0)
		{
			PrintSuph (i);
			sprintf 
			(
				lastLine, 
				"%s%s%s%s%s%s",
				sortRec [i].supplierNo,
				sortRec [i].csmNo,
				sortRec [i].poNo,
				sortRec [i].grinNo,
				sortRec [i].wkString1,
				sortRec [i].wkString2
			);
		}
	}
}

void
PrintSupplier (
	char	*supplierNo)
{
	strcpy (sumr_rec.co_no, comm_rec.co_no);
	strcpy (sumr_rec.est_no, branchNumber);
	strcpy (sumr_rec.crd_no, supplierNo);

	cc = find_rec (sumr, &sumr_rec, EQUAL, "r");

	sprintf 
	(
		err_str, 
		"! %6.6s (%9.9s) (%40.40s)                  !                     !",
					sumr_rec.crd_no,
					sumr_rec.acronym,
					sumr_rec.crd_name
	);
	fprintf (fout, ".PD%s\n", err_str);

	if (!firstFlag) 
	{
		PrintRuler ();
		fprintf (fout, "%s\n", err_str);
	}
	firstFlag = FALSE;
}

void
PrintSuph (
	int		lineNo)
{
	fprintf (fout, ".LRP4\n");

	fprintf 
	(
		fout,
	   "! %-12.12s !%-15.15s!%-15.15s!   %10.10s   !   %10.10s   ! %-20.20s!\n",
		sortRec [lineNo].csmNo,
		sortRec [lineNo].poNo,
		sortRec [lineNo].grinNo,
		sortRec [lineNo].wkString1,
		sortRec [lineNo].wkString2,
		sortRec [lineNo].wkComments
	);
}

int
heading (
 int scn)
{
	if (restart) 
		return (EXIT_SUCCESS);

	if (scn != cur_screen)
		scn_set (scn);
	clear ();
	rv_pr (ML (mlPoMess056),25,0,1);

	box (0,3,80,9);
	line_at (1,0,80);
	line_at (6,1,79);
	line_at (9,1,79);
	line_at (20,0,80);

	strcpy (err_str, ML (mlStdMess038));
	print_at (21,0,err_str, comm_rec.co_no,comm_rec.co_name);
	strcpy (err_str, ML (mlStdMess039));
	print_at (22,0,err_str, comm_rec.est_no,comm_rec.est_name);
	line_cnt = 0;
	scn_write (scn);
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

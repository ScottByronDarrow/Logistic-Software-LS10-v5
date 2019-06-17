/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: po_ass_prn.c,v 5.4 2002/07/17 09:57:32 scott Exp $
|  Program Name  : (po_ass_prn.c  )                                   |
|  Program Desc  : (Sales Orders and Purchase Orders Report     )     |
|                  (                                            )     |
|---------------------------------------------------------------------|
| $Log: po_ass_prn.c,v $
| Revision 5.4  2002/07/17 09:57:32  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.3  2002/07/03 06:41:44  scott
| Updated to change disk based sorts to memory based sort
|
| Revision 5.2  2001/08/09 09:15:13  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:36:37  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:11:01  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:32:20  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2000/11/20 07:39:11  scott
| New features related to 3PL environment
| New features related to Number Plates
| All covered in release 3 notes
|
| Revision 3.0  2000/10/10 12:17:25  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.0  2000/07/15 09:04:55  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.13  1999/12/06 01:32:28  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.12  1999/11/11 06:43:11  scott
| Updated to remove PNAME from heading as not available with ^P
|
| Revision 1.11  1999/11/05 05:17:07  scott
| Updated to fix warning errors using -Wall compile flag.
|
| Revision 1.10  1999/10/14 03:04:19  nz
| Updated from Ansi testing by Scott.
|
| Revision 1.9  1999/09/29 10:11:51  scott
| Updated to be consistant on function names.
|
| Revision 1.8  1999/09/21 04:37:54  scott
| Updated from Ansi project
|
| Revision 1.7  1999/06/17 10:06:13  scott
| Updated to remove old read_comm (), Added cvs logs, changed database names.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: po_ass_prn.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PO/po_ass_prn/po_ass_prn.c,v 5.4 2002/07/17 09:57:32 scott Exp $";

#include	<pslscr.h>
#include	<get_lpno.h>
#include 	<dsp_screen.h>
#include 	<dsp_process2.h>
#include 	<ml_std_mess.h>
#include 	<ml_po_mess.h>
#include 	<arralloc.h>


#include	"schema"

struct commRecord	comm_rec;
struct posoRecord	poso_rec;
struct inmrRecord	inmr_rec;
struct sumrRecord	sumr_rec;
struct pohrRecord	pohr_rec;
struct polnRecord	poln_rec;
struct solnRecord	soln_rec;
struct sohrRecord	sohr_rec;
struct cumrRecord	cumr_rec;


	FILE	*pout;

	char 	systemDate [11];
	char 	*data = "data";
	int		printerNumber = 1;

/*
 *	Structure for dynamic array.
 */
struct SortStruct
{
	char	sortCode [22];
	char 	purchaseOrderNo [16];
	char	supplierNo [7];
	char	purchaseStatus [2];
	long	poDueDate;
	char	itemNumber [17];
	float	qtyOrder;
	float	qtyBorder;
	char	salesOrderNo [9];
	char	customerNo [7];
	long	soDueDate;
}	*sortRec;
	DArray sortDetails;
	int	sortCnt = 0;

int		SortFunc			(const	void *,	const void *);

	extern	int	TruePosition;

/*
 * Local & Screen Structures. 
 */
struct {
	char 	dummy [11];
	long	startDate;
	long	endDate;
    int     printerNo;
	char	back [2];
	char	backDesc [11];
	char	onite [2];
	char	oniteDesc [11];
	} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "startDate",	 3, 2, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "00/00/00", "Start Date        ", "Enter Beginning Date.",
		NO, NO, JUSTRIGHT, "", "", (char *)&local_rec.startDate},
	{1, LIN, "endDate",	 4, 2, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "systemDate", "End Date          ", "Enter Ending Date.",
		NO, NO, JUSTRIGHT, "", "", (char *)&local_rec.endDate},
	{1, LIN, "printerNo", 	6, 2, INTTYPE, 
		"NN", "          ", 
		" ", " ", "Printer Number    ", " Default is 1.", 
		 YES, NO,  JUSTRIGHT, "", "", (char *)&local_rec.printerNo}, 
	{1, LIN, "back", 7, 2, CHARTYPE,
       "U", "          ", 
       " ", "N",          "Background        ", " Process in Background [Y/N] ",
       YES, NO, JUSTLEFT, "YN", "", local_rec.back},
	{1, LIN, "backDesc", 7, 25, CHARTYPE,
       "UUUUUUUUUU", "          ", 
       " ", "", "", "",
       NA, NO, JUSTLEFT, "", "", local_rec.backDesc},
	{1, LIN, "onite", 8, 2, CHARTYPE,
       "U", "          ", 
       " ", "N", 		   "Overnight         ", " Process Overnight [Y/N] ",
       YES, NO, JUSTLEFT, "YN", "", local_rec.onite},
	{1, LIN, "oniteDesc", 8, 25, CHARTYPE,
       "UUUUUUUUUU", "          ", 
       " ", "", "", "",
       NA, NO, JUSTLEFT, "", "", local_rec.backDesc},
	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*
 * Function Declarations 
 */
void 	RunProgram 		 (char *);
void 	shutdown_prog 	 (void);
void 	OpenDB 			 (void);
void 	CloseDB 		 (void);
void 	StoreData 		 (void);
void 	DisplayReport 	 (void);
void 	HeadOutput 		 (void);
int 	spec_valid 		 (int);
int 	ProcessFile 	 (void);
int 	heading 		 (int);

/*
 * Main Processing Routine 
 */
int
main (
 int argc, 
 char * argv [])
{
	TruePosition = TRUE;

	SETUP_SCR 	 (vars);

	strcpy (systemDate, DateToString (TodaysDate ()));

	/*
	 * Set up required parameters  
	 */
	OpenDB	 ();
	init_scr ();			
	set_tty ();         

	if (argc == 4)
	{
		local_rec.printerNo		=	atoi (argv [3]);
		local_rec.startDate	=	StringToDate (argv [1]);
		local_rec.endDate	=	StringToDate (argv [2]);
		dsp_screen (" Processing Sales and Purchase Order Assignment Report.",comm_rec.co_no,comm_rec.co_name);

		HeadOutput ();
		ProcessFile (); 
		DisplayReport (); 
		shutdown_prog ();
        return (EXIT_SUCCESS);
	}
	set_masks ();	
	init_vars (1);

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
		init_vars (1);		

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

		strcpy (err_str, ML (mlPoMess229));
		RunProgram (argv [0]);
		prog_exit = 1;
	}
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

void
RunProgram (
	char	*programName)
{
	char	DateStr [2] [11];

	strcpy (DateStr [0], DateToString (local_rec.startDate));
	strcpy (DateStr [1], DateToString (local_rec.endDate));

	shutdown_prog ();

	if (local_rec.onite [0] == 'Y')
	{
		sprintf 
		(
			err_str, 
			"ONIGHT \"%s\" \"%s\" \"%s\" \"%d\" \"%s\"",
			programName,
			DateStr [0],
			DateStr [1],
			local_rec.printerNo,
			ML (mlPoMess229)
		);
		SystemExec (err_str, TRUE);
    } 
    else
	{
		sprintf 
		(
			err_str, 
			"\"%s\" \"%s\" \"%s\" \"%d\"",
			programName,
			DateStr [0],
			DateStr [1],
			local_rec.printerNo
		);
		SystemExec (err_str, (local_rec.back [0] == 'Y') ? TRUE : FALSE);
	}
	return;
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
	open_rec (poso, poso_list, POSO_NO_FIELDS, "poso_hhpl_hash");
	open_rec (poln, poln_list, POLN_NO_FIELDS, "poln_hhpl_hash");
	open_rec (pohr, pohr_list, POHR_NO_FIELDS, "pohr_hhpo_hash");
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, "sumr_hhsu_hash");
	open_rec (soln, soln_list, SOLN_NO_FIELDS, "soln_hhsl_hash");
	open_rec (sohr, sohr_list, SOHR_NO_FIELDS, "sohr_hhso_hash");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
}

/*
 * Close data base files. 
 */
void
CloseDB (void)
{
	abc_fclose (inmr);
	abc_fclose (poso);
	abc_fclose (poln);
	abc_fclose (pohr);
	abc_fclose (sumr);
	abc_fclose (soln);
	abc_fclose (sohr);
	abc_fclose (cumr);
	abc_dbclose ("data");
}

int
spec_valid (
	int		field)
{
    /*
     * Validate Ending Date. 
     */
	if (LCHECK ("endDate"))
	{
		if (dflt_used)
		{
			local_rec.endDate = StringToDate (systemDate);
			DSP_FLD ("endDate"); 
			if (local_rec.startDate > local_rec.endDate)
			{
				print_mess (ML (mlStdMess026));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			return (EXIT_SUCCESS);
		}
	}

    /*
     * Validate Printer Number 
     */
	if (LCHECK ("printerNo"))
	{
		if (dflt_used)
		{
			local_rec.printerNo = 1;
			DSP_FLD ("printerNo");
			return (EXIT_SUCCESS);
		}

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
		DSP_FLD ("printerNo");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("back"))
	{
		if (local_rec.back [0] == 'Y')
		{
			strcpy (local_rec.backDesc, 	ML ("Yes"));
			strcpy (local_rec.oniteDesc, 	ML ("No "));
			entry_exit = TRUE;
		}
		else
			strcpy (local_rec.backDesc, 	ML ("No "));

		DSP_FLD ("backDesc");
		DSP_FLD ("oniteDesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("onite"))
	{
		if (local_rec.onite [0] == 'Y')
		{
			strcpy (local_rec.oniteDesc, ML ("Yes"));
			strcpy (local_rec.backDesc,  ML ("No "));
		}
		else
			strcpy (local_rec.oniteDesc, ML ("No "));

		DSP_FLD ("backDesc");
		DSP_FLD ("oniteDesc");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

int
ProcessFile (void)
{
	/*
	 * Allocate the initial array.
	 */
	ArrAlloc (&sortDetails, &sortRec, sizeof (struct SortStruct), 10);
	sortCnt = 0;

	poso_rec.hhpl_hash = 0L;	
	cc = find_rec (poso, &poso_rec, GTEQ, "r");
	while (!cc)
	{ 
		poln_rec.hhpl_hash = poso_rec.hhpl_hash;	
		cc = find_rec (poln, &poln_rec, EQUAL, "r");
		if (cc)
		{
			cc = find_rec (poso, &poso_rec, NEXT, "r");
			continue;
		}
		inmr_rec.hhbr_hash = poln_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
		if (cc)
		{
			cc = find_rec (poso, &poso_rec, NEXT, "r");
			continue;
		}
		pohr_rec.hhpo_hash = poln_rec.hhpo_hash;	
		cc = find_rec (pohr, &pohr_rec, EQUAL, "r");
		if (cc && !strcmp (pohr_rec.co_no, comm_rec.co_no) &&
				  !strcmp (pohr_rec.br_no, comm_rec.est_no))
		{
			cc = find_rec (poso, &poso_rec, NEXT, "r");
			continue;
		}
		sumr_rec.hhsu_hash = pohr_rec.hhsu_hash;	
		cc = find_rec (sumr, &sumr_rec, EQUAL, "r");
		if (cc)
		{
			cc = find_rec (poso, &poso_rec, NEXT, "r");
			continue;
		}
		soln_rec.hhsl_hash = poso_rec.hhsl_hash;	
		cc = find_rec (soln, &soln_rec, EQUAL, "r");
		if (cc)
		{
			cc = find_rec (poso, &poso_rec, NEXT, "r");
			continue;
		}
		sohr_rec.hhso_hash = soln_rec.hhso_hash;	
		cc = find_rec (sohr, &sohr_rec, EQUAL, "r");
		if (cc)
		{
			cc = find_rec (poso, &poso_rec, NEXT, "r");
			continue;
		}
		cumr_rec.hhcu_hash = sohr_rec.hhcu_hash;	
		cc = find_rec (cumr, &cumr_rec, EQUAL, "r");
		if (cc)
		{
			cc = find_rec (poso, &poso_rec, NEXT, "r");
			continue;
		}
		if (local_rec.startDate <= soln_rec.due_date &&
		    local_rec.endDate >= soln_rec.due_date)
				StoreData ();

		cc = find_rec (poso, &poso_rec, NEXT, "r");  
	}
	return (EXIT_SUCCESS);
}

void
StoreData (void)
{
	float	qtyBack	=	0;

	qtyBack = (soln_rec.qty_order + soln_rec.qty_bord) - (poln_rec.qty_ord);
	if (qtyBack < 0.00)
		qtyBack = 0.00;

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
		sortRec [sortCnt].sortCode, "%-15.15s%-6.6s",
		pohr_rec.pur_ord_no,
		sumr_rec.crd_no
	);
	strcpy (sortRec [sortCnt].purchaseOrderNo, 	pohr_rec.pur_ord_no);
	strcpy (sortRec [sortCnt].supplierNo, 		sumr_rec.crd_no);
	strcpy (sortRec [sortCnt].purchaseStatus, 	poln_rec.pur_status);
	strcpy (sortRec [sortCnt].salesOrderNo, 	sohr_rec.order_no);
	strcpy (sortRec [sortCnt].customerNo,		cumr_rec.dbt_no);
	sortRec [sortCnt].poDueDate	=	poln_rec.due_date;
	sortRec [sortCnt].soDueDate	=	soln_rec.due_date;
	sortRec [sortCnt].qtyOrder	= 	poln_rec.qty_ord;
	sortRec [sortCnt].qtyBorder	= 	qtyBack;
	/*
	 * Increment array counter.
	 */
	sortCnt++;
}

void 
DisplayReport (void)
{
	int		i;
	char	status_desc [16];

	/*
	 * Sort the array in item description order.
	 */
	qsort (sortRec, sortCnt, sizeof (struct SortStruct), SortFunc);

	for (i = 0; i < sortCnt; i++)
	{
		if (sortRec [i].purchaseStatus [0] == 'D')
			continue;

		switch (sortRec [i].purchaseStatus [0])
		{
			case 'O' :
				strcpy (status_desc, ML (" PO OUTSTANDING"));
				break;
			case 'R' :
				strcpy (status_desc, ML ("GOODS RECEIVED "));
				break;
			case 'C' :
				strcpy (status_desc, ML (" GOODS COSTED  "));
				break;
			case 'U' :
				strcpy (status_desc, ML (" NOT  APPROVED "));
				break;
			default : 
				strcpy (status_desc, ML ("UNKNOWN STATUS "));
				break;
		}
		fprintf (pout,"|%-15.15s|",		sortRec [i].purchaseOrderNo);
		fprintf (pout," %-6.6s   |",	sortRec [i].supplierNo);
		fprintf (pout,"%-15.15s     |", status_desc);
		fprintf (pout,"%-10.10s|",  	DateToString (sortRec [i].poDueDate));
		fprintf (pout,"%16.16s|", 		sortRec [i].itemNumber);
		fprintf (pout,"%8.2f   |",		sortRec [i].qtyOrder);
		fprintf (pout,"%8.2f   |", 		sortRec [i].qtyBorder);
		fprintf (pout,"%8.8s   |",  	sortRec [i].salesOrderNo);
		fprintf (pout," %6.6s   |", 	sortRec [i].customerNo);
		fprintf (pout," %10.10s  |\n",  DateToString (sortRec [i].soDueDate));
		fflush (pout); 

		dsp_process ("Order:", pohr_rec.pur_ord_no);
	}
}


/*
 * Print Details Heading. 
 */
void
HeadOutput (void)
{
	char	DspDate [2] [11];

	if ((pout = popen ("pformat","w")) == 0) 
		sys_err ("Error in pformat During (POPEN)",errno,PNAME);

	fprintf (pout, ".START%s<%s>\n", DateToString (comm_rec.inv_date), PNAME);
	fprintf (pout,".LP%d\n", printerNumber);
	fprintf (pout,".PI12\n");
	fprintf (pout,".12\n");
	fprintf (pout,".L158\n");
	fprintf (pout,".B1\n");
	fprintf (pout,".ESALES ORDER AND PURCHASE ORDER ASSIGNMENT\n");
	fprintf (pout,".B1\n");
	fprintf (pout,".E%s AS AT %s\n", comm_rec.co_short,SystemTime ());

	strcpy (DspDate [0], DateToString (local_rec.startDate));
	strcpy (DspDate [1], DateToString (local_rec.endDate));
	fprintf (pout,".E FROM : %s TO %s\n", DspDate [0],DspDate [1]);
	fprintf (pout,".E%s / %s\n", clip (comm_rec.est_name), comm_rec.cc_name);
			
	fprintf (pout,".R================");
	fprintf (pout,"===========");
	fprintf (pout,"=====================");
	fprintf (pout,"===========");
	fprintf (pout,"=================");
	fprintf (pout,"============");
	fprintf (pout,"============");
	fprintf (pout,"============");
	fprintf (pout,"===========");
	fprintf (pout,"===============\n");

	fprintf (pout,"================");
	fprintf (pout,"===========");
	fprintf (pout,"=====================");
	fprintf (pout,"===========");
	fprintf (pout,"=================");
	fprintf (pout,"============");
	fprintf (pout,"============");
	fprintf (pout,"============");
	fprintf (pout,"===========");
	fprintf (pout,"===============\n");

	fprintf (pout,"|      P.O.     ");
	fprintf (pout,"| SUPPLIER ");
	fprintf (pout,"|       STATUS       ");
	fprintf (pout,"|   P.O.   ");
	fprintf (pout,"|      ITEM      ");
	fprintf (pout,"|  QUANTITY ");
	fprintf (pout,"|  QUANTITY ");
	fprintf (pout,"|   SALES   ");
	fprintf (pout,"| CUSTOMER ");
	fprintf (pout,"| SALES ORDER |\n");

	fprintf (pout,"|     NUMBER    ");
	fprintf (pout,"|  NUMBER  ");
	fprintf (pout,"|                    ");
	fprintf (pout,"|   DATE   ");
	fprintf (pout,"|     NUMBER     ");
	fprintf (pout,"|   ORDER   ");
	fprintf (pout,"| BACKORDER ");
	fprintf (pout,"| ORDER NO. ");
	fprintf (pout,"|  NUMBER  ");
	fprintf (pout,"| REQ'D DATE  |\n");

	fprintf (pout,"|---------------");
	fprintf (pout,"|----------");
	fprintf (pout,"|--------------------");
	fprintf (pout,"|----------");
	fprintf (pout,"|----------------");
	fprintf (pout,"|-----------");
	fprintf (pout,"|-----------");
	fprintf (pout,"|-----------");
	fprintf (pout,"|----------");
	fprintf (pout,"|-------------|\n");
		
	fflush (pout);
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

/*
 * Print Heading. 
 */
int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);
		clear ();
		
		rv_pr (ML (mlPoMess128) ,20,0,1);
		
		line_at (1,0,80);
		line_at (5,1,79);
		line_at (19,0,80);

		box (0,2,80,6);

		print_at (20,0, ML (mlStdMess038) ,comm_rec.co_no,	comm_rec.co_name);
		print_at (21,0,ML (mlStdMess039) ,comm_rec.est_no,	comm_rec.est_name);
		print_at (22,0,ML (mlStdMess099) ,comm_rec.cc_no,	comm_rec.cc_name);

		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}

    return (EXIT_SUCCESS);
}

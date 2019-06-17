/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: po_itm_enq.c,v 5.3 2002/07/03 07:49:02 scott Exp $
|  Program Name  : (po_itm_enq.c)                                     |
|  Program Desc  : (Purchase Order Enquiry by Item.)                  |	
|---------------------------------------------------------------------|
|  Author        : Dirk Heinsius   | Date Written  : 31/08/93         |
|---------------------------------------------------------------------|
| $Log: po_itm_enq.c,v $
| Revision 5.3  2002/07/03 07:49:02  scott
| Updated to change disk based sorts to memory based sort
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: po_itm_enq.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PO/po_itm_enq/po_itm_enq.c,v 5.3 2002/07/03 07:49:02 scott Exp $";

#include 	<pslscr.h>
#include 	<ml_std_mess.h>
#include 	<ml_po_mess.h>
#include 	<arralloc.h>

#define		ScreenWidth		132
#define		SORT_FILENAME	"itmenq"

#include	"schema"

struct commRecord	comm_rec;
struct inmrRecord	inmr_rec;
struct inumRecord	inum_rec;
struct sumrRecord	sumr_rec;
struct sumrRecord	sumr2_rec;
struct suphRecord	suph_rec;

	int		envVarCrCo = 0;
	int		envVarCrFind = 0;

	char	branchNumber [3];

	char	*data	= "data",
			*sumr2	= "sumr2";
		
	FILE	*fout;

	float	PurCnvFct	=	1.00,
			CnvFct		=	1.00,
			StdCnvFct	=	1.00;

/*
 *	Structure for dynamic array.
 */
struct SortStruct
{
	char	sortCode 	[51];
	char	supplierNo	[sizeof sumr_rec.crd_no]; 
	char	acronym		[sizeof sumr_rec.acronym]; 
	char	branchNo	[sizeof suph_rec.br_no];
	char	poNo		[sizeof suph_rec.po_no];
	char	grnNo		[sizeof suph_rec.grn_no];
	char	uom 		[sizeof inum_rec.uom];
	long	dateOrder;
	long	dateReceipt;
	double	wsDouble;
	float	wsFloat;
}	*sortRec;
	DArray sortDetails;
	int	sortCnt = 0;

/*
 * Local & Screen Structures. 
 */
struct {
	char	itemNumber [17];
	char	itemDesc [41];
	char	selectPoType [2];
	char	selectPoDesc [25];
	long	cutoffDate;
	char	dummy [11];
} local_rec;


/*
 * Structure storing most recent date and lowest cost for each supplier	
 */
struct {
	char	supplierNo [7];
	long	selDate;
	double	selCost;
} selTable [1000];

extern	int		TruePosition;

static	struct	var	vars [] =
{
	{1, LIN, "itemNumber",	 4, 2, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "                ", "Select Item       ", "Enter item to enquire on",
		YES, NO,  JUSTLEFT, "", "", local_rec.itemNumber},
	{1, LIN, "itemDesc",	 4, 45, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.itemDesc},
	{1, LIN, "po_type",	 5, 2, CHARTYPE,
		"U", "          ",
		" ", "A", "Select P/O Type   ", "A)ll, M)ost Recent Receipt Date, L)owest Priced",
		YES, NO,  JUSTLEFT, "AML", "", local_rec.selectPoType},
	{1, LIN, "po_desc",	 5, 45, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.selectPoDesc},
	{1, LIN, "cutoffDate",	 6, 2, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "00/00/00", "Cutoff Date       ", "Ignore P/Os raised before this date",
		YES, NO,  JUSTLEFT, "", "", (char *)&local_rec.cutoffDate},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*
 * Function Declarations 
 */
void 	shutdown_prog 		(void);
void 	OpenDB 				(void);
void 	CloseDB 			(void);
void 	SupplierDisp 		(void);
void 	DisplayDataLine 	(int);
int 	spec_valid 			(int);
int 	heading 			(int);
int 	FindTableIndex 		(char *, int);
int		SortFunc			(const	void *,	const void *);


/*
 * Main Processing Routine 
 */
int
main (
	int		argc,
	char 	*argv [])
{
	TruePosition	=	TRUE;

	SETUP_SCR (vars);

	envVarCrCo 		= atoi (get_env ("CR_CO"));
	envVarCrFind 	= atoi (get_env ("CR_FIND"));

	OpenDB ();

	strcpy (branchNumber, (envVarCrCo) ? comm_rec.est_no : " 0");

	/*
	 * Setup required parameters 
	 */
	init_scr ();		/*  sets terminal from termcap	*/
	set_tty ();        	/*  get into raw mode			*/
	set_masks ();		/*  setup print using masks		*/

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

		/*
		 * Do supplier display        
		 */
		SupplierDisp ();
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
	abc_alias (sumr2, sumr);

	open_rec (inum, inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (suph, suph_list, SUPH_NO_FIELDS, "suph_hhbr_hash");
	open_rec (sumr, sumr_list, SUMR_NO_FIELDS, (!envVarCrFind) ? "sumr_id_no" 
														 	   : "sumr_id_no3");
	open_rec (sumr2,sumr_list, SUMR_NO_FIELDS, "sumr_hhsu_hash");
}

/*
 * Close data base files 
 */
void
CloseDB (void)
{
	abc_fclose (inum);
	abc_fclose (inmr);
	abc_fclose (sumr);
	abc_fclose (suph);
	SearchFindClose ();
	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
	/*
	 * Validate Item Number. 
	 */
	if (LCHECK ("itemNumber")) 
	{
		if (dflt_used)
			return (EXIT_FAILURE);

		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}
		cc = FindInmr (comm_rec.co_no, local_rec.itemNumber, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.itemNumber);
			cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			errmess (ML (mlStdMess001));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();
		
		strcpy (local_rec.itemNumber, inmr_rec.item_no);
		strcpy (local_rec.itemDesc,inmr_rec.description);
		DSP_FLD ("itemNumber");
		DSP_FLD ("itemDesc");

		inum_rec.hhum_hash	=	inmr_rec.std_uom;
		cc = find_rec ("inum", &inum_rec, COMPARISON, "r");
		StdCnvFct 	= (float) ((cc) ? 1.00 : inum_rec.cnv_fct);

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Selection Type 
	 */
	if (LCHECK ("po_type"))
	{
		switch (local_rec.selectPoType [0])
		{
		case 'A' : 
			strcpy (local_rec.selectPoDesc, ML ("All                     "));
			break;
		case 'M' : 
			strcpy (local_rec.selectPoDesc, ML ("Most Recent Receipt Date"));
			break;
		case 'L' : 
			strcpy (local_rec.selectPoDesc, ML ("Lowest Price            "));
			break;
		default  : 
			return (EXIT_FAILURE);
		} /* End Switch */

		DSP_FLD ("po_desc");
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}


/*
 * Supplier Display.
 */
void
SupplierDisp (void)
{
	double	wsDouble	= 0.00,
			wsLand		= 0.00;

	float	wsFloat		= 0.00;

	int		wsIdx		= 0,
			wsMax 		= 0,
			i;

	Dsp_open (1, 2, 12); 

	/*
	 * Allocate the initial array.
	 */
	ArrAlloc (&sortDetails, &sortRec, sizeof (struct SortStruct), 10);
	sortCnt = 0;

	Dsp_saverec ("  SUPPLIER  |   SUPPLIER  | BR |     P.O.      |     G.R.N.    |   DATE      |   DATE      |  UOM   |   LANDED    |   QUANTITY   ");
	Dsp_saverec ("  NUMBER    |   ACRONYM   | NO |     NUMBER    |     NUMBER    |  RAISED     | DELIVERED   |        |    COST     |              ");
	Dsp_saverec ("  [ REDRAW ]   [ NEXT SCREEN ]   [ PREV SCREEN ]   [ INPUT / END ]   ");

	suph_rec.hhbr_hash = inmr_rec.hhbr_hash;

	cc = find_rec (suph, &suph_rec, GTEQ, "r");

	while (!cc &&
		   (suph_rec.hhbr_hash == inmr_rec.hhbr_hash))
	{
		if ((suph_rec.ord_date >= local_rec.cutoffDate) &&
			 (strcmp (suph_rec.po_no, "               ") != 0))
		{
			sumr_rec.hhsu_hash = suph_rec.hhsu_hash;
			cc = find_rec (sumr2, &sumr_rec, EQUAL, "r");
			if (cc)
				file_err (cc, sumr, "DBFIND");

			inum_rec.hhum_hash	=	suph_rec.hhum_hash;
			cc = find_rec (inum, &inum_rec, COMPARISON, "r");
			PurCnvFct 	= (float) ((cc) ? 1.00 : inum_rec.cnv_fct);
			CnvFct	=	StdCnvFct / PurCnvFct;

			wsDouble	=	twodec (suph_rec.land_cost / CnvFct);
			wsFloat		=	twodec (suph_rec.rec_qty * CnvFct);
			wsLand		=	twodec (suph_rec.land_cost);

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
				"%s%s%s%s",
				sumr_rec.crd_no,
				suph_rec.br_no,
				suph_rec.po_no,
				suph_rec.grn_no
			);
			strcpy (sortRec [sortCnt].supplierNo,	sumr_rec.crd_no);
			strcpy (sortRec [sortCnt].acronym,		sumr_rec.acronym);
			strcpy (sortRec [sortCnt].branchNo,	 	suph_rec.br_no);
			strcpy (sortRec [sortCnt].poNo, 		suph_rec.po_no);
			strcpy (sortRec [sortCnt].grnNo, 		suph_rec.grn_no);
			strcpy (sortRec [sortCnt].uom, 			inum_rec.uom);
			sortRec [sortCnt].dateOrder		=	suph_rec.ord_date;
			sortRec [sortCnt].dateReceipt	= 	suph_rec.rec_date;
			sortRec [sortCnt].wsDouble		=	wsDouble;
			sortRec [sortCnt].wsFloat		=	wsFloat;
			/*
			 * Increment array counter.
			 */
			sortCnt++;
		
			if (local_rec.selectPoType [0] != 'A')
			{
				wsIdx = FindTableIndex (sumr_rec.crd_no, wsMax);
				if (wsIdx == wsMax)
				{
					wsMax++;
					strcpy (selTable [wsIdx].supplierNo, sumr_rec.crd_no);
					selTable [wsIdx].selDate = suph_rec.rec_date;
					selTable [wsIdx].selCost = wsLand;
				}
				else
				{
					if (suph_rec.rec_date > selTable [wsIdx].selDate)	
						selTable [wsIdx].selDate = suph_rec.rec_date;
					if (wsLand < selTable [wsIdx].selCost)	
						selTable [wsIdx].selCost = wsLand;
				}
			}
		}
		cc = find_rec (suph, &suph_rec, NEXT, "r");
	}

	/*
	 * Sort the array in item description order.
	 */
	qsort (sortRec, sortCnt, sizeof (struct SortStruct), SortFunc);
	
	for (i = 0; i < sortCnt; i++)
	{
		if (local_rec.selectPoType [0] == 'A')
			DisplayDataLine (i);

		if (local_rec.selectPoType [0] == 'M')
		{
			wsIdx = FindTableIndex (sortRec [i].supplierNo, wsMax);
			if (wsIdx < wsMax)
			{
				if (sortRec [i].dateReceipt == selTable [wsIdx].selDate)	
					DisplayDataLine (i);
			}
		}
		if (local_rec.selectPoType [0] == 'L')
		{
			wsIdx = FindTableIndex (sortRec [i].supplierNo, wsMax);
			if (wsIdx < wsMax)
			{
				if (sortRec [i].wsDouble == selTable [wsIdx].selCost)	
					DisplayDataLine (i);
			}
		}
	}
	/*
	 *	Free up the array memory
	 */
	ArrDelete (&sortDetails);
	
	Dsp_srch ();
	Dsp_close ();
}

void
DisplayDataLine (
	int		lineNo)
{
	char	wsOrdDate [11];
	char	wsRecDate [11];

	strcpy (wsOrdDate, DateToString (sortRec [lineNo].dateOrder));
	strcpy (wsRecDate, DateToString (sortRec [lineNo].dateReceipt));

	sprintf 
	(
		err_str,
		"   %6.6s   ^E  %10.10s ^E %2.2s ^E%15.15s^E%15.15s^E %-10.10s  ^E %-10.10s  ^E  %4.4s  ^E %11.2f ^E %9.2f ",
		sortRec [lineNo].supplierNo,
		sortRec [lineNo].acronym,
		sortRec [lineNo].branchNo,
		sortRec [lineNo].poNo,
		sortRec [lineNo].grnNo,
		wsOrdDate,
		wsRecDate,
		sortRec [lineNo].uom,
		sortRec [lineNo].wsDouble,
		sortRec [lineNo].wsFloat
	);

	Dsp_saverec (err_str);
}

int
FindTableIndex (
	char	*supplierNo, 
	int 	wsMax)
{
	int		wsIdx = 0;

	while (wsIdx < wsMax)
	{
		if (strncmp (selTable [wsIdx].supplierNo, supplierNo, 6) == 0)
			return (wsIdx);
		wsIdx++;
	}
	return (wsIdx);
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
int
heading (
 int scn)
{
	if (restart) 
		return (EXIT_SUCCESS);

	swide ();
	if (scn != cur_screen)
		scn_set (scn);
	clear ();
	rv_pr (ML (mlPoMess139), (ScreenWidth -40)/2,0,1);
	line_at (1,0, ScreenWidth);
	line_at (20,0, ScreenWidth);

	box (0,3,ScreenWidth,3);

	print_at (21,0, ML (mlStdMess038), comm_rec.co_no,comm_rec.co_name);
	print_at (22,0, ML (mlStdMess039),comm_rec.est_no,comm_rec.est_name);

	line_cnt = 0;
	scn_write (scn);
    return (EXIT_SUCCESS);
}

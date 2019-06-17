/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_rollsim.c,v 5.5 2002/07/17 09:57:59 scott Exp $
|  Program Name  : (sk_rollsim.c)
|  Program Desc  : (Standard Cost Rollup Simulation Program)
|---------------------------------------------------------------------|
|  Author        : Aroha Merrilees   Date Written  : 30/03/94         |
|---------------------------------------------------------------------|
| $Log: sk_rollsim.c,v $
| Revision 5.5  2002/07/17 09:57:59  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.4  2001/12/14 04:25:47  scott
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_rollsim.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_rollsim/sk_rollsim.c,v 5.5 2002/07/17 09:57:59 scott Exp $";

#include	<pslscr.h>
#include	<dsp_screen.h>
#include	<dsp_process.h>
#include	<number.h>
#include	<get_lpno.h>
#include	<ml_std_mess.h>
#include	<ml_sk_mess.h>
#include	<arralloc.h>

#define		DETAIL		 (local_rec.detSum [0] == 'D')

#include	"schema"

struct commRecord	comm_rec;
struct inmrRecord	inmr_rec;
struct inmrRecord	inmr2_rec;
struct bmmsRecord	bmms2_rec;
struct bmmsRecord	bmms_rec;
struct rghrRecord	rghr_rec;
struct rglnRecord	rgln_rec;
struct ineiRecord	inei_rec;
struct inumRecord	inum_rec;
struct rgrsRecord	rgrs_rec;

/*
 *	Structure for dynamic array,  for the rollupRec lines for qsort
 */
struct RollupStruct
{
	char	key 	[31];
	long	hhbrHash;
}	*rollupRec;
	DArray rollup_details;
	int	rollupCnt = 0;

int		RollupSort			(const	void *,	const void *);
	char	*data	= "data",
			*inmr2	= "inmr2";

FILE	*fout;
		
/* 
 * Global Variables 
 */

int		firstTime;
double	quantity;				/* Quantity Including Wastage */
struct {
	double	mat_cost;			/* Total Material Costs 	*/
	double	s_labour 	 [3];	/* Variable Labour Costs 	*/
	double	s_machine 	 [3];	/* Variable Machine Costs 	*/
	double	s_qc_check 	 [3];	/* Variable QC-Check Costs 	*/
	double	s_special 	 [3];	/* Variable Special Costs 	*/
	double	s_other 	 [3];	/* Variable Other Costs 	*/
	double	a_labour 	 [3];	/* Fixed Labour Costs	 	*/
	double	a_machine 	 [3];	/* Fixed Machine Costs	 	*/
	double	a_qc_check 	 [3];	/* Fixed QC-Check Costs 	*/
	double	a_special 	 [3];	/* Fixed Special Costs 		*/
	double	a_other 	 [3];	/* Fixed Other Costs 		*/
} reqRec;

/*
 * Local & Screen Structures.
 */
struct {
	char	startItem [17];
	char	startItemDesc [41];
	char	endItem [17];
	char	endItemDesc [41];
	int		bom_no;
	int		rtg_no;
	char	orderBy [2];
	char	orderByDesc [12];
	char	detSum [2];
	char	detSumDesc [8];
	char	newPage [2];
	char	newPageDesc [4];
	char 	dummy [11];

	char	stdUom [5];
	char	altUom [5];
	float	stdBatch;

	char	systemDate [11];
	int		printerNo;
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "startItem",	 4, 2, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "           ",
		" ", " ", " Start Item                 ", "Start Manufactured Item - Full Search Available",
		YES, NO,  JUSTLEFT, "", "", local_rec.startItem},
	{1, LIN, "startItemDesc",	 5, 2, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "           ",
		" ", " ", " Start Item Description     ", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.startItemDesc},
	{1, LIN, "endItem",	 6, 2, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "           ",
		" ", "~~~~~~~~~~~~~~~~", " End Item                   ", "End Manufactured Item - Full Search Available",
		YES, NO,  JUSTLEFT, "", "", local_rec.endItem},
	{1, LIN, "endItemDesc",	 7, 2, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "           ",
		" ", " ", " End Item Description       ", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.endItemDesc},
	{1, LIN, "bom_no",	9, 2, INTTYPE,
		"NNNNN", "          ",
		" ", "1", " BOM Number                 ", "Default BOM Number.",
		YES, NO, JUSTRIGHT, "0", "32767", (char *)&local_rec.bom_no},
	{1, LIN, "rtg_no",	10, 2, INTTYPE,
		"NNNNN", "          ",
		" ", "1", " Routing Number             ", "Default Routing Number.",
		YES, NO, JUSTRIGHT, "0", "32767", (char *)&local_rec.rtg_no},
	{1, LIN, "orderBy",	12, 2, CHARTYPE,
		"U", "          ",
		" ", "I", " Order By                   ", "Order By (I)tem Number or (S)ource Code. Default - Item Number.",
		YES, NO, JUSTLEFT, "IS", "", local_rec.orderBy},
	{1, LIN, "orderByDesc",	12, 33, CHARTYPE,
		"UUUUUUUUUUU", "          ",
		" ", "", "", "",
		NA, NO, JUSTRIGHT, "", "", local_rec.orderByDesc},
	{1, LIN, "detSum",	14, 2, CHARTYPE,
		"U", "          ",
		" ", "D", " Detail/Summary             ", " (D)etails or (S)ummary. Default - Detail.",
		YES, NO, JUSTLEFT, "DS", "", local_rec.detSum},
	{1, LIN, "detSumDesc",	14, 33, CHARTYPE,
		"AAAAAAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.detSumDesc},
	{1, LIN, "newPage",	15, 2, CHARTYPE,
		"U", "          ",
		" ", "N", " New Page Per Parent        ", " (Y)es or (N)o. Default - No.",
		YES, NO, JUSTLEFT, "NY", "", local_rec.newPage},
	{1, LIN, "newPageDesc",	15, 33, CHARTYPE,
		"UUU", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.newPageDesc},
	{1, LIN, "printerNo",	17, 2, INTTYPE,
		"NN", "          ",
		" ", "1", " Printer Number             ", "Full Search Available.",
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.printerNo},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*
 * Function Declarations
 */
void	shutdown_prog 			 (void);
void 	OpenDB 					 (void);
void 	CloseDB 				 (void);
int  	spec_valid 				 (int);
void 	ProcStdRollup 			 (void);
double 	UpdateMfgItem 			 (long, int, float, float);
void 	InitStruct 				 (void);
double 	CalcMaterials 			 (long, int, float);
float 	GetUom 					 (long);
void 	CalcResources 			 (long, float);
void 	PrintMaterial 			 (int, double, int);
void 	PrintMfgTotal 			 (int, double);
void 	PrintTotal 				 (double);
void 	InitOutput 				 (void);
void 	PrintHeading 			 (void);
void 	ParentHeading 			 (void);
void 	ComponentHeading 		 (void);
int  	heading 				 (int);
		
extern	int		manufacturingSrch;
extern	int		TruePosition;
	
/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int argc,
 char * argv [])
{
	TruePosition		=	TRUE;
	manufacturingSrch	=	TRUE;

	SETUP_SCR (vars);

	OpenDB ();

	init_scr ();			/*  sets terminal from termcap	*/
	set_tty ();
	set_masks ();			/*  setup print using masks	*/
	init_vars (1);			/*  set default values		*/

	while (prog_exit == 0)
	{
		entry_exit 	= FALSE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;
		crsr_on ();

		heading (1);
		entry (1);
		if (restart || prog_exit)
			continue;

		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
			continue;

		clear ();
		crsr_off ();
		fflush (stdout);
		InitOutput ();

		ProcStdRollup ();

		fprintf (fout, ".EOF\n");
		pclose (fout);
	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}
	
/*
 * Program exit sequence
 */
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*
 * Open data base files .
 */
void
OpenDB (void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	abc_alias (inmr2, inmr);

	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (inmr2,inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (bmms, bmms_list, BMMS_NO_FIELDS, "bmms_id_no");
	open_rec (rghr, rghr_list, RGHR_NO_FIELDS, "rghr_id_no");
	open_rec (rgln, rgln_list, RGLN_NO_FIELDS, "rgln_id_no");
	open_rec (inei, inei_list, INEI_NO_FIELDS, "inei_id_no");
	open_rec (inum, inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	open_rec (rgrs, rgrs_list, RGRS_NO_FIELDS, "rgrs_hhrs_hash");
}

/*
 * Close data base files.
 */
void
CloseDB (void)
{
	abc_fclose (inmr);
	abc_fclose (bmms);
	abc_fclose (rghr);
	abc_fclose (rgln);
	abc_fclose (inei);
	abc_fclose (inum);
	abc_fclose (rgrs);

	SearchFindClose ();
	abc_dbclose (data);
}

int
spec_valid (
	int		field)
{
	if (LCHECK ("startItem"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.startItemDesc, ML ("First Manufactured Item"));
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
			clear_mess ();
			sprintf (local_rec.startItemDesc, "%-40.40s", " ");
			DSP_FLD ("startItemDesc");
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();

		/*
		 * Check if item is a manufactured item. 
		 */
		if (strcmp (inmr_rec.source, "BP") &&
			strcmp (inmr_rec.source, "BM") &&
			strcmp (inmr_rec.source, "MC") &&
			strcmp (inmr_rec.source, "MP"))
		{
			print_mess (ML (mlSkMess554));
			sleep (sleepTime);
			clear_mess ();
			sprintf (local_rec.startItemDesc, "%-40.40s", " ");
			DSP_FLD ("startItemDesc");
			return (EXIT_FAILURE);
		}

		if (prog_status != ENTRY &&
			strcmp (local_rec.startItem, local_rec.endItem) > 0)
		{
			print_mess (ML (mlStdMess017));
			sleep (sleepTime);
			clear_mess ();
			sprintf (local_rec.startItemDesc, "%-40.40s", " ");
			DSP_FLD ("startItemDesc");
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.startItemDesc, inmr_rec.description);
		DSP_FLD ("startItemDesc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("endItem"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.endItemDesc, ML ("Last Manufactured Item"));
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
			clear_mess ();
			sprintf (local_rec.endItemDesc, "%-40.40s", " ");
			DSP_FLD ("endItemDesc");
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();

		/*
		 * Check if item is a manufactured item.
		 */
		if (strcmp (inmr_rec.source, "BP") &&
			strcmp (inmr_rec.source, "BM") &&
			strcmp (inmr_rec.source, "MC") &&
			strcmp (inmr_rec.source, "MP"))
		{
			print_mess (ML (mlSkMess554));
			sleep (sleepTime);
			clear_mess ();
			sprintf (local_rec.endItemDesc, "%-40.40s", " ");
			DSP_FLD ("startItemDesc");
			return (EXIT_FAILURE);
		}

		if (strcmp (local_rec.startItem, local_rec.endItem) > 0)
		{
			print_mess (ML (mlStdMess017));
			sleep (sleepTime);
			clear_mess ();
			sprintf (local_rec.endItemDesc, "%-40.40s", " ");
			DSP_FLD ("endItemDesc");
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.endItemDesc, inmr_rec.description);
		DSP_FLD ("endItemDesc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("bom_no"))
	{
		if (local_rec.bom_no > 32767)
		{
			print_mess (ML (mlSkMess216));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("rtg_no"))
	{
		if (local_rec.rtg_no > 32767)
		{
			print_mess (ML (mlSkMess217));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("orderBy"))
	{
		if (local_rec.orderBy [0] == 'I')
			strcpy (local_rec.orderByDesc, ML ("Item Number"));
		else
			strcpy (local_rec.orderByDesc, ML ("Source Code"));

		DSP_FLD ("orderByDesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("detSum"))
	{
		if (local_rec.detSum [0] == 'D')
			strcpy (local_rec.detSumDesc, ML ("Detail "));
		else
			strcpy (local_rec.detSumDesc, ML ("Summary"));

		DSP_FLD ("detSumDesc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("newPage"))
	{
		if (local_rec.newPage [0] == 'Y')
			strcpy (local_rec.newPageDesc, ML ("Yes"));
		else
			strcpy (local_rec.newPageDesc, ML ("No "));

		DSP_FLD ("newPageDesc");
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

	return (EXIT_SUCCESS);
}

/*
 * Process selected items. If the item is a manufactured item,
 * calculate the standard cost.                              
 */
void
ProcStdRollup (void)
{
	int		i;
	long	hhbrHash;

	/*
	 * Allocate the initial array.
	 */
	ArrAlloc (&rollup_details, &rollupRec,sizeof (struct RollupStruct),50);
	rollupCnt = 0;

	strcpy (inmr_rec.co_no, comm_rec.co_no);
	strcpy (inmr_rec.item_no, local_rec.startItem);
	cc = find_rec (inmr, &inmr_rec, GTEQ, "r");
	/* Read for all the items within the users selection. */
	while (!cc &&
		strcmp (inmr_rec.item_no, local_rec.startItem) >= 0 &&
		strcmp (inmr_rec.item_no, local_rec.endItem) <= 0)
	{
		/*
		 * Calculate the standard cost if the items is 
		 * a manufactured item.                       
		 */
		if (!strcmp (inmr_rec.source, "BP") ||
			!strcmp (inmr_rec.source, "BM") ||
			!strcmp (inmr_rec.source, "MC") ||
			!strcmp (inmr_rec.source, "MP"))
		{
			/*
			 * Check the array size before adding new element.
			 */
			if (!ArrChkLimit (&rollup_details, rollupRec, rollupCnt))
				sys_err ("ArrChkLimit (rollupRec)", ENOMEM, PNAME);

			sprintf 
			 (
				rollupRec [rollupCnt].key, 
				"%2.2s%-16.16s%010ld",
				 (local_rec.orderBy [0] == 'I') ? "  " : inmr_rec.source,
				inmr_rec.item_no,
				inmr_rec.hhbr_hash
			);
			rollupRec [rollupCnt].hhbrHash	=	inmr_rec.hhbr_hash;
			rollupCnt++;
		}
		cc = find_rec (inmr, &inmr_rec, NEXT, "r");
	}
	/*
	 * Sort the array in item description order.
	 */
	qsort (rollupRec, rollupCnt, sizeof (struct RollupStruct), RollupSort);

	PrintHeading ();

	firstTime = TRUE;

	for (i = 0; i < rollupCnt; i++)
	{
		hhbrHash	= rollupRec [i].hhbrHash;
		strcpy (inei_rec.est_no, comm_rec.est_no);
		inei_rec.hhbr_hash = hhbrHash;
		cc = find_rec (inei, &inei_rec, EQUAL, "r");
		if (cc)
			inei_rec.std_batch = 0.00;

		UpdateMfgItem (hhbrHash, 1, inei_rec.std_batch, 1.00);
	}
	/*
	 *	Free up the array memory
	 */
	ArrDelete (&rollup_details);
}

/*
 * Calculate the total standard cost of the    
 * manufactured item, down to the lowest level.
 */
double 
UpdateMfgItem (
	long 	hhbrHash, 
	int 	levelNo, 
	float 	requiredQty, 
	float 	cnvFct)
{
	int		i;
	double	materialCost = 0.00;

	strcpy (bmms_rec.co_no, comm_rec.co_no);
	bmms_rec.hhbr_hash 	= hhbrHash;
	bmms_rec.alt_no 	= local_rec.bom_no;
	bmms_rec.line_no 	= 0;
	cc = find_rec (bmms, &bmms_rec, GTEQ, "r");
	if (cc || strcmp (bmms_rec.co_no, comm_rec.co_no) ||
			bmms_rec.hhbr_hash != hhbrHash ||
			bmms_rec.alt_no != local_rec.bom_no)
	{
		return (-1.00);
	}

	strcpy (rghr_rec.co_no, comm_rec.co_no);
	strcpy (rghr_rec.br_no, comm_rec.est_no);
	rghr_rec.hhbr_hash 	= hhbrHash;
	rghr_rec.alt_no 	= local_rec.rtg_no;
	cc = find_rec (rghr, &rghr_rec, COMPARISON, "r");
	if (cc)
		return (-1.00);

	/* 
	 * Find item details 
	 */
	inmr_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec (inmr2, &inmr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, inmr2, "DBFIND");

	if (levelNo == 1)			/* Item is a Parent Manufacturing Item */
	{
		/* 
		 * Find standard UOM 
		 */
		inum_rec.hhum_hash = inmr_rec.std_uom;
		cc = find_rec (inum, &inum_rec, EQUAL, "r");
		if (cc)
			strcpy (inum_rec.uom, " ");

		strcpy (local_rec.stdUom, inum_rec.uom);

		/* 
		 * Find alternate UOM 
		 */
		inum_rec.hhum_hash = inmr_rec.alt_uom;
		cc = find_rec (inum, &inum_rec, EQUAL, "r");
		if (cc)
			strcpy (inum_rec.uom, "UNKN");

		strcpy (local_rec.altUom, inum_rec.uom);

		/* 
		 * Find standard batch size 
		 */
		strcpy (inei_rec.est_no, comm_rec.est_no);
		inei_rec.hhbr_hash = inmr_rec.hhbr_hash;
		cc = find_rec (inei, &inei_rec, EQUAL, "r");
		if (cc)
			inei_rec.std_batch = 0.00;
		local_rec.stdBatch = inei_rec.std_batch;

		/* Print new page (if necessary), and headings. */
		if (!firstTime && local_rec.newPage [0] == 'Y')
			fprintf (fout, ".PA");
		ParentHeading ();
		ComponentHeading ();
	}
	else
		if (DETAIL)
			PrintMaterial (levelNo - 1, materialCost, TRUE);

	dsp_process ("Item Number :", inmr_rec.item_no);

	materialCost = CalcMaterials (hhbrHash, levelNo, cnvFct);

	InitStruct ();

	CalcResources (hhbrHash, requiredQty);

	/* 
	 * Find item details 
	 */
	inmr_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec (inmr2, &inmr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, inmr2, "DBFIND");

	inei_rec.hhbr_hash = inmr_rec.hhbr_hash;
	strcpy (inei_rec.est_no, comm_rec.est_no);
	cc = find_rec (inei, &inei_rec, COMPARISON, "r");
	if (cc)
		inei_rec.std_batch = 1.00;

	if (levelNo == 1)
		PrintTotal (materialCost);
	else
		reqRec.mat_cost = materialCost;

	for (i = 0; i < 3; i ++)
	{
		materialCost += reqRec.s_labour [i];
		materialCost += reqRec.s_machine [i];
		materialCost += reqRec.s_qc_check [i];
		materialCost += reqRec.s_special [i];
		materialCost += reqRec.s_other [i];
		materialCost += reqRec.a_labour [i];
		materialCost += reqRec.a_machine [i];
		materialCost += reqRec.a_qc_check [i];
		materialCost += reqRec.a_special [i];
		materialCost += reqRec.a_other [i];
	}

	if (firstTime)
		firstTime = FALSE;
	
	return (materialCost);
}
/*
 * Initialise the required quantity and batch quantity record variables for 
 * standard and actual breakdown costs for setup, run and clean times.
 */
void
InitStruct (void)
{
	int		i;

	for (i = 0; i < 3; i ++)
	{
		reqRec.s_labour 	 [i]	= 0.00;
		reqRec.s_machine 	 [i]	= 0.00;
		reqRec.s_qc_check 	 [i]	= 0.00;
		reqRec.s_special 	 [i]	= 0.00;
		reqRec.s_other 		 [i]	= 0.00;
		reqRec.a_labour 	 [i]	= 0.00;
		reqRec.a_machine 	 [i]	= 0.00;
		reqRec.a_qc_check 	 [i]	= 0.00;
		reqRec.a_special 	 [i]	= 0.00;
		reqRec.a_other 		 [i]	= 0.00;
	}
}

/*
 * Calculates the total material costs at all levels
 * of the manufactured item passed.                 
 * Returns the total material cost (includes lower  
 * manufactured items total costs (both material and 
 * resource costs).                                 
 */
double 
CalcMaterials (
	long	hhbrHash, 
	int		levelNo, 
	float	cnvFct)
{
	double	totalMatrCost 	= 0.00,
			tmpValue 		= 0.00,
			stdCost 		= 0.00;

	float	tmpCnvFct		= 0.00,
			requiredQty		= 0.00;

	int		lineNo			= 0,
			flag 			= FALSE;

	strcpy (bmms_rec.co_no, inmr_rec.co_no);
	bmms_rec.hhbr_hash = hhbrHash;
	bmms_rec.alt_no = local_rec.bom_no;
	bmms_rec.line_no = 0;
	cc = find_rec (bmms, &bmms_rec, GTEQ, "r");
	while (!cc &&						 /* calculate material costs */
		!strcmp (bmms_rec.co_no, inmr_rec.co_no) &&
		bmms_rec.hhbr_hash == hhbrHash &&
		bmms_rec.alt_no == local_rec.bom_no)
	{
		flag = FALSE;
		inmr2_rec.hhbr_hash = bmms_rec.mabr_hash;
		cc = find_rec (inmr2, &inmr2_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, inmr2, "DBFIND");

		/*
		 * Manufactued item, need to calculate component costs.
		 */
		if (!strcmp (inmr2_rec.source, "BP") ||
			!strcmp (inmr2_rec.source, "BM") ||
			!strcmp (inmr2_rec.source, "MC") ||
			!strcmp (inmr2_rec.source, "MP"))
		{
			lineNo = bmms_rec.line_no;

			/* 
			 * Calculated required quantity in std uom 
			 */
			tmpCnvFct = GetUom (bmms_rec.uom);
			requiredQty = bmms_rec.matl_wst_pc;
			requiredQty += 100.00;
			requiredQty /= 100.00;
			requiredQty *= bmms_rec.matl_qty;
			requiredQty /= tmpCnvFct; /* divid by cnvFct to get std uom qty */

			strcpy (inei_rec.est_no, comm_rec.est_no);
			inei_rec.hhbr_hash = bmms_rec.mabr_hash;
			cc = find_rec (inei, &inei_rec, EQUAL, "r");
			if (cc)
				inei_rec.std_batch = 1.00;

			tmpCnvFct = requiredQty;
			tmpCnvFct /= inei_rec.std_batch;

			stdCost =	UpdateMfgItem 
						 (
							bmms_rec.mabr_hash,
							levelNo + 1,
							requiredQty,
							tmpCnvFct
						);
			if (stdCost < 0.00)
				flag = FALSE;
			else
				flag = TRUE;

			strcpy (bmms_rec.co_no, comm_rec.co_no);
			bmms_rec.hhbr_hash = hhbrHash;
			bmms_rec.alt_no = local_rec.bom_no;
			bmms_rec.line_no = lineNo;
			cc = find_rec (bmms, &bmms_rec, COMPARISON, "r");
			if (cc)
				file_err (cc, bmms, "DBFIND");

			inmr2_rec.hhbr_hash = bmms_rec.mabr_hash;
			cc = find_rec (inmr2, &inmr2_rec, COMPARISON, "r");
			if (cc)
				file_err (cc, inmr2, "DBFIND");

			strcpy (inei_rec.est_no, comm_rec.est_no);
			inei_rec.hhbr_hash = bmms_rec.mabr_hash;
			cc = find_rec (inei, &inei_rec, COMPARISON, "r");
			if (cc)
				inei_rec.std_batch = 1.00;

			if (flag)
			{
				inei_rec.std_cost = stdCost;
				inei_rec.std_cost /= requiredQty;
				inei_rec.std_cost *= inmr2_rec.outer_size;
			}
		}
		else
		{
			/*
			 * Calculate the std cost for this item.
			 */
			inei_rec.hhbr_hash = bmms_rec.mabr_hash;
			strcpy (inei_rec.est_no, comm_rec.est_no);
			cc = find_rec (inei, &inei_rec, COMPARISON, "r");
			if (cc)
				inei_rec.std_cost = 0.00;
		}

		bmms_rec.matl_wst_pc += 100.00;
		bmms_rec.matl_wst_pc /= 100.00;
		quantity = (double) bmms_rec.matl_qty * (double) bmms_rec.matl_wst_pc;
		quantity *= (double) cnvFct;

		if (!flag)
		{
			tmpCnvFct = GetUom (bmms_rec.uom);
			inei_rec.std_cost /= tmpCnvFct;

			tmpValue = out_cost (inei_rec.std_cost, inmr2_rec.outer_size);
			tmpValue *= quantity;
		}
		else
			tmpValue = stdCost;

		if (DETAIL || levelNo == 1)
		{
			inum_rec.hhum_hash	=	bmms_rec.uom;
			cc = find_rec (inum, &inum_rec, EQUAL, "r");
			if (cc)
				strcpy (inum_rec.uom, " ");

			if (!flag)
				PrintMaterial (levelNo, tmpValue, FALSE);
			else
				PrintMfgTotal (levelNo, tmpValue);
		}

		totalMatrCost += (double) tmpValue;

		cc = find_rec (bmms, &bmms_rec, NEXT, "r");
	}

	return (totalMatrCost);
}

float 
GetUom (
	 long	hhumHash)
{
	char	stdGroup [21],
			altGroup [21];

	number	stdCnvFct,
			altCnvFct,
			cnvFct,
			result,
			uomCFactor;

	/*
	 * Get the UOM conversion factor
	 */
	inum_rec.hhum_hash	=	inmr2_rec.alt_uom;
	cc = find_rec (inum, &inum_rec, EQUAL, "r");
	if (cc)
		file_err (cc, inum, "DBFIND");

	sprintf (altGroup, "%-20.20s", inum_rec.uom_group);

	/*
	 * converts a float to arbitrary precision number defined as number. 
	 */
	NumFlt (&altCnvFct, inum_rec.cnv_fct);

	inum_rec.hhum_hash	=	inmr2_rec.std_uom;
	cc = find_rec (inum, &inum_rec, EQUAL, "r");
	if (cc)
		file_err (cc, inum, "DBFIND");

	sprintf (stdGroup, "%-20.20s", inum_rec.uom_group);

	/*
	 * converts a float to arbitrary precision number defined as number.
	 */
	NumFlt (&stdCnvFct, inum_rec.cnv_fct);

	inum_rec.hhum_hash	=	hhumHash;
	cc = find_rec (inum, &inum_rec, EQUAL, "r");
	if (cc)
		file_err (cc, inum, "DBFIND");

	/*
	 * converts a float to arbitrary precision number defined as number.
	 */
	NumFlt (&cnvFct, inum_rec.cnv_fct);

	/*
	 * a function that divides one number by another and places
	 * the result in another number defined variable          
	 */
	if (strcmp (altGroup, inum_rec.uom_group))
		NumDiv (&stdCnvFct, &cnvFct, &result);
	else
	{
		NumFlt (&uomCFactor, inmr2_rec.uom_cfactor);
		NumDiv (&altCnvFct, &cnvFct, &result);
		NumMul (&result, &uomCFactor, &result);
	}

	/*
	 * converts a arbitrary precision number to a float.
	 */
	return (NumToFlt (&result));
}

/*
 * Calculates the total resource costs at all levels
 * of the manufactured item passed.                 
 * Returns the total resource cost for the item hash
 * passed.                                          
 */
void
CalcResources (
	long 	hhbrHash, 
	float 	requiredQty)
{
	/* 
	 * 0 - setup cost 1 - run cost 2 - clean cost 
	 */
	double	resourceCost [3]		=	{0.0,0.0,0.0},
			resourceOverhead [3]	=	{0.0,0.0,0.0};
	float	runTime;

	strcpy (inei_rec.est_no, comm_rec.est_no);
	inei_rec.hhbr_hash = hhbrHash;
	cc = find_rec (inei, &inei_rec, EQUAL, "r");
	if (cc)
		inei_rec.std_batch = 1.00;

	strcpy (rghr_rec.co_no, comm_rec.co_no);
	strcpy (rghr_rec.br_no, comm_rec.est_no);
	rghr_rec.hhbr_hash 	= hhbrHash;
	rghr_rec.alt_no 	= local_rec.rtg_no;
	cc = find_rec (rghr, &rghr_rec, COMPARISON, "r");
	if (!cc)
	{
		rgln_rec.hhgr_hash 	= rghr_rec.hhgr_hash;
		rgln_rec.seq_no 	= 0;
		cc = find_rec (rgln, &rgln_rec, GTEQ, "r");
		while (!cc &&
			rgln_rec.hhgr_hash == rghr_rec.hhgr_hash)
		{
			/* 
			 * Divide the fixed overhead cost by three for 
		 	 * the setup, run and clean cost calculations 
			 */
			rgln_rec.ovhd_fix /= 3;

			/* 
			 * Calculate run time for the required quantity 
			 */
			runTime  = (float) (rgln_rec.run);
			runTime *= requiredQty;
			runTime /= inei_rec.std_batch;

			/* 
			 * Total standard costs for the required quantity. 
			 */
			resourceCost [0] = (double) rgln_rec.setup * rgln_rec.rate;
			resourceCost [0] /= 60.00;
			resourceCost [0] *= (double) rgln_rec.qty_rsrc;
			resourceCost [1] = (double) runTime * rgln_rec.rate;
			resourceCost [1] /= 60.00;
			resourceCost [1] *= (double) rgln_rec.qty_rsrc;
			resourceCost [2] = (double) rgln_rec.clean * rgln_rec.rate;
			resourceCost [2] /= 60.00;
			resourceCost [2] *= (double) rgln_rec.qty_rsrc;

			/* 
			 * Total overhead costs for the required quantity. 
			 */
			resourceOverhead [0] = (double) rgln_rec.setup * rgln_rec.ovhd_var;
			resourceOverhead [0] /= 60.00;
			resourceOverhead [0] += (double) rgln_rec.ovhd_fix;
			resourceOverhead [0] *= (double) rgln_rec.qty_rsrc;
			resourceOverhead [1] = (double) runTime * rgln_rec.ovhd_var;
			resourceOverhead [1] /= 60.00;
			resourceOverhead [1] += (double) rgln_rec.ovhd_fix;
			resourceOverhead [1] *= (double) rgln_rec.qty_rsrc;
			resourceOverhead [2] = (double) rgln_rec.clean * rgln_rec.ovhd_var;
			resourceOverhead [2] /= 60.00;
			resourceOverhead [2] += (double) rgln_rec.ovhd_fix;
			resourceOverhead [2] *= (double) rgln_rec.qty_rsrc;

			/* 
			 * Check resource type, and add to appropriate total cost 
			 */
			cc = find_hash (rgrs, &rgrs_rec, EQUAL, "r", rgln_rec.hhrs_hash);
			if (!cc)
			{
				switch (rgrs_rec.type [0])
				{
				case	'L': /* Labour */
					reqRec.s_labour [0] += DOLLARS (resourceCost [0]);
					reqRec.a_labour [0] += DOLLARS (resourceOverhead [0]);
					reqRec.s_labour [1] += DOLLARS (resourceCost [1]);
					reqRec.a_labour [1] += DOLLARS (resourceOverhead [1]);
					reqRec.s_labour [2] += DOLLARS (resourceCost [2]);
					reqRec.a_labour [2] += DOLLARS (resourceOverhead [2]);
					break;
				case	'M': /* Machine */
					reqRec.s_machine [0] += DOLLARS (resourceCost [0]);
					reqRec.a_machine [0] += DOLLARS (resourceOverhead [0]);
					reqRec.s_machine [1] += DOLLARS (resourceCost [1]);
					reqRec.a_machine [1] += DOLLARS (resourceOverhead [1]);
					reqRec.s_machine [2] += DOLLARS (resourceCost [2]);
					reqRec.a_machine [2] += DOLLARS (resourceOverhead [2]);
					break;
				case	'Q': /* QC-Check */
					reqRec.s_qc_check [0] += DOLLARS (resourceCost [0]);
					reqRec.a_qc_check [0] += DOLLARS (resourceOverhead [0]);
					reqRec.s_qc_check [1] += DOLLARS (resourceCost [1]);
					reqRec.a_qc_check [1] += DOLLARS (resourceOverhead [1]);
					reqRec.s_qc_check [2] += DOLLARS (resourceCost [2]);
					reqRec.a_qc_check [2] += DOLLARS (resourceOverhead [2]);
					break;
				case	'S': /* Special */
					reqRec.s_special [0] += DOLLARS (resourceCost [0]);
					reqRec.a_special [0] += DOLLARS (resourceOverhead [0]);
					reqRec.s_special [1] += DOLLARS (resourceCost [1]);
					reqRec.a_special [1] += DOLLARS (resourceOverhead [1]);
					reqRec.s_special [2] += DOLLARS (resourceCost [2]);
					reqRec.a_special [2] += DOLLARS (resourceOverhead [2]);
					break;
				case	'O': /* Other */
					reqRec.s_other [0] += DOLLARS (resourceCost [0]);
					reqRec.a_other [0] += DOLLARS (resourceOverhead [0]);
					reqRec.s_other [1] += DOLLARS (resourceCost [1]);
					reqRec.a_other [1] += DOLLARS (resourceOverhead [1]);
					reqRec.s_other [2] += DOLLARS (resourceCost [2]);
					reqRec.a_other [2] += DOLLARS (resourceOverhead [2]);
					break;
				}
			}
			cc = find_rec (rgln, &rgln_rec, NEXT, "r");
		}
	}
}

void
PrintMaterial (
	int 	levelNo, 
	double 	materialCost, 
	int 	STAR)
{
	dsp_process ("Item Number :",		inmr2_rec.item_no);

	fprintf (fout, "|%s%2d",			 (STAR) ? "*" : " ", levelNo);
	fprintf (fout, "|%-16.16s",			inmr2_rec.item_no);
	fprintf (fout, "|%-40.40s ",		inmr2_rec.description);
	fprintf (fout, "|%7.1f",			inmr2_rec.outer_size);
	if (STAR)
	{
		fprintf (fout, "|%-4.4s",		" ");
		fprintf (fout, " %-14.14s",		" ");
		fprintf (fout, " %-10.10s",		" ");
		fprintf (fout, " %-10.10s ",	" ");
	}
	else
	{
		fprintf (fout, "|%-4.4s",		inum_rec.uom);
		fprintf (fout, "|%14.6f",		quantity);
		fprintf (fout, "|%10.2f",		inei_rec.std_cost);
		fprintf (fout, "|%10.2f|",		materialCost);
	}
	fprintf (fout, "%43s|\n",			" ");
}

void
PrintMfgTotal (
	int 	levelNo, 
	double 	materialCost)
{
	double	totalSetup 	= 0.00,
			totalRun 	= 0.00,
			totalClean 	= 0.00,
			totalCost 	= 0.00;

	/* 
	 * Total setup costs 
	 */
	totalSetup 	+= 	reqRec.a_labour 	 [0] + reqRec.a_machine 	 [0] +
					reqRec.a_qc_check 	 [0] + reqRec.a_special 	 [0] +
					reqRec.a_other 		 [0] + reqRec.s_labour 	 [0] +
					reqRec.s_machine 	 [0] + reqRec.s_qc_check [0] +
					reqRec.s_special 	 [0] + reqRec.s_other 	 [0];

	/*  
	 * Total run costs 
	 */
	totalRun += 	reqRec.a_labour 	 [1] + reqRec.a_machine 	 [1] +
					reqRec.a_qc_check 	 [1] + reqRec.a_special 	 [1] +
					reqRec.a_other 		 [1] + reqRec.s_labour 	 [1] +
					reqRec.s_machine 	 [1] + reqRec.s_qc_check [1] +
					reqRec.s_special 	 [1] + reqRec.s_other 	 [1];

	/* 
	 * Total clean costs 
	 */
	totalClean += 	reqRec.a_labour 	 [2] + reqRec.a_machine 	 [2] +
					reqRec.a_qc_check 	 [2] + reqRec.a_special 	 [2] +
					reqRec.a_other 		 [2] + reqRec.s_labour 	 [2] +
					reqRec.s_machine 	 [2] + reqRec.s_qc_check [2] +
					reqRec.s_special 	 [2] + reqRec.s_other 	 [2];

	totalCost  = reqRec.mat_cost;
	totalCost += totalSetup;
	totalCost += totalRun;
	totalCost += totalClean;

	dsp_process ("Item Number :",	inmr_rec.item_no);

	fprintf (fout, "|*%2d",			levelNo);
	fprintf (fout, "|%-16.16s",		inmr2_rec.item_no);
	fprintf (fout, "|%-40.40s ",	inmr2_rec.description);
	fprintf (fout, "|%7.1f",		inmr2_rec.outer_size);
	fprintf (fout, "|%-4.4s",		inum_rec.uom);
	fprintf (fout, "|%14.6f",		quantity);
	fprintf (fout, "|%10.2f",		inei_rec.std_cost);
	fprintf (fout, "|%10.2f",		materialCost);
	fprintf (fout, "|%10.2f",		reqRec.mat_cost);
	fprintf (fout, "|%10.2f",		totalSetup);
	fprintf (fout, "|%10.2f",		totalRun);
	fprintf (fout, "|%10.2f|\n",	totalClean);
}

void
PrintTotal (
	double	materialCost)
{
	double	resourceDirect 	= 0.00,
			resourceFixed 	= 0.00,
			totalBatch 		= 0.00,
			totalSetup 		= 0.00,
			totalRun 		= 0.00,
			totalClean 		= 0.00;

	int		i;

	/* 
	 * Total setup costs 
	 */
	totalSetup = reqRec.a_labour 	 [0] + reqRec.a_machine 	 [0] +
				 reqRec.a_qc_check 	 [0] + reqRec.a_special 	 [0] +
				 reqRec.a_other 	 [0] + reqRec.s_labour 	 [0] +
				 reqRec.s_machine 	 [0] + reqRec.s_qc_check [0] +
				 reqRec.s_special 	 [0] + reqRec.s_other 	 [0];

	/* 
	 * Total run costs 
	 */
	totalRun = 	reqRec.a_labour 	 [1] + reqRec.a_machine 	 [1] +
				reqRec.a_qc_check 	 [1] + reqRec.a_special 	 [1] +
				reqRec.a_other 		 [1] + reqRec.s_labour 	 [1] +
				reqRec.s_machine 	 [1] + reqRec.s_qc_check [1] +
				reqRec.s_special 	 [1] + reqRec.s_other 	 [1];

	/* 
	 * Total clean costs 
	 */
	totalClean = 	reqRec.a_labour 	 [2] + reqRec.a_machine 	 [2] +
					reqRec.a_qc_check 	 [2] + reqRec.a_special 	 [2] +
					reqRec.a_other 		 [2] + reqRec.s_labour 	 [2] +
					reqRec.s_machine 	 [2] + reqRec.s_qc_check [2] +
					reqRec.s_special 	 [2] + reqRec.s_other 	 [2];

	for (i = 0; i < 3; i ++)
	{
		resourceDirect += reqRec.s_labour 	 [i];
		resourceDirect += reqRec.s_machine 	 [i];
		resourceDirect += reqRec.s_qc_check [i];
		resourceDirect += reqRec.s_special 	 [i];
		resourceDirect += reqRec.s_other 	 [i];

		resourceFixed += reqRec.a_labour 	 [i];
		resourceFixed += reqRec.a_machine 	 [i];
		resourceFixed += reqRec.a_qc_check 	 [i];
		resourceFixed += reqRec.a_special 	 [i];
		resourceFixed += reqRec.a_other 	 [i];
	}
	totalBatch = totalSetup + totalRun + totalClean + materialCost;

	fprintf (fout, "|%156s|\n",		" ");

	fprintf (fout, "|%69s",			" ");
	fprintf (fout, "PARENT ITEM TOTALS");
	fprintf (fout, "%69s|\n",		" ");

	fprintf (fout, "|-------------------");
	fprintf (fout, "--------------------");
	fprintf (fout, "--------------------");
	fprintf (fout, "--------------------");
	fprintf (fout, "--------------------");
	fprintf (fout, "--------------------");
	fprintf (fout, "--------------------");
	fprintf (fout, "-----------------|\n");

	fprintf (fout, "|%35s", " ");
	fprintf (fout, "|                  DIRECT/VARIABLE COSTS               ");
	fprintf (fout, "|                       FIXED COSTS                    ");
	fprintf (fout, "|  TOTAL   |\n");
	fprintf (fout, "|%35s", " ");
	fprintf (fout, "|  LABOUR  | MACHINE  | QC-CHECK | SPECIAL  |  OTHER   ");
	fprintf (fout, "|  LABOUR  | MACHINE  | QC-CHECK | SPECIAL  |  OTHER   ");
	fprintf (fout, "|   COST   |\n");

	fprintf (fout, "| Material Costs%20s", " ");
	fprintf (fout, "|%10s",			" ");
	fprintf (fout, "|%10s",			" ");
	fprintf (fout, "|%10s",			" ");
	fprintf (fout, "|%10s",			" ");
	fprintf (fout, "|%10s",			" ");
	fprintf (fout, "|%10s",			" ");
	fprintf (fout, "|%10s",			" ");
	fprintf (fout, "|%10s",			" ");
	fprintf (fout, "|%10s",			" ");
	fprintf (fout, "|%10s",			" ");
	fprintf (fout, "|%10.2f|\n",	materialCost);

	fprintf (fout, "| Setup Costs   %20s", " ");
	fprintf (fout, "|%10.2f",	reqRec.s_labour [0]);
	fprintf (fout, "|%10.2f",	reqRec.s_machine [0]);
	fprintf (fout, "|%10.2f",	reqRec.s_qc_check [0]);
	fprintf (fout, "|%10.2f",	reqRec.s_special [0]);
	fprintf (fout, "|%10.2f",	reqRec.s_other [0]);
	fprintf (fout, "|%10.2f",	reqRec.a_labour [0]);
	fprintf (fout, "|%10.2f",	reqRec.a_machine [0]);
	fprintf (fout, "|%10.2f",	reqRec.a_qc_check [0]);
	fprintf (fout, "|%10.2f",	reqRec.a_special [0]);
	fprintf (fout, "|%10.2f",	reqRec.a_other [0]);
	fprintf (fout, "|%10.2f|\n",totalSetup);

	fprintf (fout, "| Run Costs     %20s", " ");
	fprintf (fout, "|%10.2f",	reqRec.s_labour [1]);
	fprintf (fout, "|%10.2f",	reqRec.s_machine [1]);
	fprintf (fout, "|%10.2f",	reqRec.s_qc_check [1]);
	fprintf (fout, "|%10.2f",	reqRec.s_special [1]);
	fprintf (fout, "|%10.2f",	reqRec.s_other [1]);
	fprintf (fout, "|%10.2f",	reqRec.a_labour [1]);
	fprintf (fout, "|%10.2f",	reqRec.a_machine [1]);
	fprintf (fout, "|%10.2f",	reqRec.a_qc_check [1]);
	fprintf (fout, "|%10.2f",	reqRec.a_special [1]);
	fprintf (fout, "|%10.2f",	reqRec.a_other [1]);
	fprintf (fout, "|%10.2f|\n",totalRun);

	fprintf (fout, "| Clean Costs   %20s", " ");
	fprintf (fout, "|%10.2f",	reqRec.s_labour [2]);
	fprintf (fout, "|%10.2f",	reqRec.s_machine [2]);
	fprintf (fout, "|%10.2f",	reqRec.s_qc_check [2]);
	fprintf (fout, "|%10.2f",	reqRec.s_special [2]);
	fprintf (fout, "|%10.2f",	reqRec.s_other [2]);
	fprintf (fout, "|%10.2f",	reqRec.a_labour [2]);
	fprintf (fout, "|%10.2f",	reqRec.a_machine [2]);
	fprintf (fout, "|%10.2f",	reqRec.a_qc_check [2]);
	fprintf (fout, "|%10.2f",	reqRec.a_special [2]);
	fprintf (fout, "|%10.2f",	reqRec.a_other [2]);
	fprintf (fout, "|%10.2f|\n",totalClean);

	fprintf (fout, "|%156s|\n",		" ");

	fprintf (fout, "|-------------------");
	fprintf (fout, "--------------------");
	fprintf (fout, "--------------------");
	fprintf (fout, "--------------------");
	fprintf (fout, "--------------------");
	fprintf (fout, "--------------------");
	fprintf (fout, "--------------------");
	fprintf (fout, "-----------------|\n");

	fprintf (fout, "|%156s|\n",		" ");

	fprintf (fout, "|%-25.25s                    ",	" ");
	fprintf (fout, "| COST PER  PRICE CONV ");
	fprintf (fout, "|    COST PER UNIT     ");
	fprintf (fout, "|    COST PER BATCH    |%41.41s|\n", " ");

	fprintf (fout, "|%-25.25sMaterial            ",	" ");
	fprintf (fout, "|      %10.2f      ",
			 (materialCost / inei_rec.std_batch) * inmr_rec.outer_size);
	fprintf (fout, "|    %14.6f    ",
			materialCost / inei_rec.std_batch);
	fprintf (fout, "|      %10.2f      |%41.41s|\n",
			materialCost, " ");

	fprintf (fout, "|%-25.25sRescoure Direct     ",	" ");
	fprintf (fout, "|      %10.2f      ",
			 (resourceDirect / inei_rec.std_batch) * inmr_rec.outer_size);
	fprintf (fout, "|    %14.6f    ",
			resourceDirect / inei_rec.std_batch);
	fprintf (fout, "|      %10.2f      |%41.41s|\n",
			resourceDirect, " ");

	fprintf (fout, "|%-25.25sResource Fixed      ",	" ");
	fprintf (fout, "|      %10.2f      ",
			 (resourceFixed / inei_rec.std_batch) * inmr_rec.outer_size);
	fprintf (fout, "|    %14.6f    ",
			resourceFixed / inei_rec.std_batch);
	fprintf (fout, "|      %10.2f      |%41.41s|\n",
			resourceFixed, " ");

	fprintf (fout, "|%-25.25s                    ",	" ");
	fprintf (fout, "|      ----------      ");
	fprintf (fout, "|    --------------    ");
	fprintf (fout, "|      ----------      |%41.41s|\n", " ");

	fprintf (fout, "|%-25.25sTotal Cost          ",	" ");
	fprintf (fout, "|      %10.2f      ",
			 (totalBatch / inei_rec.std_batch) * inmr_rec.outer_size);
	fprintf (fout, "|    %14.6f    ",
			totalBatch / inei_rec.std_batch);
	fprintf (fout, "|      %10.2f      |%41.41s|\n",
			totalBatch, " ");

	fprintf (fout, "|%156s|\n",					" ");

	fprintf (fout, "====================");
	fprintf (fout, "====================");
	fprintf (fout, "====================");
	fprintf (fout, "====================");
	fprintf (fout, "====================");
	fprintf (fout, "====================");
	fprintf (fout, "====================");
	fprintf (fout, "==================\n");
}

/*
 * Initialize for Printer Output.
 */
void
InitOutput (void)
{
	dsp_screen (" Printing Standard Cost Rollup Simulation Report ",
			comm_rec.co_no, 
			comm_rec.co_name);

	/*
	 * Open pipe to pformat 
	 */
	if ((fout = popen ("pformat", "w")) == NULL)
		sys_err ("Error in opening pformat During (POPEN)", errno, PNAME);

	/*
	 * Initialize printer for output. 
	 */
	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (fout, ".LP%d\n", local_rec.printerNo);
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".10\n");
	fprintf (fout, ".L158\n");
}

/*
 * Headings for printed output.
 */
void
PrintHeading (void)
{
	if (DETAIL)
		fprintf (fout, ".E STANDARD COST ROLLUP DETAILED SIMULATION REPORT \n");
	else
		fprintf (fout, ".E STANDARD COST ROLLUP SUMMARY SIMULATION REPORT \n");
	fprintf (fout, ".E COMPANY : %s \n", clip (comm_rec.co_name));
	fprintf (fout, ".E BRANCH : %s \n", clip (comm_rec.est_name));
	fprintf (fout, ".E WAREHOUSE : %s \n", clip (comm_rec.cc_name));
	fprintf (fout, ".B1\n");
	fprintf (fout, ".E START ITEM : %-16.16s  END ITEM : %-16.16s\n",
								local_rec.startItem, local_rec.endItem);
	fprintf (fout, ".E BOM NUMBER : %5d  ROUTING NUMBER : %5d\n",
								local_rec.bom_no, local_rec.rtg_no);
	fprintf (fout, ".E ORDER BY : %s  NEW PAGE : %s\n",
								local_rec.orderByDesc, local_rec.newPageDesc);
	fprintf (fout, ".B1\n");
}
/*
 * Parent headings for output.
 */
void
ParentHeading (void)
{
	fprintf (fout, ".B1\n");

	fprintf (fout, "====================");
	fprintf (fout, "====================");
	fprintf (fout, "====================");
	fprintf (fout, "====================");
	fprintf (fout, "====================");
	fprintf (fout, "====================");
	fprintf (fout, "====================");
	fprintf (fout, "==================\n");

	fprintf (fout, "| PARENT NUMBER : %-16.16s%10s", inmr_rec.item_no, " ");
	fprintf (fout, "%-40.40s%73s|\n", inmr_rec.description, " ");

	fprintf (fout,
			"| STRENGTH      : %-5.5s%54s",
			inmr_rec.description + 35,
			" ");
	fprintf (fout, "PRICING CONV  : %7.1f%57s|\n", inmr_rec.outer_size, " ");

	fprintf (fout, "| DFLT BOM      : %5d%54s", inmr_rec.dflt_bom, " ");
	fprintf (fout, "DFLT RTG      : %5d%59s|\n", inmr_rec.dflt_rtg, " ");

	fprintf (fout, "| STD UOM       : %-4.4s%55s", local_rec.stdUom, " ");
	fprintf (fout, "ALT UOM       : %-4.4s%60s|\n", local_rec.altUom, " ");

	fprintf (fout, "| STD BATCH     : %14.6f%45s", local_rec.stdBatch, " ");
	fprintf (fout, "EOQ           : %14.6f%50s|\n", inmr_rec.eoq, " ");

	fprintf (fout, "|-------------------");
	fprintf (fout, "--------------------");
	fprintf (fout, "--------------------");
	fprintf (fout, "--------------------");
	fprintf (fout, "--------------------");
	fprintf (fout, "--------------------");
	fprintf (fout, "--------------------");
	fprintf (fout, "-----------------|\n");
}

/*
 * Component headings for output.
 */
void
ComponentHeading (void)
{
	fprintf (fout, "|   ");
	fprintf (fout, "|                ");
	fprintf (fout, "|                                         ");
	fprintf (fout, "|PRICING");
	fprintf (fout, "|    ");
	fprintf (fout, "|   QUANTITY   ");
	fprintf (fout, "|   STD    ");
	fprintf (fout, "|COMPONENT ");
	fprintf (fout, "|       MFG COMPONENT  COST BREAKDOWN       |\n");

	fprintf (fout, "|LVL");
	fprintf (fout, "|  COMPONENT NO  ");
	fprintf (fout, "|              DESCRIPTION                ");
	fprintf (fout, "| CONV. ");
	fprintf (fout, "|UOM ");
	fprintf (fout, "|   REQUIRED   ");
	fprintf (fout, "|   COST   ");
	fprintf (fout, "|   COST   ");
	fprintf (fout, "| MATERIAL ");
	fprintf (fout, "|  SETUP   ");
	fprintf (fout, "|   RUN    ");
	fprintf (fout, "|  CLEAN   |\n");

	fprintf (fout, "|---");
	fprintf (fout, "|----------------");
	fprintf (fout, "|-----------------------------------------");
	fprintf (fout, "|-------");
	fprintf (fout, "|----");
	fprintf (fout, "|--------------");
	fprintf (fout, "|----------");
	fprintf (fout, "|----------");
	fprintf (fout, "|----------");
	fprintf (fout, "|----------");
	fprintf (fout, "|----------");
	fprintf (fout, "|----------|\n");
}

int 
RollupSort (
 const void *a1, 
 const void *b1)
{
	int	result;
	const struct RollupStruct a = * (const struct RollupStruct *) a1;
	const struct RollupStruct b = * (const struct RollupStruct *) b1;

	result = strcmp (a.key, b.key);

	return (result);
}

int
heading (
 int scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);

		clear ();
		snorm ();

		rv_pr (ML (mlSkMess044), 26, 0, 1);

		box (0, 3, 80, 14);
		line_at (1, 0,80);
		line_at (8, 1,79);
		line_at (11,1,79);
		line_at (13,1,79);
		line_at (16,1,79);
		line_at (20,0,80);

		print_at (21,0,ML (mlStdMess038), comm_rec.co_no,  comm_rec.co_name);
		print_at (22,0,ML (mlStdMess039), comm_rec.est_no, comm_rec.est_name);

		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}


/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_brmaint.c,v 5.5 2002/04/11 03:46:20 scott Exp $
|  Program Name  : (sk_brmaint.c)
|  Program Desc  : (Add / Update Branch Stock Records)
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written  : 10/05/86         |
|---------------------------------------------------------------------|
| $Log: sk_brmaint.c,v $
| Revision 5.5  2002/04/11 03:46:20  scott
| Updated to add comments to audit files.
|
| Revision 5.4  2001/12/10 06:10:47  scott
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_brmaint.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_brmaint/sk_brmaint.c,v 5.5 2002/04/11 03:46:20 scott Exp $";

#define	TABLINES	10
#define	MAXSCNS		6

#include <pslscr.h>
#include <twodec.h>
#include <ml_sk_mess.h>
#include <ml_std_mess.h>
#include <DBAudit.h>

#define	HAZARD		 (inputType [0] == 'Z')
#define	HNDLNG		 (inputType [0] == 'H')

#include	"schema"

struct commRecord	comm_rec;
struct inmrRecord	inmr_rec;
struct ineiRecord	inei_rec;
struct pchcRecord	pchc_rec;
struct pcpxRecord	pcpx_rec;
struct inumRecord	inum_rec;

	int		*inei_expiry_prd	=	&inei_rec.expiry_prd1;
	/*
	 * Inventory Establishment/Branch Manufacturing
	 */
#define	INEM_NO_FIELDS	15

	struct dbview	inem_list [INEM_NO_FIELDS] =
	{
		{"inem_hhbr_hash"},
		{"inem_est_no"},
		{"inem_res_type"},
		{"inem_setup_std_dir"},
		{"inem_setup_std_fix"},
		{"inem_setup_act_dir"},
		{"inem_setup_act_fix"},
		{"inem_run_std_dir"},
		{"inem_run_std_fix"},
		{"inem_run_act_dir"},
		{"inem_run_act_fix"},
		{"inem_clean_std_dir"},
		{"inem_clean_std_fix"},
		{"inem_clean_act_dir"},
		{"inem_clean_act_fix"}
	};

	struct tag_inemRecord
	{
		long	hhbr_hash;
		char	est_no [3];
		char	res_type [2];
		double	setup_std_dir;
		double	setup_std_fix;
		double	setup_act_dir;
		double	setup_act_fix;
		double	run_std_dir;
		double	run_std_fix;
		double	run_act_dir;
		double	run_act_fix;
		double	clean_std_dir;
		double	clean_std_fix;
		double	clean_act_dir;
		double	clean_act_fix;
	}	inem_rec [6];

	int		specialVersion 	= FALSE,
			newItem	 		= FALSE,
			manItem	 		= FALSE;

	char	inputType [2];

	double	resTotal		= 0.00,
			stdTotalSetup	= 0.00,
			stdTotalRun		= 0.00,
			stdTotalClean	= 0.00,
			actTotalSetup	= 0.00,
			actTotalRun		= 0.00,
			actTotalClean	= 0.00;

	char	*scn_desc [] = {
			" Item Maintenance Screen 1 ",
			" Item Maintenance Screen 2 ",
			" Manufacturing Screen ",
			" Standard & Actual Material Costs ",
			" Standard Resource breakdown Costs ",
			" Actual Resource Breakdown Costs "
			};

	char	*fieldDesc [] = {
			"Direct Labour",
			"Direct Machine",
			"Direct QC-Check",
			"Direct Special",
			"Direct Other",
			"Fixed Labour",
			"Fixed Machine",
			"Fixed QC-Check",
			"Fixed Special",
			"Fixed Other"
			};

	char	*data	= "data",
			*inem	= "inem";

	extern	int	TruePosition;

/*============================
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	prev_item [17];
	char	invDate [11];
	char	desc [36];
	char	strength [6];
	char	hazardDesc [41];
	char	handleDesc [41];
	char	std_uom [5];
	char	uomDesc [21];

	double	stdTotSet;
	double	stdTotRun;
	double	stdTotCln;
	double	actTotSet;
	double	actTotRun;
	double	actTotCln;

	char	stdDesc [16];
	double	stdSetup;
	double	stdRun;
	double	stdClean;
	double	stdTotal;
	char	actDesc [16];
	double	actSetup;
	double	actRun;
	double	actClean;
	double	actTotal;

	char	dfltQty [15];
} local_rec;

enum {SCN_ITEM1 = 1, SCN_ITEM2, SCN_MFG, SCN_MAT_CST, SCN_STD_CST, SCN_ACT_CST};
static	struct	var	vars [] =
{
	{SCN_ITEM1, LIN, "item_no",	 3, 2, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "", "Item Number         ", " ",
		YES, NO,  JUSTLEFT, "", "", inmr_rec.item_no},
	{SCN_ITEM1, LIN, "desc1",	 4, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description         ", " ",
		 NA, NO,  JUSTLEFT, "", "", inmr_rec.description},
	{SCN_ITEM1, LIN, "desc2",	 5, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description         ", " ",
		 NA, NO,  JUSTLEFT, "", "", inmr_rec.description2},
	{SCN_ITEM1, LIN, "avge_cost",	 7, 2, DOUBLETYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", "Average Cost        ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&inei_rec.avge_cost},
	{SCN_ITEM1, LIN, "prev_cost",	 8, 2, DOUBLETYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", "Previous Cost       ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&inei_rec.prev_cost},
	{SCN_ITEM1, LIN, "last_cost",	 9, 2, DOUBLETYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", "Last Cost           ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&inei_rec.last_cost},
	{SCN_ITEM1, LIN, "std_costs",	 10, 2, DOUBLETYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", "Standard Costs      ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&inei_rec.std_cost},
	{SCN_ITEM1, LIN, "date_last_cost",	11, 2, EDATETYPE,
		"NN/NN/NN", "          ",
		" ", local_rec.invDate, "Date Last Cost      ", " ",
		YES, NO,  JUSTLEFT, " ", "", (char *)&inei_rec.date_lcost},
	{SCN_ITEM1, LIN, "last_pur_qty",	12, 2, FLOATTYPE,
		"NNNNNN.NN", "          ",
		" ", "0", "Last Purchase Qty   ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&inei_rec.lpur_qty},
	{SCN_ITEM1, LIN, "abc_code",	14, 2, CHARTYPE,
		"U", "          ",
		" ", "A", "ABC Code            ", "",
		 NO, NO,  JUSTLEFT, "ABCD", "", inei_rec.abc_code},
	{SCN_ITEM1, LIN, "abc_update",	14, 40, CHARTYPE,
		"U", "          ",
		" ", "Y", "ABC Update          ", "Automatic Update of ABC Code (Y/N)",
		 NO, NO,  JUSTLEFT, "YN", "", inei_rec.abc_update},
	{SCN_ITEM1, LIN, "min",	16, 2, FLOATTYPE,
		"NNNNNN.NN", "          ",
		" ", "0", "Minimum Stock       ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&inei_rec.min_stock},
	{SCN_ITEM1, LIN, "max",	17, 2, FLOATTYPE,
		"NNNNNN.NN", "          ",
		" ", "0", "Maximum Stock       ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&inei_rec.max_stock},
	{SCN_ITEM1, LIN, "safety",18, 2, FLOATTYPE,
		"NNNNNN.NN", "          ",
		" ", "0", "Safety Stock        ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&inei_rec.safety_stock},

	{SCN_ITEM2, LIN, "exp1",		 4, 2, INTTYPE,
		"NNN", "          ",
		" ", "0", "First Expiry Period  ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&inei_rec.expiry_prd1},
	{SCN_ITEM2, LIN, "exp2",		 5, 2, INTTYPE,
		"NNN", "          ",
		" ", "0", "Second Expiry Period ", " ",
		 NO, NO, JUSTRIGHT, "", "", (char *)&inei_rec.expiry_prd2},
	{SCN_ITEM2, LIN, "exp3",		 6, 2, INTTYPE,
		"NNN", "          ",
		" ", "0", "Third Expiry Period  ", " ",
		 NO, NO, JUSTRIGHT, "", "", (char *)&inei_rec.expiry_prd3},
	{SCN_ITEM2, LIN, "std_btch",	 8, 2, FLOATTYPE,
		"NNNNNNN.NNNNNN", "          ",
		" ", "",  "Standard Batch       ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *)&inei_rec.std_batch},
	{SCN_ITEM2, LIN, "min_batch",	 9, 2, FLOATTYPE,
		"NNNNNNN.NNNNNN", "          ",
		" ", "",  "Minimum Batch        ", " ",
		 NO, NO, JUSTRIGHT, "", "", (char *)&inei_rec.min_batch},
	{SCN_ITEM2, LIN, "max_batch",	10, 2, FLOATTYPE,
		"NNNNNNN.NNNNNN", "          ",
		" ", "",  "Maximum Batch        ", " ",
		 NO, NO, JUSTRIGHT, "", "", (char *)&inei_rec.max_batch},
	{SCN_ITEM2, LIN, "prd_mult",	11, 2, FLOATTYPE,
		"NNNNNNN.NNNNNN", "          ",
		" ", "", "Prod. Multiple       ", " ",
		 NO, NO, JUSTRIGHT, "", "", (char *)&inei_rec.prd_multiple},
	{SCN_ITEM2, LIN, "haz_class",	13, 2, CHARTYPE,
		"UUUU", "          ",
		" ", " ", "Hazard Class         ", " ",
		YES, NO,  JUSTLEFT, "", "", inei_rec.hzrd_class},
	{SCN_ITEM2, LIN, "hazardDesc",	13, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.hazardDesc},
	{SCN_ITEM2, LIN, "hnd_class",	14, 2, CHARTYPE,
		"UUUU", "          ",
		" ", " ", "Handling Class       ", " ",
		YES, NO,  JUSTLEFT, "", "", inei_rec.hndl_class},
	{SCN_ITEM2, LIN, "handleDesc",	14, 35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.handleDesc},
	{SCN_ITEM2, LIN, "prod_class",	15, 2, CHARTYPE,
		"UUUU", "          ",
		" ", " ", "Production Class     ", " ",
		YES, NO,  JUSTLEFT, "", "", inei_rec.prod_class},

	{SCN_MFG, LIN, "dflt_bom", 	4, 2, INTTYPE, 
		"NNNNN", "          ", 
		" ", " ", "Default BOM No   ", "Default BOM number.", 
		NO, NO, JUSTRIGHT, "0", "32767", (char *) &inei_rec.dflt_bom}, 
	{SCN_MFG, LIN, "dflt_rtg", 	5, 2, INTTYPE, 
		"NNNNN", "          ", 
		" ", " ", "Default RTG No   ", "Default Routing number.", 
		NO, NO, JUSTRIGHT, "0", "32767", (char *) &inei_rec.dflt_rtg}, 
	{SCN_MFG, LIN, "eoq", 	6, 2, FLOATTYPE, 
		local_rec.dfltQty, "          ", 
		" ", " ", "Economic Ord Qty ", "Economic Order Quantity.", 
		NO, NO, JUSTRIGHT, "", "", (char *) &inei_rec.eoq}, 
	{SCN_MFG, LIN, "last_bom", 	8, 2, INTTYPE, 
		"NNNNN", "          ", 
		" ", " ", "Last BOM No      ", "", 
		NA, NO, JUSTRIGHT, "0", "99999", (char *) &inei_rec.last_bom}, 
	{SCN_MFG, LIN, "last_rtg", 	9, 2, INTTYPE, 
		"NNNNN", "          ", 
		" ", " ", "Last RTG No      ", "", 
		NA, NO, JUSTRIGHT, "0", "99999", (char *) &inei_rec.last_rtg}, 

	{SCN_MAT_CST, LIN, "stdMat", 	4, 2, DOUBLETYPE, 
		"NNNNNNNNN.NN", "          ", 
		" ", " ", "Std Materials Costs  ", "", 
		NO, NO, JUSTRIGHT, "", "", (char *) &inem_rec [0].setup_std_dir},
	{SCN_MAT_CST, LIN, "actMat", 	5, 2, DOUBLETYPE, 
		"NNNNNNNNN.NN", "          ", 
		" ", " ", "Act Materials Costs  ", "", 
		NA, NO, JUSTRIGHT, "", "", (char *) &inem_rec [0].setup_act_dir},
	{SCN_MAT_CST, LIN, "mStdBtch",	 7, 2, FLOATTYPE,
		"NNNNNNN.NNNNNN", "          ",
		" ", " ", "Standard Batch       ", " ",
		NA, NO, JUSTRIGHT, "", "", (char *)&inei_rec.std_batch},
	{SCN_MAT_CST, LIN, "mOuter",	 8, 2, FLOATTYPE,
		"NNNNN.N", "          ",
		" ", " ", "Pricing Conv         ", " ",
		NA, NO, JUSTRIGHT, "", "", (char *)&inmr_rec.outer_size},

	{SCN_STD_CST, TAB, "stdDesc",	MAXLINES, 0, CHARTYPE, 
		"AAAAAAAAAAAAAAA", "          ", 
		" ", " ", "              ", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.stdDesc}, 
	{SCN_STD_CST, TAB, "stdSetup",	0, 0, DOUBLETYPE, 
		"NNNNNNNNN.NN", "          ", 
		" ", " ", "   Setup    ", "", 
		NO, NO, JUSTRIGHT, "", "", (char *) &local_rec.stdSetup}, 
	{SCN_STD_CST, TAB, "stdRun",	0, 0, DOUBLETYPE, 
		"NNNNNNNNN.NN", "          ", 
		" ", " ", "    Run     ", "", 
		NO, NO, JUSTRIGHT, "", "", (char *) &local_rec.stdRun}, 
	{SCN_STD_CST, TAB, "stdClean",	0, 0, DOUBLETYPE, 
		"NNNNNNNNN.NN", "          ", 
		" ", " ", "   Clean    ", "", 
		NO, NO, JUSTRIGHT, "", "", (char *) &local_rec.stdClean}, 
	{SCN_STD_CST, TAB, "stdTotal",	0, 0, DOUBLETYPE, 
		"NNNNNNNNN.NN", "          ", 
		" ", " ", "   Total    ", "", 
		NA, NO, JUSTRIGHT, "", "", (char *) &local_rec.stdTotal}, 

	{SCN_ACT_CST, TAB, "actDesc",	MAXLINES, 0, CHARTYPE, 
		"AAAAAAAAAAAAAAA", "          ", 
		" ", " ", "              ", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.actDesc}, 
	{SCN_ACT_CST, TAB, "actSetup",	0, 0, DOUBLETYPE, 
		"NNNNNNNNN.NN", "          ", 
		" ", " ", "   Setup    ", "", 
		NA, NO, JUSTRIGHT, "", "", (char *) &local_rec.actSetup}, 
	{SCN_ACT_CST, TAB, "actRun",	0, 0, DOUBLETYPE, 
		"NNNNNNNNN.NN", "          ", 
		" ", " ", "    Run     ", "", 
		NA, NO, JUSTRIGHT, "", "", (char *) &local_rec.actRun}, 
	{SCN_ACT_CST, TAB, "actClean",	0, 0, DOUBLETYPE, 
		"NNNNNNNNN.NN", "          ", 
		" ", " ", "   Clean    ", "", 
		NA, NO, JUSTRIGHT, "", "", (char *) &local_rec.actClean}, 
	{SCN_ACT_CST, TAB, "actTotal",	0, 0, DOUBLETYPE, 
		"NNNNNNNNN.NN", "          ", 
		" ", " ", "   Total    ", "", 
		NA, NO, JUSTRIGHT, "", "", (char *) &local_rec.actTotal}, 

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*
 * Function Declarations
 */
static int MoneyZero 	(double);
void 	OpenDB 			(void);
void 	CloseDB 		(void);
int  	spec_valid 		(int);
void 	LoadCosts 		(void);
int  	TotalStdCost 	(void);
void 	DisplayTotals 	(int);
int  	ValidQuantity 	(double, int);
void 	SrchPchc 		(char *);
void 	SrchPcpx 		(char *);
void 	Update 			(void);
void 	UnloadCosts 	(void);
int  	heading 		(int);


/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int argc, 
 char * argv [])
{
	char	*sptr; 
	int		i;

	TruePosition	=	TRUE;

	sptr = strrchr (argv [0], '/');
	if (sptr)
		sptr++;
	else
		sptr = argv [0];

	if (!strcmp (sptr, "sk_sp_brmaint"))
		specialVersion = TRUE;

	/*--------------------------
	| Set up the Quantity Mask |
	--------------------------*/
	sptr = chk_env ("SK_QTY_MASK");
	if (sptr == (char *)0)
		strcpy (local_rec.dfltQty, "NNNNNNN.NNNNNN");
	else
		strcpy (local_rec.dfltQty, sptr);

	SETUP_SCR (vars);

	init_scr ();
	set_tty ();
	set_masks ();

	for (i = 0; i < MAXSCNS; i ++)
		tab_data [i]._desc = scn_desc [i];

	OpenDB ();

	strcpy (local_rec.invDate, DateToString (comm_rec.inv_date));

	while (prog_exit == 0)
	{
		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		newItem		= FALSE;
		search_ok	= TRUE;
		init_vars (SCN_ITEM1);
		init_vars (SCN_ITEM2);
		init_vars (SCN_MFG);
		init_vars (SCN_MAT_CST);
		manItem = FALSE;


		heading (SCN_ITEM1);
		entry (SCN_ITEM1);
		if (restart || prog_exit)
			continue;

		if (!newItem)
		{
			heading (SCN_ITEM1);
			scn_display (SCN_ITEM1);
		}
		else
		{
			heading (SCN_ITEM2);
			entry (SCN_ITEM2);

			if (manItem)
			{
				heading (SCN_MFG);
				entry (SCN_MFG);
			}
		}
		if (restart)
			continue;

		if (manItem)
		{
			tab_row = 4;
			tab_col = 5;

			edit_ok (SCN_MFG);
			edit_ok (SCN_MAT_CST);
			edit_ok (SCN_STD_CST);
			edit_ok (SCN_ACT_CST);
		}
		else
		{
			no_edit (SCN_MFG);
			no_edit (SCN_MAT_CST);
			no_edit (SCN_STD_CST);
			no_edit (SCN_ACT_CST);
		}

		do
		{
			edit_all ();
			if (restart)
				break;

			if (manItem && TotalStdCost ())
			{
				sprintf (err_str, ML (mlSkMess320),twodec (resTotal), twodec (inei_rec.std_cost));
				print_mess (err_str);
				sleep (sleepTime);
				clear_mess ();
			}
			else
				break;
		} while (TRUE);
		if (restart)
			continue;

		Update ();
	}
	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

/*
 * Open data base files.
 */
void
OpenDB (void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (inei, inei_list, INEI_NO_FIELDS, "inei_id_no");
	open_rec (inem, inem_list, INEM_NO_FIELDS, "inem_id_no");
	open_rec (pchc, pchc_list, PCHC_NO_FIELDS, "pchc_id_no");
	open_rec (pcpx, pcpx_list, PCPX_NO_FIELDS, "pcpx_id_no");
	open_rec (inum, inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	/*
	 * Open audit file.
	 */
	OpenAuditFile ("StockBranchMaster.txt");
}

/*
 * Close data base files.
 */
void
CloseDB (void)
{
	abc_fclose (inmr);
	abc_fclose (inei);
	abc_fclose (inem);
	abc_fclose (pchc);
	abc_fclose (pcpx);
	abc_fclose (inum);

	SearchFindClose ();
	/*
	 * Close audit file.
	 */
	CloseAuditFile ();
	abc_dbclose (data);
}

int
spec_valid (
	int		field)
{
	int		exp_no, 
			i;

	/*
	 * Validate Item Number input.
	 */
	if (LCHECK ("item_no"))
	{
		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}
		cc = FindInmr (comm_rec.co_no, inmr_rec.item_no, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			errmess (ML (mlStdMess001));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		SuperSynonymError ();
		DSP_FLD ("item_no");
		DSP_FLD ("desc1");
		DSP_FLD ("desc2");

		if (!specialVersion)
		{
			FLD ("avge_cost")		= NA;
			FLD ("last_cost")		= NA;
			FLD ("date_last_cost")	= NA;
			FLD ("last_pur_qty")	= NA;
		}
		if (!strcmp (inmr_rec.source, "PP") ||
		    !strcmp (inmr_rec.source, "RM"))
		{
			FLD ("avge_cost")		= YES;
			FLD ("last_cost")		= YES;
			FLD ("date_last_cost")	= YES;
			FLD ("last_pur_qty")	= YES;
		}

		FLD ("std_btch")	= NA;
		FLD ("min_batch")	= NA;
		FLD ("max_batch")	= NA;
		FLD ("prd_mult")	= NA;
		if (!strcmp (inmr_rec.source, "MC") ||
		    !strcmp (inmr_rec.source, "MP") ||
		    !strcmp (inmr_rec.source, "BM") ||
		    !strcmp (inmr_rec.source, "BP"))
		{
			FLD ("std_btch")	= YES;
			FLD ("min_batch")	= YES;
			FLD ("max_batch")	= YES;
			FLD ("prd_mult")	= YES;
			manItem			= TRUE;
		}

		inum_rec.hhum_hash	=	inmr_rec.std_uom;
		cc = find_rec (inum, &inum_rec, COMPARISON, "r");
		if (cc)
		{
			strcpy (local_rec.std_uom, "    ");
			sprintf (local_rec.uomDesc, "%-21.21s", "Unknown UOM");
		}
		else
		{
			sprintf (local_rec.std_uom, "%-4.4s", inum_rec.uom);
			sprintf (local_rec.uomDesc, "%-21.21s", inum_rec.desc);
		}

		strcpy (inei_rec.est_no, comm_rec.est_no);
		inei_rec.hhbr_hash = inmr_rec.hhbr_hash;
		cc = find_rec (inei, &inei_rec, COMPARISON, "w");
		if (cc)
			newItem	 = TRUE;
		else
		{
			/*
	 		 * Save old record.
	 		 */
			SetAuditOldRec (&inei_rec, sizeof (inei_rec));
			entry_exit = 1;
			newItem	 = FALSE;

			/*
			 * Lookup hazard, handling class
			 */
			strcpy (pchc_rec.co_no, comm_rec.co_no);
			strcpy (pchc_rec.type, "Z");
			sprintf (pchc_rec.pchc_class, inei_rec.hzrd_class);
			cc = find_rec (pchc, &pchc_rec, COMPARISON, "r");
			if (cc)
			{
				sprintf (local_rec.hazardDesc, "%-40.40s", ML (mlSkMess556));
			}	
			else
				sprintf (local_rec.hazardDesc, "%-40.40s", pchc_rec.desc);

			strcpy (pchc_rec.co_no, comm_rec.co_no);
			strcpy (pchc_rec.type, "H");
			sprintf (pchc_rec.pchc_class, inei_rec.hndl_class);
			cc = find_rec (pchc, &pchc_rec, COMPARISON, "r");
			if (cc)
			{
				sprintf (local_rec.handleDesc, "%-40.40s", ML (mlSkMess556));
			}
			else
				sprintf (local_rec.handleDesc, "%-40.40s", pchc_rec.desc);
			if (manItem)
			{
				LoadCosts ();
				if (TotalStdCost ())
				{
					sprintf (err_str, ML (mlSkMess320),twodec (resTotal), twodec (inei_rec.std_cost));
					print_mess (err_str);
					sleep (sleepTime);
					clear_mess ();
				}
			}
		}

		DSP_FLD ("item_no");
		DSP_FLD ("desc1");
		DSP_FLD ("desc2");
		return (EXIT_SUCCESS);
	}

	if (LNCHECK ("exp", 3))
	{
		exp_no = FIELD.label [3] - '1';

		if (inei_expiry_prd [exp_no] == 0)
		{
			for (i = exp_no + 1; i < 3; i++)
				inei_expiry_prd [i] = 0;

			DSP_FLD ("exp1");
			DSP_FLD ("exp2");
			DSP_FLD ("exp3");
		}

		if (exp_no == 0)
			return (EXIT_SUCCESS);

		if (inei_expiry_prd [exp_no - 1] == 0)
		{
			inei_expiry_prd [exp_no] = 0;
			return (EXIT_SUCCESS);
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("haz_class") || LCHECK ("hnd_class"))
	{
		if (dflt_used || F_NOKEY (field))
			return (EXIT_SUCCESS);

		if (LCHECK ("haz_class"))
			strcpy (inputType, "Z");
		else
			strcpy (inputType, "H");

		if (SRCH_KEY)
		{
			SrchPchc (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (pchc_rec.co_no, comm_rec.co_no);
		strcpy (pchc_rec.type, inputType);
		if (HAZARD)
			sprintf (pchc_rec.pchc_class, inei_rec.hzrd_class);
		else
			sprintf (pchc_rec.pchc_class, inei_rec.hndl_class);
		cc = find_rec (pchc, &pchc_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlSkMess556));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (HAZARD)
		{
			sprintf (local_rec.hazardDesc, "%-40.40s", pchc_rec.desc);
			DSP_FLD ("hazardDesc");
		}
		else
		{
			sprintf (local_rec.handleDesc, "%-40.40s", pchc_rec.desc);
			DSP_FLD ("handleDesc");
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("prod_class"))
	{
		if (dflt_used || F_NOKEY (field))
			return (EXIT_SUCCESS);
		
		if (SRCH_KEY)
		{
			SrchPcpx (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (pcpx_rec.co_no, comm_rec.co_no);
		strcpy (pcpx_rec.pcpx_class, inei_rec.prod_class);
		strcpy (pcpx_rec.excl_class, "    ");
		cc = find_rec (pcpx, &pcpx_rec, GTEQ, "r");
		if (cc || strcmp (pcpx_rec.pcpx_class, inei_rec.prod_class))
		{
			print_mess (ML (mlSkMess556));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("std_btch"))
	{
		if (dflt_used)
			return (EXIT_SUCCESS);

		if (inei_rec.std_batch <= 0.00)
		{
			print_mess (ML (mlSkMess321));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		clear_mess ();

		if (prog_status == ENTRY)
			return (EXIT_SUCCESS);

		if (inei_rec.std_batch < inei_rec.min_batch)
		{
			inei_rec.min_batch = inei_rec.std_batch;
			DSP_FLD ("min_batch");
		}

		if (inei_rec.std_batch > inei_rec.max_batch &&
		    inei_rec.max_batch != 0.00)
		{
			inei_rec.max_batch = inei_rec.std_batch;
			DSP_FLD ("max_batch");
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("min_batch"))
	{
		if (dflt_used)
			inei_rec.min_batch = inei_rec.std_batch;

		if (inei_rec.min_batch < 0.00)
		{
			print_mess (ML (mlSkMess322));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (inei_rec.min_batch > inei_rec.std_batch)
		{
			print_mess (ML (mlSkMess323));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		clear_mess ();

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("max_batch"))
	{
		if (dflt_used)
			inei_rec.max_batch = inei_rec.std_batch;

		if (inei_rec.max_batch < inei_rec.std_batch &&
		    inei_rec.max_batch != 0.00)
		{
			print_mess (ML (mlSkMess324));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		clear_mess ();

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("dflt_bom"))
	{
		if (dflt_used)
		{
			inei_rec.dflt_bom = inmr_rec.dflt_bom;
			DSP_FLD ("dflt_bom");
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("dflt_rtg"))
	{
		if (dflt_used)
		{
			inei_rec.dflt_rtg = inmr_rec.dflt_rtg;
			DSP_FLD ("dflt_rtg");
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("eoq"))
	{
		if (dflt_used)
		{
			inei_rec.eoq = inmr_rec.eoq;
			DSP_FLD ("eoq");
		}
		inei_rec.eoq = (float) (n_dec (inei_rec.eoq, inmr_rec.dec_pt));
		if (!ValidQuantity (inei_rec.eoq, inmr_rec.dec_pt))
			return (EXIT_FAILURE);
		return (EXIT_SUCCESS);
	}

	if (LNCHECK ("std", 3))
	{
		if (LCHECK ("std_btch") ||
			!manItem)
			return (EXIT_SUCCESS);

		if (F_NOKEY (field))
			return (EXIT_SUCCESS);

		if (!LCHECK ("std_costs") && !LCHECK ("stdMat"))
		{
			local_rec.stdTotal  = local_rec.stdSetup;
			local_rec.stdTotal += local_rec.stdRun;
			local_rec.stdTotal += local_rec.stdClean;
			putval (line_cnt);
			DSP_FLD ("stdTotal");
		}

		if (TotalStdCost ())
		{
			if (LCHECK ("std_costs"))
			{
				sprintf (err_str, ML (mlSkMess325), twodec (inei_rec.std_cost), twodec (resTotal));

				print_mess (err_str);
				sleep (sleepTime);
				clear_mess ();
				scn_set (1);
				return (EXIT_FAILURE);
			}
			inei_rec.std_cost = resTotal;
		}

		if (LCHECK ("std_costs"))
			scn_set (1);
		else
		if (LCHECK ("stdMat"))
			scn_set (4);
		else
		{
			getval (line_cnt);
			DisplayTotals (TRUE);
		}
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

/*
 * load std costs in to tab screen
 */
void
LoadCosts (void)
{
	int		i;

	/*
	 * Read all inem (mfg breakdown costs) records.
	 */
	for (i = 0; i < 6; i ++)
	{
		inem_rec [i].hhbr_hash = inmr_rec.hhbr_hash;
		strcpy (inem_rec [i].est_no, comm_rec.est_no);
		if (i == 0)
			strcpy (inem_rec [i].res_type, "T"); /* maTerial */
		if (i == 1)
			strcpy (inem_rec [i].res_type, "L"); /* Labour */
		if (i == 2)
			strcpy (inem_rec [i].res_type, "M"); /* Machine */
		if (i == 3)
			strcpy (inem_rec [i].res_type, "Q"); /* Qc-check */
		if (i == 4)
			strcpy (inem_rec [i].res_type, "S"); /* Special */
		if (i == 5)
			strcpy (inem_rec [i].res_type, "O"); /* Other */
		if (find_rec (inem, &inem_rec [i], EQUAL, "w"))
		{
			/* costs not found set values to zero */

			inem_rec [i].setup_std_dir = 0.00;
			inem_rec [i].setup_std_fix = 0.00;
			inem_rec [i].setup_act_dir = 0.00;
			inem_rec [i].setup_act_fix = 0.00;
			inem_rec [i].run_std_dir = 0.00;
			inem_rec [i].run_std_fix = 0.00;
			inem_rec [i].run_act_dir = 0.00;
			inem_rec [i].run_act_fix = 0.00;
			inem_rec [i].clean_std_dir = 0.00;
			inem_rec [i].clean_std_fix = 0.00;
			inem_rec [i].clean_act_dir = 0.00;
			inem_rec [i].clean_act_fix = 0.00;
		}
	}

	lcount [SCN_STD_CST] = 0;
	scn_set (SCN_STD_CST);

	stdTotalSetup = 0.00;
	stdTotalRun = 0.00;
	stdTotalClean = 0.00;

	for (i = 0; i < 10; i ++)
	{
		strcpy (local_rec.stdDesc, fieldDesc [i]);
		switch (i)
		{
		case	0: /* standard labour direct costs */
			local_rec.stdSetup	= inem_rec [1].setup_std_dir;
			local_rec.stdRun	= inem_rec [1].run_std_dir;
			local_rec.stdClean	= inem_rec [1].clean_std_dir;
			break;
		case	1: /* standard machine direct costs */
			local_rec.stdSetup	= inem_rec [2].setup_std_dir;
			local_rec.stdRun	= inem_rec [2].run_std_dir;
			local_rec.stdClean	= inem_rec [2].clean_std_dir;
			break;
		case	2: /* standard qc-check direct costs */
			local_rec.stdSetup	= inem_rec [3].setup_std_dir;
			local_rec.stdRun	= inem_rec [3].run_std_dir;
			local_rec.stdClean	= inem_rec [3].clean_std_dir;
			break;
		case	3: /* standard special direct costs */
			local_rec.stdSetup	= inem_rec [4].setup_std_dir;
			local_rec.stdRun	= inem_rec [4].run_std_dir;
			local_rec.stdClean	= inem_rec [4].clean_std_dir;
			break;
		case	4: /* standard other direct costs */
			local_rec.stdSetup	= inem_rec [5].setup_std_dir;
			local_rec.stdRun	= inem_rec [5].run_std_dir;
			local_rec.stdClean	= inem_rec [5].clean_std_dir;
			break;
		case	5: /* standard labour fixed costs */
			local_rec.stdSetup	= inem_rec [1].setup_std_fix;
			local_rec.stdRun	= inem_rec [1].run_std_fix;
			local_rec.stdClean	= inem_rec [1].clean_std_fix;
			break;
		case	6: /* standard machine fixed costs */
			local_rec.stdSetup	= inem_rec [2].setup_std_fix;
			local_rec.stdRun	= inem_rec [2].run_std_fix;
			local_rec.stdClean	= inem_rec [2].clean_std_fix;
			break;
		case	7: /* standard qc-check fixed costs */
			local_rec.stdSetup	= inem_rec [3].setup_std_fix;
			local_rec.stdRun	= inem_rec [3].run_std_fix;
			local_rec.stdClean	= inem_rec [3].clean_std_fix;
			break;
		case	8: /* standard special fixed costs */
			local_rec.stdSetup	= inem_rec [4].setup_std_fix;
			local_rec.stdRun	= inem_rec [4].run_std_fix;
			local_rec.stdClean	= inem_rec [4].clean_std_fix;
			break;
		case	9: /* standard other fixed costs */
			local_rec.stdSetup	= inem_rec [5].setup_std_fix;
			local_rec.stdRun	= inem_rec [5].run_std_fix;
			local_rec.stdClean	= inem_rec [5].clean_std_fix;
			break;
		}
		local_rec.stdTotal	= local_rec.stdSetup +
							  local_rec.stdRun +
							  local_rec.stdClean;

		stdTotalSetup	+= local_rec.stdSetup;
		stdTotalRun		+= local_rec.stdRun;
		stdTotalClean	+= local_rec.stdClean;

		putval (lcount [SCN_STD_CST] ++);
	}

	lcount [SCN_ACT_CST] = 0;
	scn_set (SCN_ACT_CST);

	actTotalSetup	= 0.00;
	actTotalRun		= 0.00;
	actTotalClean	= 0.00;

	for (i = 0; i < 10; i ++)
	{
		strcpy (local_rec.actDesc, fieldDesc [i]);
		switch (i)
		{
		case	0: /* actual labour direct costs */
			local_rec.actSetup	= inem_rec [1].setup_act_dir;
			local_rec.actRun	= inem_rec [1].run_act_dir;
			local_rec.actClean	= inem_rec [1].clean_act_dir;
			break;
		case	1: /* actual machine direct costs */
			local_rec.actSetup	= inem_rec [2].setup_act_dir;
			local_rec.actRun	= inem_rec [2].run_act_dir;
			local_rec.actClean	= inem_rec [2].clean_act_dir;
			break;
		case	2: /* actual qc-check direct costs */
			local_rec.actSetup	= inem_rec [3].setup_act_dir;
			local_rec.actRun	= inem_rec [3].run_act_dir;
			local_rec.actClean	= inem_rec [3].clean_act_dir;
			break;
		case	3: /* actual special direct costs */
			local_rec.actSetup	= inem_rec [4].setup_act_dir;
			local_rec.actRun	= inem_rec [4].run_act_dir;
			local_rec.actClean	= inem_rec [4].clean_act_dir;
			break;
		case	4: /* actual other direct costs */
			local_rec.actSetup	= inem_rec [5].setup_act_dir;
			local_rec.actRun	= inem_rec [5].run_act_dir;
			local_rec.actClean	= inem_rec [5].clean_act_dir;
			break;
		case	5: /* actual labour fixed costs */
			local_rec.actSetup	= inem_rec [1].setup_act_fix;
			local_rec.actRun	= inem_rec [1].run_act_fix;
			local_rec.actClean	= inem_rec [1].clean_act_fix;
			break;
		case	6: /* actual machine fixed costs */
			local_rec.actSetup	= inem_rec [2].setup_act_fix;
			local_rec.actRun	= inem_rec [2].run_act_fix;
			local_rec.actClean	= inem_rec [2].clean_act_fix;
			break;
		case	7: /* actual qc-check fixed costs */
			local_rec.actSetup	= inem_rec [3].setup_act_fix;
			local_rec.actRun	= inem_rec [3].run_act_fix;
			local_rec.actClean	= inem_rec [3].clean_act_fix;
			break;
		case	8: /* actual special fixed costs */
			local_rec.actSetup	= inem_rec [4].setup_act_fix;
			local_rec.actRun	= inem_rec [4].run_act_fix;
			local_rec.actClean	= inem_rec [4].clean_act_fix;
			break;
		case	9: /* actual other fixed costs */
			local_rec.actSetup	= inem_rec [5].setup_act_fix;
			local_rec.actRun	= inem_rec [5].run_act_fix;
			local_rec.actClean	= inem_rec [5].clean_act_fix;
			break;
		}
		local_rec.actTotal	= local_rec.actSetup +
							  local_rec.actRun +
							  local_rec.actClean;

		actTotalSetup	+= local_rec.actSetup;
		actTotalRun		+= local_rec.actRun;
		actTotalClean	+= local_rec.actClean;

		putval (lcount [SCN_ACT_CST] ++);
	}
}

/*
 * Total all the individual material and resource costs, and compare to std cost
 */
int
TotalStdCost (void)
{
	int		i;

	scn_set (SCN_STD_CST);

	stdTotalSetup	= 0.00;
	stdTotalRun		= 0.00;
	stdTotalClean	= 0.00;

	resTotal = inem_rec [0].setup_std_dir;

	for (i = 0; i < lcount [SCN_STD_CST]; i ++)
	{
		getval (i);
		resTotal += local_rec.stdSetup;
		resTotal += local_rec.stdRun;
		resTotal += local_rec.stdClean;

		stdTotalSetup += local_rec.stdSetup;
		stdTotalRun   += local_rec.stdRun;
		stdTotalClean += local_rec.stdClean;
	}
	if (!MoneyZero ((resTotal - inei_rec.std_cost)))
		return (EXIT_FAILURE);

	return (EXIT_SUCCESS);
}

void
DisplayTotals (
	int		flag)
{
	if (flag) /* display standard resource costs */
	{
		sprintf (err_str," %s          %12.2f %12.2f %12.2f %12.2f", ML (mlSkMess563), stdTotalSetup, stdTotalRun, stdTotalClean, (stdTotalSetup + stdTotalRun + stdTotalClean)); 

		rv_pr (err_str, 5, 17, 0);
	}
	else /* display actual resource costs */
	{
		sprintf (err_str, " %s          %12.2f %12.2f %12.2f %12.2f", ML (mlSkMess563), actTotalSetup, actTotalRun, actTotalClean, (actTotalSetup + actTotalRun + actTotalClean)); 

		rv_pr (err_str, 5, 17, 0);
	}
}

/*
 * Checks if the quantity entered by the user valid quantity that can be saved 
 * to a float variable without any problems of losing figures after the 
 * decimal point.  eg. if _dec_pt is 2 then the greatest quantity the user 
 * can enter is 99999.99   
 */
int
ValidQuantity (
	double 	_qty, 
	int 	_dec_pt)
{
	/*
	 * Quantities to be compared with with the user has entered.    
	 */
	double	compare [7];
	
	compare [0] = 9999999.00;
	compare [1] = 999999.90;
	compare [2] = 99999.99;
	compare [3] = 9999.999;
	compare [4] = 999.9999;
	compare [5] = 99.99999;
	compare [6] = 9.999999;

	if (_qty > compare [_dec_pt])
	{
		sprintf (err_str, ML (mlSkMess238), _qty, compare [_dec_pt]);

		print_mess (err_str);
		sleep (sleepTime);
		clear_mess ();
		return (FALSE);
	}

	return (TRUE);
}

void
SrchPchc (
	char	*key_val)
{
	_work_open (4,0,40);
	save_rec ("#Code", "#Description");

	strcpy (pchc_rec.co_no, comm_rec.co_no);
	strcpy (pchc_rec.type, inputType);
	sprintf (pchc_rec.pchc_class, "%-4.4s", key_val);
	cc = find_rec (pchc, &pchc_rec, GTEQ, "r");
	while (!cc &&
		!strcmp (pchc_rec.co_no, comm_rec.co_no) &&
		!strcmp (pchc_rec.type, inputType) &&
		!strncmp (pchc_rec.pchc_class, key_val, strlen (key_val)))
	{
		cc = save_rec (pchc_rec.pchc_class, pchc_rec.desc);
		if (cc)
			break;

		cc = find_rec (pchc, &pchc_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (pchc_rec.co_no, comm_rec.co_no);
	strcpy (pchc_rec.type, inputType);
	sprintf (pchc_rec.pchc_class, "%-4.4s", key_val);
	cc = find_rec (pchc, &pchc_rec, GTEQ, "r");
	if (cc)
		file_err (cc, pchc, "DBFIND");
}

void
SrchPcpx (
	char	*key_val)
{
	char	old_class [5];

	old_class [0] = 0;
	_work_open (4,0,40);
	save_rec ("#Code", "#");

	strcpy (pcpx_rec.co_no, comm_rec.co_no);
	sprintf (pcpx_rec.pcpx_class, "%-4.4s", key_val);
	strcpy (pcpx_rec.excl_class, "    ");
	cc = find_rec (pcpx, &pcpx_rec, GTEQ, "r");
	while (!cc && !strcmp (pcpx_rec.co_no, comm_rec.co_no))
	{
		if (strcmp (pcpx_rec.pcpx_class, old_class))
		{
			strcpy (old_class, pcpx_rec.pcpx_class);
			cc = save_rec (pcpx_rec.pcpx_class, " ");
			if (cc)
				break;
		}

		cc = find_rec (pcpx, &pcpx_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
}

/*==================================================
| Update or add a record to inventory branch file. |
==================================================*/
void
Update (void)
{
	clear ();
	print_at (0,0,ML (mlStdMess035));
	fflush (stdout);

	if (manItem)
		UnloadCosts ();

	/*--------------------------------------
	| Add or update inventory item record. |
	--------------------------------------*/
	if (newItem	 == 1)
	{
		strcpy (inei_rec.stat_flag, "0");
		cc = abc_add (inei, &inei_rec);
		if (cc) 
			file_err (cc, inei, "DBADD");
	}
	else
	{
		/*
	 	 * Update changes audit record.
	 	 */
		 
		 sprintf (err_str, "%s : %s (%s)", ML ("Item"), inmr_rec.item_no, inmr_rec.description);
	 	AuditFileAdd (err_str, &inei_rec, inei_list, INEI_NO_FIELDS);
		cc = abc_update (inei, &inei_rec);
		if (cc) 
			file_err (cc, inei, "DBUPDATE");
	}
    abc_unlock (inei);
	strcpy (local_rec.prev_item, inmr_rec.item_no);
}

void
UnloadCosts (
 void)
{
	double	stdMaterial;
	int		i;

	scn_set (SCN_STD_CST);

	for (i = 0; i < (lcount [SCN_STD_CST] - 1); i ++)
	{
		getval (i);
		switch (i)
		{
		case	0: /* standard labour direct costs */
			inem_rec [1].setup_std_dir	= local_rec.stdSetup;
			inem_rec [1].run_std_dir	= local_rec.stdRun;
			inem_rec [1].clean_std_dir	= local_rec.stdClean;
			break;
		case	1: /* standard machine direct costs */
			inem_rec [2].setup_std_dir	= local_rec.stdSetup;
			inem_rec [2].run_std_dir	= local_rec.stdRun;
			inem_rec [2].clean_std_dir	= local_rec.stdClean;
			break;
		case	2: /* standard qc-check direct costs */
			inem_rec [3].setup_std_dir	= local_rec.stdSetup;
			inem_rec [3].run_std_dir	= local_rec.stdRun;
			inem_rec [3].clean_std_dir	= local_rec.stdClean;
			break;
		case	3: /* standard special direct costs */
			inem_rec [4].setup_std_dir	= local_rec.stdSetup;
			inem_rec [4].run_std_dir	= local_rec.stdRun;
			inem_rec [4].clean_std_dir	= local_rec.stdClean;
			break;
		case	4: /* standard other direct costs */
			inem_rec [5].setup_std_dir	= local_rec.stdSetup;
			inem_rec [5].run_std_dir	= local_rec.stdRun;
			inem_rec [5].clean_std_dir	= local_rec.stdClean;
			break;
		case	5: /* standard labour fixed costs */
			inem_rec [1].setup_std_fix	= local_rec.stdSetup;
			inem_rec [1].run_std_fix	= local_rec.stdRun;
			inem_rec [1].clean_std_fix	= local_rec.stdClean;
			break;
		case	6: /* standard machine fixed costs */
			inem_rec [2].setup_std_fix	= local_rec.stdSetup;
			inem_rec [2].run_std_fix	= local_rec.stdRun;
			inem_rec [2].clean_std_fix	= local_rec.stdClean;
			break;
		case	7: /* standard qc-check fixed costs */
			inem_rec [3].setup_std_fix	= local_rec.stdSetup;
			inem_rec [3].run_std_fix	= local_rec.stdRun;
			inem_rec [3].clean_std_fix	= local_rec.stdClean;
			break;
		case	8: /* standard special fixed costs */
			inem_rec [4].setup_std_fix	= local_rec.stdSetup;
			inem_rec [4].run_std_fix	= local_rec.stdRun;
			inem_rec [4].clean_std_fix	= local_rec.stdClean;
			break;
		case	9: /* standard other fixed costs */
			inem_rec [5].setup_std_fix	= local_rec.stdSetup;
			inem_rec [5].run_std_fix	= local_rec.stdRun;
			inem_rec [5].clean_std_fix	= local_rec.stdClean;
			break;
		}
	}

	stdMaterial = inem_rec [0].setup_std_dir;
	for (i = 0; i < 6; i ++)
	{
		inem_rec [0].hhbr_hash = inmr_rec.hhbr_hash;
		strcpy (inem_rec [0].est_no, comm_rec.est_no);
		if (i == 0)
			strcpy (inem_rec [0].res_type, "T"); /* maTerial */
		if (i == 1)
			strcpy (inem_rec [0].res_type, "L"); /* Labour */
		if (i == 2)
			strcpy (inem_rec [0].res_type, "M"); /* Machine */
		if (i == 3)
			strcpy (inem_rec [0].res_type, "Q"); /* Qc-check */
		if (i == 4)
			strcpy (inem_rec [0].res_type, "S"); /* Special */
		if (i == 5)
			strcpy (inem_rec [0].res_type, "O"); /* Other */
		cc = find_rec (inem, &inem_rec [0], EQUAL, "u");
		if (i == 0)
		{
			strcpy (inem_rec [0].res_type, "T"); /* maTerial */
			inem_rec [0].setup_std_dir	= stdMaterial;
			inem_rec [0].setup_std_fix	= 0.00;
			inem_rec [0].run_std_dir	= 0.00;
			inem_rec [0].run_std_fix	= 0.00;
			inem_rec [0].clean_std_dir	= 0.00;
			inem_rec [0].clean_std_fix	= 0.00;
		}
		else
		{
			inem_rec [0].setup_std_dir	= inem_rec [i].setup_std_dir;
			inem_rec [0].setup_std_fix	= inem_rec [i].setup_std_fix;
			inem_rec [0].run_std_dir	= inem_rec [i].run_std_dir;
			inem_rec [0].run_std_fix	= inem_rec [i].run_std_fix;
			inem_rec [0].clean_std_dir	= inem_rec [i].clean_std_dir;
			inem_rec [0].clean_std_fix	= inem_rec [i].clean_std_fix;
		}

		if (cc)
		{
			if (inem_rec [i].setup_std_dir == 0.00 &&
				inem_rec [i].setup_std_fix == 0.00 &&
				inem_rec [i].run_std_dir == 0.00 &&
				inem_rec [i].run_std_fix == 0.00 &&
				inem_rec [i].clean_std_dir == 0.00 &&
				inem_rec [i].clean_std_fix == 0.00)
				continue; 

			if ((cc = abc_add (inem, &inem_rec [0])))
				file_err (cc, inem, "DBADD");
		}
		else
		{
			if ((cc = abc_update (inem, &inem_rec [0])))
				file_err (cc, inem, "DBUPDATE");
		}
	}
}

int
heading (
	int		scn)
{
	int	i;

	if (!restart)
	{
		if (scn != cur_screen)
			scn_set (scn);

		clear ();

		rv_pr (ML (mlSkMess326), 27, 0, 1);
		print_at (0,55, ML (mlSkMess096) , local_rec.prev_item); 

		line_at (1,0,80);

		switch (scn)
		{
		case	SCN_ITEM1:
			box (0, 2, 80, 16);
			line_at (6, 1,79);
			line_at (13,1,79);
			line_at (15,1,79);
			break;
		
		case	SCN_ITEM2:
			box (0, 3, 80, 12);
			line_at (7, 1,79);
			line_at (12,1,79);

			sprintf (err_str, ML (mlSkMess623),
								local_rec.std_uom, local_rec.uomDesc);
			rv_pr (err_str, 40, 8, 0);

			for (i = 0; i < 3; i++)
				rv_pr (ML (mlSkMess624), 35, 4 + i, 0);
				
			break;

		case	SCN_MFG:
			box (0, 3, 80, 6);
			line_at (7,1,79);
			break;

		case	SCN_MAT_CST:
			box (0, 3, 80, 5);
			line_at (6, 1, 79);
			break;

		case	SCN_STD_CST:
			box (5, 3, 69, 12);

			rv_pr (ML (mlSkMess327), 23, 2, 1);
			DisplayTotals (TRUE);

			sprintf (err_str, ML (mlSkMess328), inei_rec.std_batch, inmr_rec.outer_size);

			rv_pr (err_str, 6, 18, 0);
			break;

		case	SCN_ACT_CST:
			box (5, 3, 69, 12);

			rv_pr (ML (mlSkMess329), 24, 2, 1);

			DisplayTotals (FALSE);

			sprintf (err_str, ML (mlSkMess328), inei_rec.std_batch, inmr_rec.outer_size);

			rv_pr (err_str, 6, 18, 0);
			break;
		}
		print_at (20,0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name); 
		print_at (21,0, ML (mlStdMess039), comm_rec.est_no, comm_rec.est_name); 

		line_at (22,0,80);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}

/*
 *	Minor support functions
 */
static int
MoneyZero (
 double	m)
{
	return (fabs (m) < 0.0001);
}

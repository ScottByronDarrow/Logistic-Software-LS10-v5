/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_acc_prt.c,v 5.4 2002/07/17 09:57:51 scott Exp $
|  Program Desc  : (Accumulative Lead Time Print Program)
|---------------------------------------------------------------------|
|  Author        : Aroha Merrilees   Date Written  : 20/05/94         |
|---------------------------------------------------------------------|
| $Log: sk_acc_prt.c,v $
| Revision 5.4  2002/07/17 09:57:51  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.3  2001/08/09 09:17:54  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:44:32  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:18:42  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_acc_prt.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_acc_prt/sk_acc_prt.c,v 5.4 2002/07/17 09:57:51 scott Exp $";

#include	<pslscr.h>
#include	<ml_std_mess.h>
#include	<ml_sk_mess.h>
#include	<dsp_screen.h>
#include	<dsp_process.h>
#include	<number.h>
#include	<get_lpno.h>
#include	<twodec.h>
#include	<Costing.h>

#define	DETAIL		(local_rec.det_sum [0] == 'D')
#define	PRINT		(local_rec.prt_dsp [0] == 'P')
#define NEWPAGE		(local_rec.new_page [0] == 'Y')

		
char	*j_line = "^^GGGGGJGGGGGGGGGGGGGGGGGGJGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGJGGGGGGJGGGGGGGGGGGGGGGGJGGGGGGGGGGJGGGGGGGGGGJGGGGGGGGGG";
char	*i_line = "^^GGGGGIGGGGGGGGGGGGGGGGGGIGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGIGGGGGGIGGGGGGGGGGGGGGGGIGGGGGGGGGGIGGGGGGGGGGIGGGGGGGGGG";
char	*g_line = "^^GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG";

#include	"schema"

struct commRecord	comm_rec;
struct ccmrRecord	ccmr_rec;
struct inmrRecord	inmr_rec;
struct inmrRecord	inmr2_rec;
struct inccRecord	incc_rec;
struct bmmsRecord	bmms_rec;
struct rghrRecord	rghr_rec;
struct rglnRecord	rgln_rec;
struct inumRecord	inum_rec;

	char	*data	= "data",
			*inmr2	= "inmr2";

FILE	*fout;
int		firstTime;
int		LOWER_LEVELS;					/* Update lower manufactured items. */
double	quantity;
double	rtg_leadtime;

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char	startItem [17];
	char	startDesc [41];
	char	endItem [17];
	char	endDesc [41];
	int		bom_no;
	int		rtg_no;
	char	det_sum [2];
	char	det_sum_desc [8];
	char	prt_dsp [2];
	char	prt_dsp_desc [8];
	char	new_page [2];
	char	new_page_desc [4];
	int		printerNo;
	char 	dummy [11];

	char	std_uom [5];
	char	alt_uom [5];
	char	systemDate [11];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "startItem",	 4, 30, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "           ",
		" ", " ", " Start Item               : ", "Start Manufactured Item - Full Search Available",
		YES, NO,  JUSTLEFT, "", "", local_rec.startItem},
	{1, LIN, "startDesc",	 5, 30, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "           ",
		" ", " ", " Start Item Description   : ", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.startDesc},
	{1, LIN, "endItem",	 6, 30, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "           ",
		" ", "~~~~~~~~~~~~~~~~", " End Item                 : ", "End Manufactured Item - Full Search Available",
		YES, NO,  JUSTLEFT, "", "", local_rec.endItem},
	{1, LIN, "endDesc",	 7, 30, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU", "           ",
		" ", " ", " End Item Description     : ", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.endDesc},
	{1, LIN, "bom_no",	9, 30, INTTYPE,
		"NNNNN", "          ",
		" ", "1", " BOM Number               : ", "Default BOM Number.",
		YES, NO, JUSTRIGHT, "0", "32767", (char *)&local_rec.bom_no},
	{1, LIN, "rtg_no",	10, 30, INTTYPE,
		"NNNNN", "          ",
		" ", "1", " RTG Number               : ", "Default RTG Number.",
		YES, NO, JUSTRIGHT, "0", "32767", (char *)&local_rec.rtg_no},
	{1, LIN, "det_sum",	12, 30, CHARTYPE,
		"U", "          ",
		" ", "D", " Detail/Summary           : ", "(D)etails or (S)ummary. Default - Detail.",
		YES, NO, JUSTLEFT, "DS", "", local_rec.det_sum},
	{1, LIN, "det_sum_desc",	12, 33, CHARTYPE,
		"AAAAAAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.det_sum_desc},
	{1, LIN, "prt_dsp",	13, 30, CHARTYPE,
		"U", "          ",
		" ", "D", " Print/Display            : ", "(P)rint or (D)isplay. Default - Display.",
		YES, NO, JUSTLEFT, "DP", "", local_rec.prt_dsp},
	{1, LIN, "prt_dsp_desc",	13, 33, CHARTYPE,
		"AAAAAAA", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.prt_dsp_desc},
	{1, LIN, "new_page",	15, 30, CHARTYPE,
		"U", "          ",
		" ", "N", " New Page Per Parent      : ", "(Y)es or (N)o. Default - No.",
		YES, NO, JUSTLEFT, "NY", "", local_rec.new_page},
	{1, LIN, "new_page_desc",	15, 33, CHARTYPE,
		"UUU", "          ",
		" ", "", "", "",
		NA, NO, JUSTLEFT, "", "", local_rec.new_page_desc},
	{1, LIN, "printerNo",	16, 30, INTTYPE,
		"NN", "          ",
		" ", "1", " Printer Number           : ", "Full Search Available.",
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.printerNo},

	{0, LIN, "",	 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*=======================
| Function Declarations |
=======================*/
double 	CalcMaterials 		(long, int, float);
double 	UpdateMfgItem 		(long, int, float, int, float);
float 	GetUom 				(long);
int  	check_page 			(void);
int  	heading 			(int);
int  	spec_valid 			(int);
void 	CalcResources		(long, double *, double *, float);
void 	CloseDB 			(void);
void 	InitOutput 			(void);
void 	OpenDB 				(void);
void 	PrintHeading 		(void);
void 	PrintMaterial 		(int, double, int);
void 	PrintMfgTotal 		(int, double);
void 	PrintParent 		(void);
void 	PrintTotal 			(long, double, double);
void 	ProcAccLeadTime		(void);
void 	shutdown_prog 		(void);

extern	int	manufacturingSrch;
			
/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int argc,
 char * argv [])
{
	SETUP_SCR (vars);

	strcpy (local_rec.systemDate, DateToString (TodaysDate()));

	OpenDB ();

	init_scr ();			/*  sets terminal from termcap	*/
	set_tty ();
	set_masks ();			/*  setup print using masks	*/
	init_vars (1);			/*  set default values		*/

	while (prog_exit == 0)
	{
		manufacturingSrch	= TRUE;
		entry_exit 			= FALSE;
		edit_exit 			= FALSE;
		prog_exit 			= FALSE;
		restart 			= FALSE;
		search_ok 			= TRUE;
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
		InitOutput ();

		ProcAccLeadTime ();

		if (PRINT)
		{
			fprintf (fout, ".EOF\n");
			pclose (fout);
		}
		else
		{
			Dsp_srch ();
			Dsp_close ();
		}
	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}
	
/*=========================
| Program exit sequence	. |
=========================*/
void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (
 void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (ccmr,  ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");

	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, comm_rec.est_no);
	strcpy (ccmr_rec.cc_no, comm_rec.cc_no);
	cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");

	abc_fclose (ccmr);

	abc_alias (inmr2, inmr);

	open_rec (inmr,  inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (inmr2, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (incc,  incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (bmms,  bmms_list, BMMS_NO_FIELDS, "bmms_id_no");
	open_rec (rghr,  rghr_list, RGHR_NO_FIELDS, "rghr_id_no");
	open_rec (rgln,  rgln_list, RGLN_NO_FIELDS, "rgln_id_no");
	open_rec (inum,  inum_list, INUM_NO_FIELDS, "inum_hhum_hash");

}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (inmr);
	abc_fclose (inmr2);
	abc_fclose (incc);
	abc_fclose (bmms);
	abc_fclose (rghr);
	abc_fclose (rgln);
	abc_fclose (inum);

	SearchFindClose ();
	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
	if (LCHECK ("startItem"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.startDesc,ML ("First Manufactured Item"));
			DSP_FLD ("startDesc");
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
			cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			print_mess (ML(mlStdMess001));
			sleep (sleepTime);
			clear_mess ();
			sprintf (local_rec.startDesc, "%-40.40s", " ");
			DSP_FLD ("startDesc");
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();
		strcpy (local_rec.startItem, inmr_rec.item_no);

		incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
		incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
		cc = find_rec (incc, &incc_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML(mlSkMess419));
			sleep (sleepTime);
			clear_mess ();
			sprintf (local_rec.startDesc, "%-40.40s", " ");
			DSP_FLD ("startDesc");
			return (EXIT_FAILURE);
		}

		/*---------------------------------------
		| Check if item is a manufactured item. |
		---------------------------------------*/
		if (strcmp (inmr_rec.source, "BP") &&
			strcmp (inmr_rec.source, "BM") &&
			strcmp (inmr_rec.source, "MC") &&
			strcmp (inmr_rec.source, "MP"))
		{
			print_mess (ML(mlSkMess531));
			sleep (sleepTime);
			clear_mess ();
			sprintf (local_rec.startDesc, "%-40.40s", " ");
			DSP_FLD ("startDesc");
			return (EXIT_FAILURE);
		}

		if (prog_status != ENTRY &&
			strcmp (local_rec.startItem, local_rec.endItem) > 0)
		{
			print_mess (ML(mlStdMess017));
			sleep (sleepTime);
			clear_mess ();
			sprintf (local_rec.startDesc, "%-40.40s", " ");
			DSP_FLD ("startDesc");
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.startDesc, inmr_rec.description);
		DSP_FLD ("startDesc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("endItem"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.endDesc, ML ("Last Manufactured Item"));
			DSP_FLD ("endDesc");
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
			cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			print_mess (ML(mlStdMess001));
			sleep (sleepTime);
			clear_mess ();
			sprintf (local_rec.endDesc, "%-40.40s", " ");
			DSP_FLD ("endDesc");

			return (EXIT_FAILURE);
		}
		SuperSynonymError ();
		strcpy (local_rec.endItem, inmr_rec.item_no);

		incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
		incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
		cc = find_rec (incc, &incc_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML(mlSkMess419));
			sleep (sleepTime);
			clear_mess ();
			sprintf (local_rec.endDesc, "%-40.40s", " ");
			DSP_FLD ("endDesc");
			return (EXIT_FAILURE);
		}

		/*---------------------------------------
		| Check if item is a manufactured item. |
		---------------------------------------*/
		if (strcmp (inmr_rec.source, "BP") &&
			strcmp (inmr_rec.source, "BM") &&
			strcmp (inmr_rec.source, "MC") &&
			strcmp (inmr_rec.source, "MP"))
		{
			print_mess (ML(mlSkMess531));
			sleep (sleepTime);
			clear_mess ();
			sprintf (local_rec.endDesc, "%-40.40s", " ");
			DSP_FLD ("startDesc");
			return (EXIT_FAILURE);
		}

		if (strcmp (local_rec.startItem, local_rec.endItem) > 0)
		{
			print_mess (ML(mlStdMess017));
			sleep (sleepTime);
			clear_mess ();
			sprintf (local_rec.endDesc, "%-40.40s", " ");
			DSP_FLD ("endDesc");
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.endDesc, inmr_rec.description);
		DSP_FLD ("endDesc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("det_sum"))
	{
		if (local_rec.det_sum [0] == 'D')
			strcpy (local_rec.det_sum_desc, ML ("Detail "));
		else
			strcpy (local_rec.det_sum_desc, ML ("Summary"));

		DSP_FLD ("det_sum_desc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("prt_dsp"))
	{
		if (local_rec.prt_dsp [0] == 'P')
		{
			strcpy (local_rec.prt_dsp_desc, ML ("Print"));
			FLD ("new_page") = YES;
			FLD ("printerNo") = YES;
			if (prog_status != ENTRY)
			{
				DSP_FLD ("prt_dsp_desc");
				do
				{
					get_entry (label ("new_page"));
					cc = spec_valid (label ("new_page"));
				} while (cc && !restart);
				do
				{
					get_entry (label ("printerNo"));
					cc = spec_valid (label ("printerNo"));
				} while (cc && !restart);
			}
		}
		else
		{
			strcpy (local_rec.prt_dsp_desc, ML ("Display"));
			FLD ("new_page") = NA;
			strcpy (local_rec.new_page_desc, " ");
			DSP_FLD ("new_page_desc");
			FLD ("printerNo") = NA;
			local_rec.printerNo = 0;
			DSP_FLD ("printerNo");
		}

		DSP_FLD ("prt_dsp_desc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("new_page"))
	{
		if (F_NOKEY (field))
			return (EXIT_SUCCESS);

		if (local_rec.new_page [0] == 'Y')
			strcpy (local_rec.new_page_desc, ML ("Yes"));
		else
			strcpy (local_rec.new_page_desc, ML ("No "));

		DSP_FLD ("new_page_desc");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("printerNo"))
	{
		if (F_NOKEY (field))
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			local_rec.printerNo = get_lpno (0);
			return (EXIT_SUCCESS);
		}
	
		if (!valid_lp (local_rec.printerNo))
		{
			print_mess (ML(mlStdMess020));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		DSP_FLD ("printerNo");

		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

/*-------------------------------------------------------------
| Process selected items. If the item is a manufactured item, |
| calculate the accumulative lead time.                       |
-------------------------------------------------------------*/
void
ProcAccLeadTime (
 void)
{
	firstTime = TRUE;
	PrintHeading ();

	strcpy (inmr_rec.co_no, comm_rec.co_no);
	strcpy (inmr_rec.item_no, local_rec.startItem);
	cc = find_rec (inmr, &inmr_rec, GTEQ, "r");
	/* Read for all the items within the users selection. */
	while (!cc &&
		strcmp (inmr_rec.item_no, local_rec.startItem) >= 0 &&
		strcmp (inmr_rec.item_no, local_rec.endItem) <= 0)
	{
		incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
		incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
		cc = find_rec (incc, &incc_rec, COMPARISON, "r");

		if (!cc &&
			(!strcmp (inmr_rec.source, "BP") ||
			!strcmp (inmr_rec.source, "BM") ||
			!strcmp (inmr_rec.source, "MC") ||
			!strcmp (inmr_rec.source, "MP")))
		{
			cc = FindInei (inmr_rec.hhbr_hash, comm_rec.est_no, "r");
			if (cc)
				ineiRec.std_batch = 1.00;

			UpdateMfgItem 
			(
				inmr_rec.hhbr_hash,
				TRUE,
				ineiRec.std_batch,
				1,
				1.00
			);
		}
		
		cc = find_rec (inmr, &inmr_rec, NEXT, "r");
	}
}

/*----------------------------------------------
| Calculate the accumulative lead time of the  |
| manufactured item, down to the lowest level. |
----------------------------------------------*/
double 
UpdateMfgItem (
	long 	hhbrHash, 
	int 	updateFlag, 
	float 	requiredQty, 
	int 	levelNo, 
	float 	cnvFct)
{
	double	mat_lt = 0.00;		/* Total Material Lead Time */
	double	rtg_lt = 0.00;		/* Total Resource Lead Time - batch size */
	double	rtg_req = 0.00;		/* Total Resource Lead Time - required qty */

	strcpy (bmms_rec.co_no, comm_rec.co_no);
	bmms_rec.hhbr_hash = hhbrHash;
	bmms_rec.alt_no = local_rec.bom_no;
	bmms_rec.line_no = 0;
	cc = find_rec (bmms, &bmms_rec, GTEQ, "r");
	if (cc ||
		strcmp (bmms_rec.co_no, inmr_rec.co_no) ||
		bmms_rec.hhbr_hash != hhbrHash ||
		bmms_rec.alt_no != local_rec.bom_no)
	{
		return (-1.00);
	}

	strcpy (rghr_rec.co_no, comm_rec.co_no);
	strcpy (rghr_rec.br_no, comm_rec.est_no);
	rghr_rec.hhbr_hash = hhbrHash;
	rghr_rec.alt_no = local_rec.rtg_no;
	cc = find_rec (rghr, &rghr_rec, COMPARISON, "r");
	if (cc)
		return (-1.00);

	/* find item details */
	cc = find_hash (inmr2, &inmr2_rec, COMPARISON, "r", hhbrHash);
	if (cc)
		file_err (cc, inmr, "DBFIND");

	if (levelNo == 1) /* parent manufacturing item */
	{
		/* find std uom */
		cc = find_hash (inum, &inum_rec, EQUAL, "r", inmr2_rec.std_uom);
		if (cc)
			strcpy (inum_rec.uom, "UNKN");
		strcpy (local_rec.std_uom, inum_rec.uom);

		/* find alt uom */
		cc = find_hash (inum, &inum_rec, EQUAL, "r", inmr2_rec.alt_uom);
		if (cc)
			strcpy (inum_rec.uom, "UNKN");
		strcpy (local_rec.alt_uom, inum_rec.uom);

		/* find std batch size */
		cc = FindInei (inmr2_rec.hhbr_hash, comm_rec.est_no, "r");
		if (cc)
			ineiRec.std_batch = 1.00;

		/* find last bom & rtg numbers, and lead time */
		incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
		incc_rec.hhbr_hash = inmr2_rec.hhbr_hash;
		cc = find_rec (incc, &incc_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, incc, "DBFIND");

		if (!firstTime && PRINT && NEWPAGE)
			fprintf (fout, ".PA");
		PrintParent ();
	}
	else
		if (DETAIL)
			PrintMaterial (levelNo - 1, mat_lt, TRUE);

	if (PRINT)
		dsp_process ("Item Number :", inmr2_rec.item_no);

	mat_lt = CalcMaterials (hhbrHash, levelNo, cnvFct);

	CalcResources (hhbrHash, &rtg_lt, &rtg_req, requiredQty);

	if (levelNo == 1)
		PrintTotal (hhbrHash, mat_lt, rtg_lt);

	if (firstTime)
		firstTime = FALSE;

	rtg_leadtime = twodec (rtg_req);

	return (twodec (mat_lt) + twodec (rtg_req));
}

/*-------------------------------------------------
| Calculates the total material lead times at all |
| levels of the manufactured item passed.         |
| Returns the total material lead time (includes  |
| lower manufactured items total costs (both      |
| material and resource costs).                   |
-------------------------------------------------*/
double 
CalcMaterials (
	long 	hhbrHash, 
	int 	levelNo, 
	float 	cnvFct)
{
	double	total_lead_time = 0.00,
			lead_time = 0.00;
	int		line_no,
			flag;
	float	cnv_fct,
			req_qty;

	strcpy (bmms_rec.co_no, inmr_rec.co_no);
	bmms_rec.hhbr_hash = hhbrHash;
	bmms_rec.alt_no = local_rec.bom_no;
	bmms_rec.line_no = 0;
	cc = find_rec (bmms, &bmms_rec, GTEQ, "r");
	while (!cc &&
		!strcmp (bmms_rec.co_no, inmr_rec.co_no) &&
		bmms_rec.hhbr_hash == hhbrHash &&
		bmms_rec.alt_no == local_rec.bom_no)
	{
		flag = TRUE;
		inmr2_rec.hhbr_hash = bmms_rec.mabr_hash;
		cc = find_rec (inmr2, &inmr2_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, inmr2, "DBFIND");

		if (!strcmp (inmr2_rec.source, "BP") ||
			!strcmp (inmr2_rec.source, "BM") ||
			!strcmp (inmr2_rec.source, "MC") ||
			!strcmp (inmr2_rec.source, "MP"))
		{
			line_no = bmms_rec.line_no;

			/* calculate required quantity in the std uom */
			cnv_fct = GetUom (bmms_rec.uom);
			req_qty = bmms_rec.matl_wst_pc;
			req_qty += 100.00;
			req_qty /= 100.00;
			req_qty *= bmms_rec.matl_qty;
			req_qty /= cnv_fct; /* divid by cnv_fct to get std uom qty */

			cc = FindInei (bmms_rec.mabr_hash, comm_rec.est_no, "r");
			if (cc)
				ineiRec.std_batch = 1.00;

			/* conversion factor for std batch and required quantities */
			cnv_fct = (float) req_qty;
			cnv_fct /= ineiRec.std_batch;

			lead_time = UpdateMfgItem (bmms_rec.mabr_hash,
					LOWER_LEVELS,
					req_qty,
					levelNo + 1,
					cnv_fct);

			if (lead_time < 0.00)
				flag = TRUE;
			else
				flag = FALSE;

			strcpy (bmms_rec.co_no, comm_rec.co_no);
			bmms_rec.hhbr_hash = hhbrHash;
			bmms_rec.alt_no = local_rec.bom_no;
			bmms_rec.line_no = line_no;
			cc = find_rec (bmms, &bmms_rec, COMPARISON, "r");
			if (cc)
				file_err (cc, bmms, "DBFIND");

			inmr2_rec.hhbr_hash = bmms_rec.mabr_hash;
			cc = find_rec (inmr2, &inmr2_rec, COMPARISON, "r");
			if (cc)
				file_err (cc, inmr2, "DBFIND");
		}
		
		if (flag)
		{
			/*-----------------------------------
			| Read the lead time for this item. |
			-----------------------------------*/
			incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
			incc_rec.hhbr_hash = bmms_rec.mabr_hash;
			cc = find_rec (incc, &incc_rec, COMPARISON, "r");
			if (cc)
				lead_time = 0.00;
			else
				lead_time = incc_rec.lead_time;
		}

		if (DETAIL || levelNo == 1)
		{
			cc = find_hash (inum, &inum_rec, EQUAL, "r", bmms_rec.uom);
			if (cc)
				strcpy (inum_rec.uom, "UNKN");

			quantity = (double) bmms_rec.matl_wst_pc;
			quantity += 100.00;
			quantity /= 100.00;
			quantity *= (double) bmms_rec.matl_qty;
			quantity *= (double) cnvFct;
			
			if (flag)
				PrintMaterial (levelNo, lead_time, FALSE);
			else
				PrintMfgTotal (levelNo, lead_time);
		}

		if (lead_time > total_lead_time)
			total_lead_time = twodec (lead_time);

		cc = find_rec (bmms, &bmms_rec, NEXT, "r");
	}

	return (twodec (total_lead_time));
}

float 
GetUom (
 long _hhum_hash)
{
	char	std_group [21],
			alt_group [21];
	number	std_cnv_fct,
			alt_cnv_fct,
			cnv_fct,
			result,
			uom_cfactor;

	/*-------------------------------
	| Get the UOM conversion factor	|
	-------------------------------*/
	cc = find_hash (inum, &inum_rec, EQUAL, "r", inmr2_rec.alt_uom);
	if (cc)
		file_err (cc, inum, "DBFIND");
	sprintf (alt_group, "%-20.20s", inum_rec.uom_group);
	/*------------------------------------------------
	| converts a float to arbitrary precision number |
	| defined as number.                             |
	------------------------------------------------*/
	NumFlt (&alt_cnv_fct, inum_rec.cnv_fct);

	cc = find_hash (inum, &inum_rec, EQUAL, "r", inmr2_rec.std_uom);
	if (cc)
		file_err (cc, inum, "DBFIND");
	sprintf (std_group, "%-20.20s", inum_rec.uom_group);
	/*------------------------------------------------
	| converts a float to arbitrary precision number |
	| defined as number.                             |
	------------------------------------------------*/
	NumFlt (&std_cnv_fct, inum_rec.cnv_fct);

	cc = find_hash (inum, &inum_rec, EQUAL, "r", _hhum_hash);
	if (cc)
		file_err (cc, inum, "DBFIND");
	/*------------------------------------------------
	| converts a float to arbitrary precision number |
	| defined as number.                             |
	------------------------------------------------*/
	NumFlt (&cnv_fct, inum_rec.cnv_fct);

	/*----------------------------------------------------------
	| a function that divides one number by another and places |
	| the result in another number defined variable            |
	----------------------------------------------------------*/
	if (strcmp (alt_group, inum_rec.uom_group))
		NumDiv (&std_cnv_fct, &cnv_fct, &result);
	else
	{
		NumFlt (&uom_cfactor, inmr2_rec.uom_cfactor);
		NumDiv (&alt_cnv_fct, &cnv_fct, &result);
		NumMul (&result, &uom_cfactor, &result);
	}

	/*---------------------------------------
	| converts a arbitrary precision number |
	| to a float                            |
	---------------------------------------*/
	return (NumToFlt (&result));
}

/*---------------------------------------------------
| Calculates the total resource lead time at all    |
| levels of the manufactured item passed.           |
| Returns the total resource lead time for the item |
| hash  passed.                                     |
---------------------------------------------------*/
void
CalcResources (
	long	hhbrHash, 
	double	*_rtg_lt, 
	double	*_rtg_req, 
	float	_req_qty)
{
	long	std_time = 0L,
			req_time = 0L;

	cc = FindInei (hhbrHash, comm_rec.est_no, "r");
	if (cc)
		ineiRec.std_batch = 1.00;

	strcpy (rghr_rec.co_no, comm_rec.co_no);
	strcpy (rghr_rec.br_no, comm_rec.est_no);
	rghr_rec.hhbr_hash = hhbrHash;
	rghr_rec.alt_no = local_rec.rtg_no;
	cc = find_rec (rghr, &rghr_rec, COMPARISON, "r");
	if (!cc)
	{
		rgln_rec.hhgr_hash = rghr_rec.hhgr_hash;
		rgln_rec.seq_no = 0;
		cc = find_rec (rgln, &rgln_rec, GTEQ, "r");
		while (!cc &&
			rgln_rec.hhgr_hash == rghr_rec.hhgr_hash)
		{
			/* Total time for the standard batch size. */
			std_time += rgln_rec.setup;
			std_time += rgln_rec.run;
			std_time += rgln_rec.clean;

			/* Total time for the required quantity. */
			rgln_rec.run *= _req_qty;
			rgln_rec.run /= ineiRec.std_batch;
			req_time += rgln_rec.setup;
			req_time += rgln_rec.run;
			req_time += rgln_rec.clean;

			cc = find_rec (rgln, &rgln_rec, NEXT, "r");
		}
	}
	*_rtg_lt = ((double) std_time / 60.00) / 40.00;
	*_rtg_req = ((double) req_time / 60.00) / 40.00;
}

void
PrintParent (
 void)
{
	if (PRINT)
	{
		fprintf (fout, "====================");
		fprintf (fout, "====================");
		fprintf (fout, "====================");
		fprintf (fout, "====================");
		fprintf (fout, "====================");
		fprintf (fout, "====================");
		fprintf (fout, "======\n");

		fprintf (fout,
				"| PARENT NUMBER : %-16.16s%14s", inmr2_rec.item_no, " ");
		fprintf (fout, "%-40.40s%37s|\n", inmr2_rec.description, " ");

		fprintf (fout, "| DFLT BOM      : %5d%25s", incc_rec.dflt_bom, " ");
		fprintf (fout, "DFLT RTG      : %5d%56s|\n", incc_rec.dflt_rtg, " ");

		fprintf (fout, "| STD UOM       : %-4.4s%26s", local_rec.std_uom, " ");
		fprintf (fout, "ALT UOM       : %-4.4s%26s", local_rec.alt_uom, " ");
		fprintf (fout, "STD BATCH     : %14.6f |\n", ineiRec.std_batch);

		fprintf (fout, "| LAST BOM      : %5d%25s", incc_rec.last_bom, " ");
		fprintf (fout, "LAST RTG      : %5d%25s", incc_rec.last_rtg, " ");
		fprintf (fout, "LEAD TIME     : %6.2f         |\n", incc_rec.lead_time);

		fprintf (fout, "|-------------------");
		fprintf (fout, "--------------------");
		fprintf (fout, "--------------------");
		fprintf (fout, "--------------------");
		fprintf (fout, "--------------------");
		fprintf (fout, "--------------------");
		fprintf (fout, "-----|\n");

		fprintf (fout, "|LEVEL");
		fprintf (fout, "|                  ");
		fprintf (fout, "|                                          ");
		fprintf (fout, "|      ");
		fprintf (fout, "|    QUANTITY    ");
		fprintf (fout, "|COMPONENT ");
		fprintf (fout, "|HIGH COMP.");
		fprintf (fout, "|TOTAL RES |\n");

		fprintf (fout, "| NO  ");
		fprintf (fout, "|   COMPONENT NO   ");
		fprintf (fout, "|               DESCRIPTION                ");
		fprintf (fout, "| UOM  ");
		fprintf (fout, "|    REQUIRED    ");
		fprintf (fout, "| LEADTIME ");
		fprintf (fout, "| LEADTIME ");
		fprintf (fout, "| LEADTIME |\n");

		fprintf (fout, "|-----");
		fprintf (fout, "|------------------");
		fprintf (fout, "|------------------------------------------");
		fprintf (fout, "|------");
		fprintf (fout, "|----------------");
		fprintf (fout, "|----------");
		fprintf (fout, "|----------");
		fprintf (fout, "|----------|\n");
	}
	else
	{
		sprintf (err_str,
			"PARENT NUMBER : %-16.16s    %-40.40s",
			inmr2_rec.item_no,
			inmr2_rec.description);
		Dsp_saverec (err_str);

		sprintf (err_str,
			"DFLT BOM      : %5d%15sDFLT RTG      : %5d",
			incc_rec.dflt_bom, " ",
			incc_rec.dflt_rtg);
		Dsp_saverec (err_str);

		sprintf (err_str,
			"STD UOM       : %-4.4s%16sALT UOM       : %-4.4s%16sSTD BATCH     : %14.6f",
			local_rec.std_uom, " ",
			local_rec.alt_uom, " ",
			ineiRec.std_batch);
		Dsp_saverec (err_str);

		sprintf (err_str,
			"LAST BOM      : %5d%15sLAST RTG      : %5d%15sLEAD TIME     : %6.2f",
			incc_rec.last_bom, " ",
			incc_rec.last_rtg, " ",
			incc_rec.lead_time);
		Dsp_saverec (err_str);

		Dsp_saverec (i_line); /* displays a line */
	}
}

void
PrintMaterial (
 int _level, 
 double _mat_lt, 
 int flag)
{
	if (PRINT)
	{
		fprintf (fout, "| %s%2d ",			flag ? "*" : " ", _level);
		fprintf (fout, "| %-16.16s ",		inmr2_rec.item_no);
		fprintf (fout, "| %-40.40s ",		inmr2_rec.description);
		if (flag)
		{
			fprintf (fout, "| %4s ",		" ");
			fprintf (fout, "  %14s ",		" ");
			fprintf (fout, "   %6s  ",		" ");
		}
		else
		{
			fprintf (fout, "| %-4.4s ",		inum_rec.uom);
			fprintf (fout, "| %14.6f ",		quantity);
			fprintf (fout, "|  %6.2f  ",	_mat_lt);
		}
		fprintf (fout, "%s%21s|\n",			flag ? " " : "|", " ");
	}
	else
	{
		if (flag)
		{
			sprintf (err_str,
					" *%2d ^E %-16.16s ^E %-40.40s ^E ",
					_level,
					inmr2_rec.item_no,
					inmr2_rec.description);
		}
		else
		{
			sprintf (err_str,
					"  %2d ^E %-16.16s ^E %-40.40s ^E %-4.4s ^E %14.6f ^E  %6.2f  ^E ",
					_level,
					inmr2_rec.item_no,
					inmr2_rec.description,
					inum_rec.uom,
					quantity,
					_mat_lt);
		}
		Dsp_saverec (err_str);
	}
}

void
PrintMfgTotal (
 int _level, 
 double _tot_lt)
{
	if (PRINT)
	{
		fprintf (fout, "| *%2d ",		_level);
		fprintf (fout, "| %-16.16s ",	inmr2_rec.item_no);
		fprintf (fout, "| %-40.40s ",	inmr2_rec.description);
		fprintf (fout, "| %-4.4s ",		inum_rec.uom);
		fprintf (fout, "| %14.6f ",		quantity);
		fprintf (fout, "|  %6.2f  ",	twodec (_tot_lt));
		fprintf (fout, "|  %6.2f  ",
				twodec (_tot_lt) - twodec (rtg_leadtime));
		fprintf (fout, "|  %6.2f  |\n",	twodec (rtg_leadtime));
	}
	else
	{
		sprintf (err_str,
				" *%2d ^E %-16.16s ^E %-40.40s ^E %-4.4s ^E %14.6f ^E  %6.2f  ^E  %6.2f  ^E  %6.2f  ",
				_level,
				inmr2_rec.item_no,
				inmr2_rec.description,
				inum_rec.uom,
				quantity,
				twodec (_tot_lt),
				twodec (_tot_lt) - twodec (rtg_leadtime),
				twodec (rtg_leadtime));
		Dsp_saverec (err_str);
	}
}

void
PrintTotal (
 long _hhbr_hash, 
 double _mat_lt, 
 double _rtg_lt)
{
	cc = find_hash (inmr2, &inmr2_rec, COMPARISON, "r", _hhbr_hash);
	if (cc)
		file_err (cc, inmr2, "DBFIND");

	if (PRINT)
	{
		dsp_process ("Item Number :",	inmr2_rec.item_no);

		fprintf (fout, "|===================");
		fprintf (fout, "====================");
		fprintf (fout, "====================");
		fprintf (fout, "====================");
		fprintf (fout, "====================");
		fprintf (fout, "====================");
		fprintf (fout, "=====|\n");

		fprintf (fout, "|%124s|\n", 	" ");

		fprintf (fout, "|                   ");
		fprintf (fout, "                 ");
		fprintf (fout, "                       ");
		fprintf (fout, "|HIGH COMP. LT ");
		fprintf (fout, "| TOTAL RES LT ");
		fprintf (fout, "| ACC LEADTIME ");
		fprintf (fout, "|                   |\n");

		fprintf (fout, "|                   ");
		fprintf (fout, "PARENT LEAD TIME ");
		fprintf (fout, ":   %-16.16s   ",	inmr2_rec.item_no);
		fprintf (fout, "|    %6.2f    ",	twodec (_mat_lt));
		fprintf (fout, "|    %6.2f    ",	twodec (_rtg_lt));
		fprintf (fout, "|    %6.2f    ",
				twodec (_mat_lt) + twodec (_rtg_lt));
		fprintf (fout, "|                   |\n");

		fprintf (fout, "|%124s|\n", 	" ");

		fprintf (fout, "====================");
		fprintf (fout, "====================");
		fprintf (fout, "====================");
		fprintf (fout, "====================");
		fprintf (fout, "====================");
		fprintf (fout, "====================");
		fprintf (fout, "======\n");
	}
	else
	{
		Dsp_saverec (j_line);

		sprintf (err_str,
			"                                           ^E  HIGH COMP. LT   ^E   TOTAL RES LT   ^E   ACC LEADTIME   ^E");
		Dsp_saverec (err_str);

		sprintf (err_str,
			"    PARENT LEAD TIME : %-16.16s    ^E      %6.2f      ^E      %6.2f      ^E      %6.2f      ^E",
			inmr2_rec.item_no,
			twodec (_mat_lt),
			twodec (_rtg_lt),
			twodec (_mat_lt) + twodec (_rtg_lt));
		Dsp_saverec (err_str);

		Dsp_saverec (g_line);
	}
}

/*===========================================
| Initialize Output for Display or Printer. |
===========================================*/
void
InitOutput (
 void)
{
	if (PRINT)
	{
		dsp_screen (" Printing Accumulative Lead Time Report",
					comm_rec.co_no, comm_rec.co_name);

		/*----------------------
		| Open pipe to pformat | 
 		----------------------*/
		if ((fout = popen ("pformat", "w")) == NULL)
			sys_err ("Error in pformat During (POPEN)", cc, PNAME);
	
		/*---------------------------------
		| Initialize printer for output.  |
		---------------------------------*/
		fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate()), PNAME);
		fprintf (fout, ".LP%d\n", local_rec.printerNo);
		fprintf (fout, ".12\n");
		fprintf (fout, ".PI12\n");
		fprintf (fout, ".L130\n");
	}
	else
	{
		clear ();
		rv_pr (ML(mlSkMess146) , 22, 0, 1);

 		print_at (2, 5, ML(mlStdMess038), comm_rec.co_no, comm_rec.co_name);
		print_at (3, 5, ML(mlSkMess206), local_rec.startItem,local_rec.endItem);
		print_at (4, 5, ML(mlSkMess207), local_rec.bom_no, local_rec.rtg_no);
		print_at (5, 5, ML(mlSkMess208), local_rec.det_sum_desc);
	}
}

void
PrintHeading (
 void)
{
	if (PRINT)
	{
		if (DETAIL)
			fprintf (fout, ".E ACCUMULATIVE LEAD TIME DETAIL REPORT \n");
		else
			fprintf (fout, ".E ACCUMULATIVE LEAD TIME SUMMARY REPORT \n");
		fprintf (fout, ".E COMPANY   : %s \n", clip (comm_rec.co_name));
		fprintf (fout, ".E BRANCH    : %s \n", clip (comm_rec.est_name));
		fprintf (fout, ".E WAREHOUSE : %s \n", clip (comm_rec.cc_name));
		fprintf (fout, ".B1\n");
		fprintf (fout,
			".E START ITEM : %-16.16s  END ITEM : %-16.16s \n",
			local_rec.startItem,
			local_rec.endItem);
		fprintf (fout,
			".E BOM NUMBER : %5d  ROUTING NUMBER : %5d \n",
			local_rec.bom_no,
			local_rec.rtg_no);
		fprintf (fout,
			".E NEW PAGE : %s \n",
			local_rec.new_page_desc);
		fprintf (fout, ".B1\n");
	}
	else
	{
		Dsp_nc_prn_open (4, 6, 11, err_str, 
			comm_rec.co_no, comm_rec.co_name, 
			comm_rec.est_no, comm_rec.est_name, 
			(char *)0,  (char *)0);

		/* Title for the screen display. */
		Dsp_saverec ("LEVEL|                  |                                          |      |    REQUIRED    |COMPONENT |HIGH COMP.|TOTAL RES ");
		Dsp_saverec (" NO  |   COMPONENT NO   |               DESCRIPTION                | UOM  |    QUANTITY    | LEADTIME | LEADTIME | LEADTIME ");
		Dsp_saverec (" [REDRAW] [PRINT] [NEXT] [PREV] [EDIT/END] ");
	}
}

/*=====================================================
| check if a new page is needed on screen or printer. |
=====================================================*/
int
check_page (
 void)
{
	return (EXIT_SUCCESS);
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
		swide ();
		
		rv_pr (ML(mlSkMess147) ,50, 0, 1);

		box (0, 3, 132, 13);
		line_at (1 ,0,132);
		line_at (8 ,1,131);
		line_at (11,1,131);
		line_at (14,1,131);
		line_at (20,0,132);

		print_at (21,0, ML(mlStdMess038), comm_rec.co_no,  comm_rec.co_name);
		print_at (22,0, ML(mlStdMess039), comm_rec.est_no, comm_rec.est_name);

		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}


/*=====================================================================
|  Copyright (C) 1996 - 2001 Logistic Software Limited   .            |
|=====================================================================|
|  $Id: rg_maint.c,v 5.9 2002/07/24 08:39:10 scott Exp $
|  Program Name  : (rg_maint.c)
|  Program Desc  : (Routing Maintenance)
|---------------------------------------------------------------------|
|  Date Written  : (11/11/91)      | Author       : Trevor van Bremen |
|---------------------------------------------------------------------|
|  $Log: rg_maint.c,v $
|  Revision 5.9  2002/07/24 08:39:10  scott
|  Updated to ensure SetSortArray () is after set_masks
|
|  Revision 5.8  2002/07/18 07:04:59  scott
|  Updated to make sure lcount [] is being set to zero at top of while.
|
|  Revision 5.7  2002/07/03 04:27:06  scott
|  Updated to add SetSortArray () for new LS10-GUI screen features
|
|  Revision 5.6  2002/03/07 06:33:00  scott
|  ..
|
|  Revision 5.5  2002/03/07 05:01:38  scott
|  Updated to always display last version of special instructions.
|
|  Revision 5.4  2002/03/06 07:36:01  scott
|  S/C 00828 - RTGMR2-Product Routing Maintainance; WINDOWS CLIENT (1) Item Number and Wrk Cntr display error message with no delay. (2) Detail box is not aligned (3) When deleting a detail, press <backspace>, then <ctrl-D>, 2 lines are deleted.  Another way to delete which works fine is <ctrl-D>.  CHAR-BASED / WINDOWS CLIENT (4) During edit, the PSlip Method in header is sometimes enabled.  It disables when the last key pressed is F12, and view the record again.  Is it suppose to be disabled all the time ?
|
|  Revision 5.3  2002/01/14 03:59:38  scott
|  Updated to fix incorrect message on split.
|
|  Revision 5.2  2001/08/09 09:16:30  scott
|  Updated to add FinishProgram () function
|
|  Revision 5.1  2001/08/06 23:39:42  scott
|  RELEASE 5.0
|
|  Revision 5.0  2001/06/19 08:12:58  robert
|  LS10-5.0 New Release as of 19 JUNE 2001
|
|  Revision 4.1  2001/04/16 04:33:45  scott
|  Updated to add app.schema - removes code related to tables from program as it
|  allows for better quality contol.
|  Updated to add sleep delay - did not work with LS10-GUI
|  Updated to adjust screen to look better with LS10-GUI
|  Updated to perform routine maintenance to ensure standards are maintained.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: rg_maint.c,v $";
char	*PROG_VERSION = "@ (#) - $Header: /usr/LS10/REPOSITORY/LS10.5/RG/rg_maint/rg_maint.c,v 5.9 2002/07/24 08:39:10 scott Exp $";

#define	TABLINES	11

#include <pslscr.h>
#include <ml_rg_mess.h>
#include <ml_std_mess.h>

#define	CURR_HEAD	 (seq_dtl [currentSequence].list_head)
extern	int	_win_func;

#include	"schema"

struct commRecord	comm_rec;
struct inmrRecord	inmr_rec;
struct pcidRecord	pcid_rec;
struct pcwcRecord	pcwc_rec;
struct rgbpRecord	rgbp_rec;
struct rghrRecord	rghr_rec;
struct rglnRecord	rgln_rec;
struct rgrsRecord	rgrs_rec;
struct rgylRecord	rgyl_rec;
struct ineiRecord	inei_rec;

	char	*data	= "data",
			*inmr2	= "inmr2",
			*pcwc2	= "pcwc2",
			*rgrs2	= "rgrs2";

	int		newRoute = FALSE,
			currentSequence,
			clearOK,
			inEditAll = FALSE;

	long	envVarPcTimeRes;

struct	storeRec
{
	int		seq_no;
	long	hhwcHash;
	double	rate;
	double	overHeadVariable;
	double	overHeadFixed;
	long	setup;
	long	run;
	long	clean;
	char	canSplit [2];
	int		resorceQty;
	char	type [2];
	int		instructionNo;
	char	yieldCalc [5];
} store [MAXLINES];

struct
{
	int		byProductCheck;
	char	yieldCalc [5];
	struct	SEQ_PTR	*list_head;
} seq_dtl [1000];

struct	SEQ_PTR
{
	long	hhbrHash;
	float	qty;
	struct	SEQ_PTR	*next;
	struct	SEQ_PTR	*prev;
};

#define	SEQ_NULL	 ((struct SEQ_PTR *) NULL)
struct	SEQ_PTR		*free_head = SEQ_NULL;
struct	SEQ_PTR		*free_tail = SEQ_NULL;

/*============================
| Local & Screen Structures. |
============================*/
struct
{
	char	dummy [11];
	char	printAll [5];
	char	printAllDesc [5];
	char	itemNumber [17];
	char	itemDesc [41];
	char	workCentre [9];
	double	totalCost;
	long	hhbrHash;
	long	totalTime;
	char	canSplit [2];
	char	byProduct [4];
	char	byProductItem [17];
	char	byProductDesc [41];
	float	byProductQty;
	long	byProductHhbrHash;
	int		alternateNo;
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "itemNumber",	 2, 19, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "", "Item Number: ", "",
		 NE, NO,  JUSTLEFT, "", "", local_rec.itemNumber},
	{1, LIN, "alt_no",	 2, 59, INTTYPE,
		"NNNNN", "          ",
		" ", "0", "Alternate #: ", "",
		 NE, NO, JUSTRIGHT, "0", "32767", (char *) &local_rec.alternateNo},
	{1, LIN, "printAll",	 2, 99, CHARTYPE,
		"U", "          ",
		" ", "", " PSlip Method: ", "Picking Slip shows A(ll) sequences or N(ext) sequence only ",
		 NO, NO,  JUSTLEFT, "AN", "", local_rec.printAll},
	{1, LIN, "printAllDesc",	 2, 102, CHARTYPE,
		"AAAA", "          ",
		" ", "", "", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.printAllDesc},
	{1, LIN, "hhbr",		 0, 0, LONGTYPE,
		"NNNNNNNNNN", "          ",
		"", "", "", "",
		 ND, NO,  JUSTLEFT, "", "", (char *) &local_rec.hhbrHash},
	{1, LIN, "itemDesc",	 3, 19, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description: ", "",
		 NA, NO,  JUSTLEFT, "", "", local_rec.itemDesc},
	{1, LIN, "totalTime",	 4, 19, TIMETYPE,
		"NNNNNN:NN", "          ",
		" ", "", "Time (Hrs) : ", "",
		 NA, NO, JUSTRIGHT, "", "", (char *) &local_rec.totalTime},
	{1, LIN, "totalCost",	 4, 49, MONEYTYPE,
		"NNNNNN.NN", "          ",
		" ", "", "Cost       : ", "",
		 NA, NO, JUSTRIGHT, "", "", (char *) &local_rec.totalCost},

	{2, TAB, "seq_no",	 MAXLINES, 0, INTTYPE,
		"NNN", "          ",
		" ", "1", "Seq", "Enter Duplicate Sequence numbers where concurrecny of operations is required",
		YES, NO, JUSTRIGHT, "1", "999", (char *) &rgln_rec.seq_no},
	{2, TAB, "wrk_cntr",	 0, 0, CHARTYPE,
		"UUUUUUUU", "          ",
		" ", " ", "Wrk Cntr", "",
		YES, NO,  JUSTLEFT, "", "", local_rec.workCentre},
	{2, TAB, "hhwcHash",	 0, 0, LONGTYPE,
		"NNNNNNNNNN", "          ",
		"", "", "", "",
		 ND, NO, JUSTRIGHT, "", "", (char *) &rgln_rec.hhwc_hash},
	{2, TAB, "res_code",	 0, 0, CHARTYPE,
		"UUUUUUUU", "          ",
		" ", " ", "Resource", "",
		YES, NO,  JUSTLEFT, "", "", rgrs_rec.code},
	{2, TAB, "hhrs_hash",	 0, 0, LONGTYPE,
		"NNNNNNNNNN", "          ",
		"", "", "", "",
		 ND, NO, JUSTRIGHT, "", "", (char *) &rgln_rec.hhrs_hash},
	{2, TAB, "res_desc",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		"", "", "          D e s c r i p t i o n         ", "",
		 NA, NO,  JUSTLEFT, "", "", rgrs_rec.desc},
	{2, TAB, "res_type",	 0, 0, CHARTYPE,
		"A", "          ",
		"", "", "", "",
		 ND, NO,  JUSTLEFT, "", "", rgrs_rec.type},
	{2, TAB, "res_type_name", 0, 0, CHARTYPE,
		"AAAAAAAAAA", "          ",
		"", "", "  T y p e ", "",
		 NA, NO,  JUSTLEFT, "", "", rgrs_rec.type_name},
	{2, TAB, "rate",	 0, 0, MONEYTYPE,
		"NNNNN.NN", "          ",
		"", "", "", "",
		 ND, NO, JUSTRIGHT, "", "", (char *) &rgln_rec.rate},
	{2, TAB, "overHeadVariable",	 0, 0, MONEYTYPE,
		"NNNNN.NN", "          ",
		"", "", "", "",
		 ND, NO, JUSTRIGHT, "", "", (char *) &rgln_rec.ovhd_var},
	{2, TAB, "overHeadFixed",	 0, 0, MONEYTYPE,
		"NNNNN.NN", "          ",
		"", "", "", "",
		 ND, NO, JUSTRIGHT, "", "", (char *) &rgln_rec.ovhd_fix},
	{2, TAB, "setup",	 0, 0, TIMETYPE,
		"NNNN:NN", "          ",
		" ", "0:00", " Setup ", "",
		YES, NO, JUSTRIGHT, "0", "599999", (char *) &rgln_rec.setup},
	{2, TAB, "run",		 0, 0, TIMETYPE,
		"NNNN:NN", "          ",
		" ", "0:00", "  Run  ", "",
		YES, NO, JUSTRIGHT, "0", "599999", (char *) &rgln_rec.run},
	{2, TAB, "clean",	 0, 0, TIMETYPE,
		"NNNN:NN", "          ",
		" ", "0:00", " Clean ", "",
		YES, NO, JUSTRIGHT, "0", "599999", (char *) &rgln_rec.clean},
	{2, TAB, "canSplit",	 0, 2, CHARTYPE,
		"U", "          ",
		" ", "    ", "Splt", "Splitting in scheduling. Y(es)-Any split is valid. N(o)-Never split. S(ame)-Splitting allowed on same resource.",
		 NO, NO, JUSTLEFT, "YNS", "", local_rec.canSplit},
	{2, TAB, "resorceQty",	 0, 1, INTTYPE,
		"NN", "          ",
		" ", "1", "Qty ", "Enter the required number of operators",
		YES, NO, JUSTRIGHT, "1", "99", (char *) &rgln_rec.qty_rsrc},
	{2, TAB, "instructionNo",	 0, 1, INTTYPE,
		"NN", "          ",
		" ", "0", "Ins.", "Enter the instruction number",
		YES, NO, JUSTRIGHT, "", "", (char *) &rgln_rec.instr_no},
	{2, TAB, "yieldCalc",	 0, 0, CHARTYPE,
		"UUUU", "          ",
		" ", " ", "Yld.", "Enter the yield calculation name",
		YES, NO, JUSTLEFT, "", "", rgln_rec.yld_clc},
	{2, TAB, "byProduct",	 0, 0, CHARTYPE,
		"AAA", "          ",
		" ", "    ", "B/P", "",
		 NA, NO, JUSTLEFT, "", "", local_rec.byProduct},

	{3, TAB, "byProductItem",	 MAXLINES, 0, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "1", "By-Product   ", " Enter By-Product ",
		YES, NO, JUSTLEFT, "", "", local_rec.byProductItem},
	{3, TAB, "byProductDesc",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "D e s c r i p t i o n          ", "",
		 NA, NO, JUSTLEFT, "", "", local_rec.byProductDesc},
	{3, TAB, "byProductQty",	 0, 0, FLOATTYPE,
		"NNNNNNN.NNNNNN", "          ",
		" ", "", "Quantity   ", "",
		YES, NO, JUSTRIGHT, "0", "9999999.999999", (char *)&local_rec.byProductQty},
	{3, TAB, "byProductHhbrHash",	 0, 0, LONGTYPE,
		"NNNNNNNNNN", "          ",
		" ", "", "", "",
		 ND, NO, JUSTRIGHT, "", "", (char *)&local_rec.byProductHhbrHash},

	{0, LIN, "",		 0, 0, INTTYPE,
		"A", "          ",
		"", "", "dummy", "",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

/*=====================================================================
| Local Function Prototypes.
=====================================================================*/
int 	ByProductFree 			(struct SEQ_PTR *);
int 	DeleteByProductLine 	(void);
int 	DeleteLine 				(void);
int 	heading 				(int);
int 	InitSequence 			(void);
int 	InsertByProductLine 	(void);
int 	InsertLine 				(void);
int 	LoadByProducts 			(void);
int 	LoadFromList 			(void);
int 	spec_valid 				(int);
int 	ValidInst 				(long, long, int);
int 	win_function 			(int, int, int, int);
struct SEQ_PTR *bp_alloc 		(void);
void    CalcValues 				(void);
void 	CloseDB 				(void);
void    FixHhwc 				(void);
void    FreeList 				(void);
void    LoadDetails 			(void);
void 	OpenDB 					(void);
void 	shutdown_prog 			(void);
void    SrchPcid 				(void);
void    SrchPcwc 				(char *);
void    SrchRghr 				(void);
void    SrchRgrs 				(char *);
void    SrchRgyl 				(char *);
void 	tab_other 				(int);
void    Update 					(void);
void 	UpdateYield 			(int);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int    argc,
 char*  argv [])
{
	char	*sptr;

	_win_func = TRUE;
	/*-------------------------------
	| This `disables` sub-edit mode |
	-------------------------------*/
	in_sub_edit = TRUE;

	sptr = get_env ("PC_TIME_RES");
	envVarPcTimeRes = (sptr) ? atol (sptr) : 5L;

	SETUP_SCR (vars);


	OpenDB ();
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	init_scr ();
	set_tty ();
	set_masks ();
/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (2, store, sizeof (struct storeRec));
#endif
	init_vars (1);
	init_vars (2);

	/*===================================
	| Beginning of input control loop . |
	===================================*/
	while (prog_exit == 0)
	{
		/*-----------------------
		| Reset control flags . |
		-----------------------*/
		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		init_ok		= TRUE;
		search_ok	= TRUE;
		init_vars (1);
		init_vars (2);
		init_vars (3);
		lcount [2]	= 0;
		lcount [3]	= 0;

		InitSequence ();

		/*-------------------------------
		| Enter screen 1 linear input . |
		-------------------------------*/
		clearOK = TRUE;
		heading (1);
		entry (1);
		if (prog_exit || restart)
		{
			FLD ("printAll") = NO;
			abc_unlock (rghr);
			continue;
		}

		clearOK = FALSE;
		heading (2);
		clearOK = TRUE;
		scn_display (2);

		no_edit (3);
		inEditAll = TRUE;
		edit_all ();
		inEditAll = FALSE;

		if (restart)
		{
			FLD ("printAll") = NO;
			abc_unlock (rghr);
			continue;
		}

		Update ();
	}	/* end of input control loop	*/
	shutdown_prog ();
    return (EXIT_SUCCESS);
}

/*========================
| Program exit sequence. |
========================*/
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

	abc_alias (pcwc2, pcwc);
	abc_alias (rgrs2, rgrs);
	abc_alias (inmr2, inmr);

	open_rec (inmr,  inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (inmr2, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (pcid,  pcid_list, PCID_NO_FIELDS, "pcid_id_no");
	open_rec (pcwc,  pcwc_list, PCWC_NO_FIELDS, "pcwc_hhwc_hash");
	open_rec (pcwc2, pcwc_list, PCWC_NO_FIELDS, "pcwc_id_no");
	open_rec (rgbp,  rgbp_list, RGBP_NO_FIELDS, "rgbp_id_no");
	open_rec (rghr,  rghr_list, RGHR_NO_FIELDS, "rghr_id_no");
	open_rec (rgln,  rgln_list, RGLN_NO_FIELDS, "rgln_id_no");
	open_rec (rgrs,  rgrs_list, RGRS_NO_FIELDS, "rgrs_hhrs_hash");
	open_rec (rgrs2, rgrs_list, RGRS_NO_FIELDS, "rgrs_id_no");
	open_rec (rgyl,  rgyl_list, RGYL_NO_FIELDS, "rgyl_id_no");
	open_rec (rgbp,  rgbp_list, RGBP_NO_FIELDS, "rgbp_id_no");
	open_rec (inei,  inei_list, INEI_NO_FIELDS, "inei_id_no");
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
	abc_fclose (pcid);
	abc_fclose (pcwc);
	abc_fclose (pcwc2);
	abc_fclose (rgbp);
	abc_fclose (rghr);
	abc_fclose (rgln);
	abc_fclose (rgrs);
	abc_fclose (rgrs2);
	abc_fclose (rgyl);
	abc_fclose (rgbp);
	abc_fclose (inei);
	SearchFindClose ();
	abc_dbclose (data);
}

/*---------------------------
| Initialise sequence array |
---------------------------*/
int
InitSequence (
 void)
{
	int	i;

	for (i = 0; i < MAXLINES; i++)
	{
		seq_dtl [i].byProductCheck = FALSE;
		sprintf (seq_dtl [i].yieldCalc, "%-4.4s", " ");

		if (seq_dtl [i].list_head != SEQ_NULL)
		{
			currentSequence = i;
			FreeList ();
		}
		seq_dtl [i].list_head = SEQ_NULL;
	}

	return (EXIT_SUCCESS);
}

int
heading (
 int scn)
{
	if (!restart)
	{
		if (clearOK || scn != 3)
		{
			swide ();
			clear ();
		}

		if (scn == 1 || scn == 2 || (scn == 3 && clearOK))
		{
			box (4, 1, 123, 3);
			if (scn != 1)
			{
				scn_set (1);
				scn_write (1);
				scn_display (1);
			}
		}

		if (scn == 2 || (scn == 3 && clearOK) || (scn == 1 && inEditAll))
		{
			tab_col = 4;
			tab_row = 8;
			if (scn != 2)
			{
				scn_set (2);
				scn_write (2);
				scn_display (2);
			}
		}

		if (scn == 3)
		{
			tab_col = 5;
			tab_row = 9;
		}
		else
		{
			strcpy (err_str,ML (mlStdMess038));
			print_at (22, 0,err_str,comm_rec.co_no,comm_rec.co_name);
			strcpy (err_str,ML (mlStdMess039));
			print_at (22,40,err_str,comm_rec.est_no,comm_rec.est_name);
		}
		rv_pr (ML (mlRgMess012), 55, 0, 1);

		scn_set (scn);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
        return (EXIT_SUCCESS);
	}
    return (EXIT_FAILURE);
}

int
spec_valid (
 int    field)
{
	if (LCHECK ("itemNumber"))
	{
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
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();

		local_rec.hhbrHash = inmr_rec.hhbr_hash;
		strcpy (local_rec.itemNumber, inmr_rec.item_no);
		sprintf (local_rec.itemDesc, "%-40.40s", inmr_rec.description);
		DSP_FLD ("itemNumber");
		DSP_FLD ("itemDesc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("alt_no"))
	{
		if (dflt_used)
		{
			strcpy (inei_rec.est_no, comm_rec.est_no);
			inei_rec.hhbr_hash = local_rec.hhbrHash;
			cc = find_rec (inei, &inei_rec, EQUAL, "r");
			if (cc || inei_rec.dflt_rtg <= 0)
			{
				inmr_rec.hhbr_hash	=	local_rec.hhbrHash;
				cc = find_rec (inmr2, &inmr_rec, EQUAL, "r");
				if (cc)
					file_err (cc, inmr2, "DBFIND");

				if (inmr_rec.dflt_rtg <= 0)
				{
					print_mess (ML (mlStdMess007));
					sleep (sleepTime);
					clear_mess ();
					return (EXIT_FAILURE);
				}
				local_rec.alternateNo = inmr_rec.dflt_rtg; 
				DSP_FLD ("alt_no");
			}
			else
			{
				local_rec.alternateNo = inei_rec.dflt_rtg;
				DSP_FLD ("alt_no");
			}
		}

		if (SRCH_KEY)
		{
			SrchRghr ();
			return (EXIT_SUCCESS);
		}
		newRoute = FALSE;
		strcpy (rghr_rec.co_no, comm_rec.co_no);
		strcpy (rghr_rec.br_no, comm_rec.est_no);
		rghr_rec.hhbr_hash 	= inmr_rec.hhbr_hash;
		rghr_rec.alt_no		= local_rec.alternateNo;
		cc = find_rec (rghr, &rghr_rec, EQUAL, "u");
		if (cc)
		{
			abc_unlock (rghr);
			newRoute = TRUE;
			rghr_rec.hhgr_hash = 0L;
			strcpy (local_rec.printAll, "A");
			strcpy (local_rec.printAllDesc, ML ("All "));
			strcpy (rghr_rec.print_all, "A");
		}
		else
		{
			if (rghr_rec.print_all [0] == 'N')
			{
				strcpy (local_rec.printAll, "N");
				strcpy (local_rec.printAllDesc, ML ("Next"));
			}
			else
			{
				strcpy (local_rec.printAll, "A");
				strcpy (local_rec.printAllDesc, ML ("All "));
			}
			entry_exit = TRUE;
		}

		LoadDetails ();
		CalcValues ();

		DSP_FLD ("totalTime");
		DSP_FLD ("totalCost");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("printAll"))
	{
		FLD ("printAll") = NA;
		if (dflt_used)
		{
			strcpy (local_rec.printAll, "A");
			strcpy (local_rec.printAllDesc, ML ("All "));
			DSP_FLD ("printAllDesc");
			return (EXIT_SUCCESS);
		}

		if (local_rec.printAll [0] == 'N')
			strcpy (local_rec.printAllDesc, ML ("Next"));
		else
			strcpy (local_rec.printAllDesc, ML ("All "));
		DSP_FLD ("printAllDesc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("seq_no"))
	{
		if (last_char == INSLINE)
			return (InsertLine ());

		if (last_char == DELLINE || dflt_used)
			return (DeleteLine ());

		if (line_cnt > 0)
		{
			if (rgln_rec.seq_no < store [line_cnt - 1].seq_no)
			{
				errmess (ML (mlRgMess016));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
			if
			 (
				prog_status == EDIT &&
				rgln_rec.seq_no == store [line_cnt - 1].seq_no &&
				rgln_rec.hhwc_hash != store [line_cnt - 1].hhwcHash
			)
			{
				errmess (ML (mlRgMess010));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
		}
		if (line_cnt + 1 < lcount [2])
		{
			if (rgln_rec.seq_no > store [line_cnt + 1].seq_no)
			{
				errmess (ML (mlRgMess016));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
			if
			 (
				prog_status == EDIT &&
				rgln_rec.seq_no == store [line_cnt + 1].seq_no &&
				rgln_rec.hhwc_hash != store [line_cnt + 1].hhwcHash
			)
			{
				errmess (ML (mlRgMess010));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
		}
		store [line_cnt].seq_no = rgln_rec.seq_no;
		UpdateYield (FALSE);

		/* reset variables if this is a new sequence number */
		if (line_cnt + 1 > lcount [2])
		{
			sprintf (local_rec.workCentre,	"%-8.8s", " ");
			strcpy (local_rec.canSplit,	" ");
			sprintf (local_rec.byProduct,	"%-3.3s", " ");
			memset (&rgln_rec, 0, sizeof (rgln_rec));
			memset (&rgrs_rec, 0, sizeof (rgrs_rec));
			rgln_rec.seq_no = store [line_cnt].seq_no;
		}
	}

	if (LCHECK ("wrk_cntr"))
	{
		if (SRCH_KEY)
		{
			SrchPcwc (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (pcwc_rec.co_no, comm_rec.co_no);
		strcpy (pcwc_rec.br_no, comm_rec.est_no);
		strcpy (pcwc_rec.work_cntr, local_rec.workCentre);
		cc = find_rec (pcwc2, &pcwc_rec, EQUAL, "r");
		if (cc)
		{
			errmess (ML (mlRgMess009));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		if (line_cnt > 0)
		{
		    if ( rgln_rec.seq_no == store [line_cnt - 1].seq_no &&
				 pcwc_rec.hhwc_hash != store [line_cnt - 1].hhwcHash)
		    {
				errmess (ML (mlRgMess010));
				sleep (sleepTime);
				return (EXIT_FAILURE);
		    }
		}
		rgln_rec.hhwc_hash = pcwc_rec.hhwc_hash;
		store [line_cnt].hhwcHash = pcwc_rec.hhwc_hash;
		if (line_cnt + 1 < lcount [2])
		{
		    if ( rgln_rec.seq_no == store [line_cnt + 1].seq_no &&
				 rgln_rec.hhwc_hash != store [line_cnt + 1].hhwcHash)
				FixHhwc ();

		    if (!ValidInst ( rghr_rec.hhbr_hash, 
							 rgln_rec.hhwc_hash, 
							 rgln_rec.instr_no))
		    {
				rgln_rec.instr_no = 0;
				DSP_FLD ("instructionNo");
		    }
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("res_code"))
	{
		if (SRCH_KEY)
		{
			SrchRgrs (temp_str);
			return (EXIT_SUCCESS);
		}
		strcpy (rgrs_rec.co_no, comm_rec.co_no);
		strcpy (rgrs_rec.br_no, comm_rec.est_no);
		cc = find_rec (rgrs2, &rgrs_rec, EQUAL, "r");
		if (cc)
		{
			errmess (ML (mlRgMess011));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		rgln_rec.hhrs_hash = rgrs_rec.hhrs_hash;
		rgln_rec.rate = rgrs_rec.rate;
		rgln_rec.ovhd_var = rgrs_rec.ovhd_var;
		rgln_rec.ovhd_fix = rgrs_rec.ovhd_fix;

		store [line_cnt].rate = rgrs_rec.rate;
		store [line_cnt].overHeadVariable = rgrs_rec.ovhd_var;
		store [line_cnt].overHeadFixed = rgrs_rec.ovhd_fix;
		strcpy (store [line_cnt].type, rgrs_rec.type);
		rgln_rec.qty_rsrc = 1;
		store [line_cnt].resorceQty = 1;

		DSP_FLD ("res_desc");
		DSP_FLD ("res_type_name");
		DSP_FLD ("resorceQty");
		centre_at (6, 132, ML (mlRgMess015),
			DOLLARS (store [line_cnt].rate),
			DOLLARS (store [line_cnt].overHeadVariable),
			DOLLARS (store [line_cnt].overHeadFixed));

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("setup"))
	{
		rgln_rec.setup = rnd_time (rgln_rec.setup, envVarPcTimeRes);
		store [line_cnt].setup = rgln_rec.setup;
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("run"))
	{
		rgln_rec.run = rnd_time (rgln_rec.run, envVarPcTimeRes);
		store [line_cnt].run = rgln_rec.run;
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("clean"))
	{
		rgln_rec.clean = rnd_time (rgln_rec.clean, envVarPcTimeRes);
		store [line_cnt].clean = rgln_rec.clean;
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("canSplit"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.canSplit, "Y");
			DSP_FLD ("canSplit");
			return (EXIT_SUCCESS);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("resorceQty"))
	{
		store [line_cnt].resorceQty = rgln_rec.qty_rsrc;
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("instructionNo"))
	{
		if (SRCH_KEY)
		{
			SrchPcid ();
			return (EXIT_SUCCESS);
		}
		if (rgln_rec.instr_no == 0)
			return (EXIT_SUCCESS);

		if
		 (
		    ValidInst
		    (
			rghr_rec.hhbr_hash,
			rgln_rec.hhwc_hash,
			rgln_rec.instr_no
		   )
		)
		{
			return (EXIT_SUCCESS);
		}
		errmess (ML (mlStdMess184));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}

	if (LCHECK ("yieldCalc"))
	{
		if (dflt_used)
			strcpy (rgln_rec.yld_clc, "    ");

		if (SRCH_KEY)
		{
			SrchRgyl (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (rgyl_rec.co_no, comm_rec.co_no);
		sprintf (rgyl_rec.yld_clc, "%-4.4s", rgln_rec.yld_clc);
		cc = find_rec (rgyl, &rgyl_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlRgMess013));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		UpdateYield (TRUE);

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("byProductItem"))
	{
		if (last_char == INSLINE)
			return (InsertByProductLine ());

		if (last_char == DELLINE || dflt_used)
			return (DeleteByProductLine ());

		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}

		cc = FindInmr (comm_rec.co_no, local_rec.byProductItem, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.byProductItem);
			cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			errmess (ML (mlStdMess001));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();

		strcpy (local_rec.byProductItem, inmr_rec.item_no);
		sprintf (local_rec.byProductDesc, "%-40.40s", inmr_rec.description);
		local_rec.byProductHhbrHash = inmr_rec.hhbr_hash;

		DSP_FLD ("byProductItem");
		DSP_FLD ("byProductDesc");

		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

/*-----------------------------
| Update yield calc field for |
| all lines in this sequence  |
-----------------------------*/
void
UpdateYield (
 int    newYield)
{
	int	i, tmp_line, this_page;

	putval (line_cnt);
	this_page = line_cnt / TABLINES;
	currentSequence = rgln_rec.seq_no;

	if (newYield)
		sprintf (seq_dtl [currentSequence].yieldCalc,"%-4.4s",rgln_rec.yld_clc);

	tmp_line = line_cnt;
	for (i = 0; (i < lcount [2] && store [i].seq_no <= currentSequence); i++)
	{
		if (store [i].seq_no != currentSequence)
			continue;

		getval (i);
		sprintf (rgln_rec.yld_clc,"%-4.4s",seq_dtl [currentSequence].yieldCalc);
		putval (i);

		line_cnt = i;
		if (this_page == (line_cnt / TABLINES))
			DSP_FLD ("yieldCalc");
	}
	line_cnt = tmp_line;
	getval (line_cnt);
}

/*----------------------------------------------
| Insert a line into the by-product TAB screen |
----------------------------------------------*/
int
InsertByProductLine (
 void)
{
	int	i,
		this_page = line_cnt / TABLINES;

	for (i = line_cnt, line_cnt = lcount [3]; line_cnt > i; line_cnt--)
	{
		getval (line_cnt - 1);
		putval (line_cnt);
		if (this_page == line_cnt / TABLINES)
			line_display ();
	}
	lcount [3]++;
	line_cnt = i;

	sprintf (local_rec.byProductItem, "%-16.16s", " ");
	sprintf (local_rec.byProductDesc, "%-40.40s", " ");
	local_rec.byProductQty = 0.00;
	local_rec.byProductHhbrHash = 0L;
	putval (line_cnt);

	if (this_page == line_cnt / TABLINES)
		line_display ();
	init_ok = FALSE;
	prog_status = ENTRY;
	scn_entry (cur_screen);
	prog_status = !ENTRY;
	init_ok = TRUE;
	getval (line_cnt);

	return (EXIT_SUCCESS);
}

/*----------------------------------------------
| Delete a line from the by-product TAB screen |
----------------------------------------------*/
int
DeleteByProductLine (
 void)
{
	int	i,
		this_page = line_cnt / TABLINES;

	if (prog_status == ENTRY)
	{
		print_mess (ML (mlStdMess005));
		return (EXIT_FAILURE);
	}

	lcount [3]--;

	for (i = line_cnt; line_cnt < lcount [3]; line_cnt++)
	{
		getval (line_cnt + 1);
		putval (line_cnt);
		if (this_page == line_cnt / TABLINES)
			line_display ();
	}
	if (this_page == line_cnt / TABLINES)
		blank_display ();
	line_cnt = i;
	getval (line_cnt);

	return (EXIT_SUCCESS);
}

int
win_function (
 int    fld,
 int    lin,
 int    scn,
 int mode)
{
	struct	SEQ_PTR	*lcl_ptr, *curr_ptr, *tmp_ptr;
	int	i, tmp_line, add_to_item;

	if (scn != 2)
	{
		print_mess (ML (mlStdMess224));
		sleep (sleepTime);
		clear_mess ();
		return (PSLW);
	}

	if (mode == ENTRY)
	{
		print_mess (ML (mlStdMess225));
		sleep (sleepTime);
		clear_mess ();
		return (PSLW);
	}

	_win_func = FALSE;
	curr_ptr = SEQ_NULL;
	tmp_line = line_cnt;

	getval (lin);
	currentSequence = rgln_rec.seq_no;

	init_vars (3);
	lcount [3] = 0;

	if (CURR_HEAD != SEQ_NULL)
		LoadFromList ();

	clearOK = FALSE;
	heading (3);
	clearOK = TRUE;
	scn_display (3);
	edit (3);

	if (!restart)
	{
		FreeList ();
		/*-------------------
		| Build linked list |
		-------------------*/
		for (i = 0; i < lcount [3]; i++)
		{
			add_to_item = FALSE;
			getval (i);

			tmp_ptr = CURR_HEAD;
			while (tmp_ptr != SEQ_NULL)
			{
				if (local_rec.byProductHhbrHash == tmp_ptr->hhbrHash)
				{
					add_to_item = TRUE;
					break;
				}

				tmp_ptr = tmp_ptr->next;
			}

			if (add_to_item)
			{
				tmp_ptr->qty += local_rec.byProductQty;
				continue;
			}

			lcl_ptr = bp_alloc ();

			lcl_ptr->hhbrHash = local_rec.byProductHhbrHash;
			lcl_ptr->qty = local_rec.byProductQty;

			if (CURR_HEAD == SEQ_NULL)
			{
				CURR_HEAD = lcl_ptr;
				lcl_ptr->prev = SEQ_NULL;
			}
			else
			{
				curr_ptr->next = lcl_ptr;
				lcl_ptr->prev = curr_ptr;
			}

			curr_ptr = lcl_ptr;
		}
	}

	line_cnt = tmp_line;
	tab_col = 4;
	tab_row = 8;
	scn_set (2);

	box (4, 7, 123, 13);
	print_at (22, 0,ML (mlStdMess038),comm_rec.co_no,comm_rec.co_name);
	print_at (22,40,ML (mlStdMess039),comm_rec.est_no,comm_rec.est_name);

	/*------------------------------
	| Update byProduct flag on scn 2 |
	------------------------------*/
	for (i = 0; (i < lcount [2] && store [i].seq_no <= currentSequence); i++)
	{
		if (store [i].seq_no != currentSequence)
			continue;

		getval (i);
		strcpy (local_rec.byProduct, (CURR_HEAD == SEQ_NULL) ? "   " : "Yes");
		putval (i);
	}

	restart = FALSE;
	_win_func = TRUE;
	return (PSLW);
}

/*--------------------------------
| Load by-products from list for |
| current seq_no into TAB scr 3  |
--------------------------------*/
int
LoadFromList (
 void)
{
	struct	SEQ_PTR	*lcl_ptr;

	lcl_ptr = CURR_HEAD;
	while (lcl_ptr != SEQ_NULL)
	{
		cc = find_hash (inmr2, &inmr_rec, COMPARISON, "r", lcl_ptr->hhbrHash);
		if (!cc)
		{
			sprintf (local_rec.byProductItem, "%-16.16s", inmr_rec.item_no);
			sprintf (local_rec.byProductDesc, "%-40.40s", inmr_rec.description);

			local_rec.byProductQty = lcl_ptr->qty;
			local_rec.byProductHhbrHash = lcl_ptr->hhbrHash;
			putval (lcount [3]++);
		}

		lcl_ptr = lcl_ptr->next;
	}
	return (EXIT_SUCCESS);
}

struct SEQ_PTR *
bp_alloc (
 void)
{
	struct SEQ_PTR *lcl_ptr;

	/*-----------------------------
	| Return pointer to a node in |
	| the free list if possible   |
	-----------------------------*/
	if (free_head != SEQ_NULL)
	{
		lcl_ptr = free_head;
		free_head = free_head->next;

		lcl_ptr->next = SEQ_NULL;
		return (lcl_ptr);
	}

	lcl_ptr = (struct SEQ_PTR *) malloc (sizeof (struct SEQ_PTR));

	if (lcl_ptr == SEQ_NULL)
		sys_err ("Error allocating memory for sequence list During (MALLOC)", errno, PNAME);

	lcl_ptr->next = SEQ_NULL;

	return (lcl_ptr);
}

/*-----------------------------
| 'Free' list of by-products. |
-----------------------------*/
void
FreeList (
 void)
{
	struct SEQ_PTR *lcl_ptr, *tmp_ptr;

	lcl_ptr = CURR_HEAD;
	while (lcl_ptr != SEQ_NULL)
	{
		tmp_ptr = lcl_ptr;
		lcl_ptr = lcl_ptr->next;
		ByProductFree (tmp_ptr);
	}
	CURR_HEAD = SEQ_NULL;
}

/*----------------------------------------
| 'Free' node by storing in a free list. |
----------------------------------------*/
int
ByProductFree (
 struct SEQ_PTR*   bp_node)
{
	bp_node->hhbrHash = 0L;
	bp_node->qty       = 0.00;

	/*--------------------------------------
	| Remove node from current linked list |
	--------------------------------------*/
	if (bp_node->prev == SEQ_NULL)
		CURR_HEAD = bp_node->next;
	else
		bp_node->prev->next = bp_node->next;

	if (bp_node->next != SEQ_NULL)
		bp_node->next->prev = bp_node->prev;

	bp_node->next = SEQ_NULL;
	bp_node->prev = SEQ_NULL;

	/*----------------------------------
	| Insert node at head of free list |
	----------------------------------*/
	if (free_head != SEQ_NULL)
		bp_node->next = free_head;

	free_head = bp_node;

	return (EXIT_SUCCESS);
}

void
FixHhwc (
 void)
{
	char	workCentre [9];
	int	save_seq_no = rgln_rec.seq_no,
		this_page = line_cnt / TABLINES,
		i = line_cnt;

	strcpy (workCentre, local_rec.workCentre);
	putval (i);
	while
	 (
	    line_cnt + 1 < lcount [2] &&
	    store [line_cnt + 1].seq_no == save_seq_no
	)
	{
		line_cnt++;
		getval (line_cnt);
		strcpy (local_rec.workCentre, workCentre);
		store [line_cnt].hhwcHash = store [i].hhwcHash;
		rgln_rec.hhwc_hash = store [i].hhwcHash;
		if
		 (
		    !ValidInst
		    (
			rghr_rec.hhbr_hash,
			rgln_rec.hhwc_hash,
			rgln_rec.instr_no
		   )
		)
		{
			rgln_rec.instr_no = 0;
		}
		putval (line_cnt);
		if (this_page == line_cnt / TABLINES)
			line_display ();
	}
	getval (i);
	line_cnt = i;
}

int
ValidInst (
	long 	hhbrHash,
	long 	hhwcHash,
	int		instructionNo)
{
	strcpy (pcid_rec.co_no, comm_rec.co_no);
	pcid_rec.hhbr_hash 	= hhbrHash;
	pcid_rec.hhwc_hash 	= hhwcHash;
	pcid_rec.instr_no 	= instructionNo;
	pcid_rec.version 	= 1;
	pcid_rec.line_no 	= 0;

	cc = find_rec (pcid, &pcid_rec, GTEQ, "r");
	if
	 (
		!cc &&
		!strcmp (pcid_rec.co_no, comm_rec.co_no) &&
		pcid_rec.hhbr_hash == hhbrHash &&
		pcid_rec.hhwc_hash == hhwcHash &&
		pcid_rec.instr_no == instructionNo
	)
	{
		return (TRUE);
	}

	return (FALSE);
}

int
InsertLine (
 void)
{
	int	i,
		this_page = line_cnt / TABLINES;

	for (i = line_cnt, line_cnt = lcount [2]; line_cnt > i; line_cnt--)
	{
		getval (line_cnt - 1);
		store [line_cnt].seq_no				= rgln_rec.seq_no;
		store [line_cnt].hhwcHash			= rgln_rec.hhwc_hash;
		store [line_cnt].rate				= rgln_rec.rate;
		store [line_cnt].overHeadVariable	= rgln_rec.ovhd_var;
		store [line_cnt].overHeadFixed		= rgln_rec.ovhd_fix;
		store [line_cnt].setup				= rgln_rec.setup;
		store [line_cnt].run				= rgln_rec.run;
		store [line_cnt].clean				= rgln_rec.clean;
		strcpy (store [line_cnt].canSplit, 	local_rec.canSplit);
		store [line_cnt].resorceQty			= rgln_rec.qty_rsrc;
		strcpy (store [line_cnt].type,	  	rgrs_rec.type);
		store [line_cnt].instructionNo		= rgln_rec.instr_no;
		strcpy (store [line_cnt].yieldCalc,  rgln_rec.yld_clc);
		putval (line_cnt);
		if (this_page == line_cnt / TABLINES)
			line_display ();
	}
	lcount [2]++;
	line_cnt = i;

	rgln_rec.seq_no = 0;
	sprintf (pcwc_rec.work_cntr, "%-8.8s", " ");
	rgln_rec.hhwc_hash = 0L;
	sprintf (rgrs_rec.code, "%-8.8s", " ");
	sprintf (rgrs_rec.desc, "%-40.40s", " ");
	sprintf (rgrs_rec.type, "%-1.1s", " ");
	sprintf (rgrs_rec.type_name, "%-10.10s", " ");
	rgln_rec.rate 		= 0.00;
	rgln_rec.ovhd_var 	= 0.00;
	rgln_rec.ovhd_fix 	= 0.00;
	rgln_rec.setup 		= 0L;
	rgln_rec.run 		= 0L;
	rgln_rec.clean 		= 0L;
	strcpy (local_rec.canSplit, " ");
	rgln_rec.qty_rsrc 	= 0;
	rgln_rec.instr_no 	= 0;
	sprintf (rgln_rec.yld_clc, "%-4.4s", " ");
	putval (line_cnt);

	if (this_page == line_cnt / TABLINES)
		line_display ();
	init_ok = FALSE;
	prog_status = ENTRY;
	scn_entry (cur_screen);
	prog_status = !ENTRY;
	init_ok = TRUE;
	getval (line_cnt);

	return (EXIT_SUCCESS);
}

int
DeleteLine (
 void)
{
	int	i, seq_ok;
	int	this_page = line_cnt / TABLINES;

	if (prog_status == ENTRY)
	{
		print_mess (ML (mlStdMess005));
		return (EXIT_FAILURE);
	}

	lcount [2]--;

	currentSequence = store [line_cnt].seq_no;
	seq_ok = FALSE;

	for (i = line_cnt; line_cnt < lcount [2]; line_cnt++)
	{
		getval (line_cnt + 1);
		store [line_cnt].seq_no				= rgln_rec.seq_no;
		store [line_cnt].hhwcHash			= rgln_rec.hhwc_hash;
		store [line_cnt].rate				= rgln_rec.rate;
		store [line_cnt].overHeadVariable	= rgln_rec.ovhd_var;
		store [line_cnt].overHeadFixed		= rgln_rec.ovhd_fix;
		store [line_cnt].setup				= rgln_rec.setup;
		store [line_cnt].run				= rgln_rec.run;
		store [line_cnt].clean				= rgln_rec.clean;
		strcpy (store [line_cnt].canSplit,local_rec.canSplit);
		store [line_cnt].resorceQty			= rgln_rec.qty_rsrc;
		strcpy (store [line_cnt].type,	rgrs_rec.type);
		store [line_cnt].instructionNo		= rgln_rec.instr_no;
		strcpy (store [line_cnt].yieldCalc,rgln_rec.yld_clc);
		putval (line_cnt);
		if (this_page == line_cnt / TABLINES)
			line_display ();
	}
	if (this_page == line_cnt / TABLINES)
		blank_display ();
	line_cnt = i;
	getval (line_cnt);

	for (i = 0; i < lcount [2]; i++)
	{
		if (store [i].seq_no == currentSequence)
		{
			seq_ok = TRUE;
			break;
		}
	}

	if (!seq_ok)
	{
		seq_dtl [currentSequence].byProductCheck = FALSE;
		sprintf (seq_dtl [currentSequence].yieldCalc, "%-4.4s", " ");
		FreeList ();
	}

	return (EXIT_SUCCESS);
}

void
tab_other (
 int    line_no)
{
	char	*ttoa (long int, char *);
	int	i;

	if (cur_screen == 2)
	{
		if (line_no < lcount [2])
			centre_at (6, 132, ML (mlRgMess015),
				DOLLARS (store [line_no].rate),
				DOLLARS (store [line_no].overHeadVariable),
				DOLLARS (store [line_no].overHeadFixed));
		else
		{
			move (0, 6);
			cl_line ();
		}
	}

	CalcValues ();

	print_at (4, 22, "%s", ttoa (local_rec.totalTime, "NNNNNN:NN"));
	print_at (4, 52, "%9.2f", DOLLARS (local_rec.totalCost));

	i = (line_no / TABLINES) + 1;
	print_at (4, 64, ML (mlStdMess110), i);
	i = ((lcount [2] - 1) / TABLINES) + 1;
	print_at (4, 74, "of %2d", i);
}

void
LoadDetails (
 void)
{
	int	i, last_seq;

	init_vars (2);

	lcount [2] = 0;
	last_seq = 0;

	rgln_rec.hhgr_hash = rghr_rec.hhgr_hash;
	rgln_rec.seq_no = 0;
	cc = find_rec (rgln, &rgln_rec, GTEQ, "r");
	while (!cc && rghr_rec.hhgr_hash == rgln_rec.hhgr_hash)
	{
		cc = find_hash (pcwc, &pcwc_rec, EQUAL, "r", rgln_rec.hhwc_hash);
		if (cc)
			strcpy (pcwc_rec.work_cntr, "NOT FND.");
		strcpy (local_rec.workCentre, pcwc_rec.work_cntr);
		cc = find_hash (rgrs, &rgrs_rec, EQUAL, "r", rgln_rec.hhrs_hash);
		if (cc)
			strcpy (rgrs_rec.code, "NOT FND.");
		store [lcount [2]].seq_no			= rgln_rec.seq_no;
		store [lcount [2]].hhwcHash			= rgln_rec.hhwc_hash;
		store [lcount [2]].rate				= rgln_rec.rate;
		store [lcount [2]].overHeadVariable	= rgln_rec.ovhd_var;
		store [lcount [2]].overHeadFixed	= rgln_rec.ovhd_fix;
		store [lcount [2]].setup			= rgln_rec.setup;
		store [lcount [2]].run				= rgln_rec.run;
		store [lcount [2]].clean			= rgln_rec.clean;
		if (rgln_rec.can_split [0] == 'Y')
			strcpy (local_rec.canSplit, "Y");
		else if (rgln_rec.can_split [0] == 'S')
			strcpy (local_rec.canSplit, "S");
		else
			strcpy (local_rec.canSplit, "N");
		strcpy (store [lcount [2]].canSplit, local_rec.canSplit);
		store [lcount [2]].resorceQty		= rgln_rec.qty_rsrc;
		strcpy (store [lcount [2]].type,	rgrs_rec.type);
		store [lcount [2]].instructionNo	= rgln_rec.instr_no;

		if (last_seq != rgln_rec.seq_no)
		{
			sprintf (store [lcount [2]].yieldCalc, "%-4.4s",
				rgln_rec.yld_clc);

			sprintf (seq_dtl [rgln_rec.seq_no].yieldCalc, "%-4.4s",
				rgln_rec.yld_clc);
		}
		else
		{
			sprintf (store [lcount [2]].yieldCalc, "%-4.4s",
				seq_dtl [rgln_rec.seq_no].yieldCalc);

			sprintf (rgln_rec.yld_clc, "%-4.4s",
				seq_dtl [rgln_rec.seq_no].yieldCalc);
		}

		putval (lcount [2]++);
		cc = find_rec (rgln, &rgln_rec, NEXT, "r");
	}

	/*----------------------------------------
	| Load any by-products into linked lists |
	----------------------------------------*/
	LoadByProducts ();

	/*------------------------------
	| Update byProduct flag on scn 2 |
	------------------------------*/
	for (i = 0; i < lcount [2]; i++)
	{
		currentSequence = store [i].seq_no;

		getval (i);
		strcpy (local_rec.byProduct, (CURR_HEAD == SEQ_NULL) ? "   " : "Yes");
		putval (i);
	}

	scn_set (1);
}

/*----------------------------
| Load by-products from rgbp |
----------------------------*/
int
LoadByProducts (
 void)
{
	struct	SEQ_PTR
			*lcl_ptr,
			*curr_ptr = (struct SEQ_PTR *) 0;
	int	i;

	for (i = 0; i < lcount [2]; i++)
	{
		currentSequence = store [i].seq_no;

		if (seq_dtl [currentSequence].byProductCheck == TRUE)
			continue;

		rgbp_rec.hhgr_hash = rghr_rec.hhgr_hash;
		rgbp_rec.seq_no = currentSequence;
		rgbp_rec.hhbr_hash = 0L;
		cc = find_rec (rgbp, &rgbp_rec, GTEQ, "r");
		while
		 (
			!cc &&
			rgbp_rec.hhgr_hash == rghr_rec.hhgr_hash &&
			rgbp_rec.seq_no == currentSequence
		)
		{
			lcl_ptr = bp_alloc ();

			lcl_ptr->hhbrHash = rgbp_rec.hhbr_hash;
			lcl_ptr->qty = rgbp_rec.qty;

			if (CURR_HEAD == SEQ_NULL)
			{
				CURR_HEAD = lcl_ptr;
				lcl_ptr->prev = SEQ_NULL;
			}
			else
			{
				curr_ptr->next = lcl_ptr;
				lcl_ptr->prev = curr_ptr;
			}

			curr_ptr = lcl_ptr;

			cc = find_rec (rgbp, &rgbp_rec, NEXT, "r");
		}
		seq_dtl [currentSequence].byProductCheck = TRUE;
	}

	return (EXIT_SUCCESS);
}

void
CalcValues (
 void)
{
	double	cur_cost;
	long	cur_time,
		tmp_time = 0L;
	int	i;

	local_rec.totalTime = 0L;
	local_rec.totalCost = 0.00;

	for (tmp_time = 0L, i = 0; i < lcount [2]; i++)
	{
		cur_time = store [i].setup + store [i].run + store [i].clean;
		if (cur_time > tmp_time)
			tmp_time = cur_time;
		if (i + 1 == lcount [2] || store [i].seq_no != store [i + 1].seq_no)
		{
			local_rec.totalTime += tmp_time;
			tmp_time = 0L;
		}

		cur_cost = (store [i].rate + store [i].overHeadVariable) * cur_time;
		cur_cost /= 60.0;
		cur_cost += store [i].overHeadFixed;
		cur_cost *= store [i].resorceQty;
		local_rec.totalCost += cur_cost;
	}
}

void
Update (
 void)
{
	struct	SEQ_PTR	*lcl_ptr;
	int	i, last_seq;

	/*-------------------------------
	| If we're trying to create a	|
	| new routing with no lines, we	|
	| might as well suppress the	|
	| creation of the header too!!	|
	-------------------------------*/
	if (newRoute && lcount [2] == 0)
	{
		abc_unlock (rghr);
        return;
	}

	if (local_rec.printAll [0] == 'N')
		strcpy (rghr_rec.print_all, "N");
	else
		strcpy (rghr_rec.print_all, "A");

	if (newRoute)
	{
		strcpy (rghr_rec.co_no, comm_rec.co_no);
		strcpy (rghr_rec.br_no, comm_rec.est_no);
		rghr_rec.hhbr_hash 	= inmr_rec.hhbr_hash;
		rghr_rec.alt_no		= local_rec.alternateNo;
		/*-------------------------------
		| Note, this is a bit lax...	|
		| A different user on another	|
		| tty may have 'add'ed it!		|
		-------------------------------*/
		cc = abc_add (rghr, &rghr_rec);
		if (cc)
			file_err (cc, rghr, "DBADD");

		/*-------------------------------
		| We need to re-read it to get	|
		| the value of rghr_hhgr_hash!!	|
		-------------------------------*/
		cc = find_rec (rghr, &rghr_rec, EQUAL, "u");
	}

	/*-----------------------------------------------
	| Delete all old routing lines for this routing |
	-----------------------------------------------*/
	rgln_rec.hhgr_hash 	= rghr_rec.hhgr_hash;
	rgln_rec.seq_no 	= 0;
	cc = find_rec (rgln, &rgln_rec, GTEQ, "u");
	while (!cc && rgln_rec.hhgr_hash == rghr_rec.hhgr_hash)
	{
		cc = abc_delete (rgln);
		if (cc)
			file_err (cc, rgln, "DBDELETE");
		rgln_rec.hhgr_hash  = rghr_rec.hhgr_hash;
		rgln_rec.seq_no = 0;
		cc = find_rec (rgln, &rgln_rec, GTEQ, "u");
	}

	/*----------------------------
	| Delete all old by-products |
	----------------------------*/
	rgbp_rec.hhgr_hash = rghr_rec.hhgr_hash;
	rgbp_rec.seq_no = 0;
	rgbp_rec.hhbr_hash = 0L;
	cc = find_rec (rgbp, &rgbp_rec, GTEQ, "u");
	while (!cc && rgbp_rec.hhgr_hash == rghr_rec.hhgr_hash)
	{
		cc = abc_delete (rgbp);
		if (cc)
			file_err (cc, rgbp, "DBDELETE");

		rgbp_rec.hhgr_hash = rghr_rec.hhgr_hash;
		rgbp_rec.seq_no = 0;
		rgbp_rec.hhbr_hash = 0L;
		cc = find_rec (rgbp, &rgbp_rec, GTEQ, "u");
	}

	scn_set (2);

	last_seq = 0;
	for (i = 0; i < lcount [2]; i++)
	{
		getval (i);
		/*------------------
		| Add routing line |
		------------------*/
		rgln_rec.hhgr_hash = rghr_rec.hhgr_hash;
		strcpy (rgln_rec.can_split, local_rec.canSplit);
		cc = abc_add (rgln, &rgln_rec);
		if (cc)
			file_err (cc, rgln, "DBADD");

		/*-------------------------------------
		| Add any by-products of this routing |
		-------------------------------------*/
		currentSequence = store [i].seq_no;
		if (CURR_HEAD != SEQ_NULL && last_seq != currentSequence)
		{
			lcl_ptr = CURR_HEAD;
			while (lcl_ptr != SEQ_NULL)
			{
				rgbp_rec.hhgr_hash = rghr_rec.hhgr_hash;
				rgbp_rec.seq_no    = currentSequence;
				rgbp_rec.hhbr_hash = lcl_ptr->hhbrHash;
				rgbp_rec.qty       = lcl_ptr->qty;
				cc = abc_add (rgbp, &rgbp_rec);
				if (cc)
					file_err (cc, rgbp, "DBADD");

				lcl_ptr = lcl_ptr->next;
			}
		}
		last_seq = currentSequence;
	}

	if (!newRoute)
	{
		if (lcount [2] == 0)
		{
			cc = abc_delete (rghr);
			if (cc)
				file_err (cc, rghr, "DBDELETE");
		}
		else
		{
			cc = abc_update (rghr, &rghr_rec);
			if (cc)
				file_err (cc, rghr, "DBDELETE");
		}
	}
	else
		abc_unlock (rghr);

}

/*=======================
| Search for Work-Cntr	|
=======================*/
void
SrchRgyl (
 char*  key_val)
{
	work_open ();
	save_rec ("#Code", "#Name");
	strcpy (rgyl_rec.co_no, comm_rec.co_no);
	sprintf (rgyl_rec.yld_clc, "%-4.4s", key_val);
	cc = find_rec (rgyl, &rgyl_rec, GTEQ, "r");
	while (!cc &&
		!strcmp (rgyl_rec.co_no, comm_rec.co_no) &&
		!strncmp (rgyl_rec.yld_clc, key_val, strlen (key_val)))
	{
		cc = save_rec (rgyl_rec.yld_clc, rgyl_rec.yld_name);
		if (cc)
			break;
		cc = find_rec (rgyl, &rgyl_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (rgyl_rec.co_no, comm_rec.co_no);
	sprintf (rgyl_rec.yld_clc, "%-4.4s", key_val);
	cc = find_rec (rgyl, &rgyl_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, rgyl, "DBFIND");
}

/*=======================
| Search for Work-Cntr	|
=======================*/
void
SrchPcwc (
 char*      key_val)
{
	work_open ();
	save_rec ("#Code", "#Name");
	strcpy (pcwc_rec.co_no, comm_rec.co_no);
	strcpy (pcwc_rec.br_no, comm_rec.est_no);
	sprintf (pcwc_rec.work_cntr, "%-8.8s", key_val);
	cc = find_rec (pcwc2, &pcwc_rec, GTEQ, "r");
	while
	 (
		!cc &&
		!strcmp (pcwc_rec.co_no, comm_rec.co_no) &&
		!strcmp (pcwc_rec.br_no, comm_rec.est_no) &&
		!strncmp (pcwc_rec.work_cntr, key_val, strlen (key_val))
	)
	{
		cc = save_rec (pcwc_rec.work_cntr, pcwc_rec.name);
		if (cc)
			break;
		cc = find_rec (pcwc2, &pcwc_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (pcwc_rec.co_no, comm_rec.co_no);
	strcpy (pcwc_rec.br_no, comm_rec.est_no);
	sprintf (pcwc_rec.work_cntr, "%-8.8s", temp_str);
	cc = find_rec (pcwc2, &pcwc_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, pcwc2, "DBFIND");
}

/*=======================
| Search for Resource	|
=======================*/
void
SrchRgrs (
 char*  key_val)
{
	work_open ();
	save_rec ("#Code", "#Description");
	strcpy (rgrs_rec.co_no, comm_rec.co_no);
	strcpy (rgrs_rec.br_no, comm_rec.est_no);
	sprintf (rgrs_rec.code, "%-8.8s", key_val);
	cc = find_rec (rgrs2, &rgrs_rec, GTEQ, "r");
	while
	 (
		!cc &&
		!strcmp (rgrs_rec.co_no, comm_rec.co_no) &&
		!strcmp (rgrs_rec.br_no, comm_rec.est_no) &&
		!strncmp (rgrs_rec.code, key_val, strlen (key_val))
	)
	{
		cc = save_rec (rgrs_rec.code, rgrs_rec.desc);
		if (cc)
			break;
		cc = find_rec (rgrs2, &rgrs_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (rgrs_rec.co_no, comm_rec.co_no);
	strcpy (rgrs_rec.br_no, comm_rec.est_no);
	sprintf (rgrs_rec.code, "%-8.8s", temp_str);
	cc = find_rec (rgrs2, &rgrs_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, rgrs2, "DBFIND");
}

/*=======================
| Search for Alternates	|
=======================*/
void
SrchRghr (
 void)
{
	char	*ttoa (long int, char *),
		alt_str [6];

	work_open ();
	save_rec ("#Alt.", "#     Time     Cost");

	strcpy (rghr_rec.co_no, comm_rec.co_no);
	strcpy (rghr_rec.br_no, comm_rec.est_no);
	rghr_rec.hhbr_hash = inmr_rec.hhbr_hash;
	rghr_rec.alt_no = 1;
	cc = find_rec (rghr, &rghr_rec, GTEQ, "r");
	while
	 (
		!cc &&
		!strcmp (rghr_rec.co_no, comm_rec.co_no) &&
		!strcmp (rghr_rec.br_no, comm_rec.est_no) &&
		rghr_rec.hhbr_hash == inmr_rec.hhbr_hash
	)
	{
		LoadDetails ();
		CalcValues ();

		sprintf (alt_str, "%5d", rghr_rec.alt_no);

		sprintf (err_str, "%s%9.2f",
			ttoa (local_rec.totalTime, "NNNNNN:NN"),
			DOLLARS (local_rec.totalCost));
		cc = save_rec (alt_str, err_str);
		if (cc)
			break;
		cc = find_rec (rghr, &rghr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (rghr_rec.co_no, comm_rec.co_no);
	strcpy (rghr_rec.br_no, comm_rec.est_no);
	rghr_rec.hhbr_hash = inmr_rec.hhbr_hash;
	rghr_rec.alt_no = atoi (temp_str);
	cc = find_rec (rghr, &rghr_rec, GTEQ, "r");
}

/*
 * Search for Instr_nos	
 */
void
SrchPcid (void)
{
	int		firstTime	=	TRUE;
	int		lastInstr	=	0;
	int		lastVersion	=	0;
	char	lastText	[61];

	_work_open (2,0,40);
	save_rec ("#No", "#Instruction Description");
	strcpy (pcid_rec.co_no, comm_rec.co_no);
	pcid_rec.hhbr_hash = inmr_rec.hhbr_hash;
	pcid_rec.hhwc_hash = pcwc_rec.hhwc_hash;
	pcid_rec.instr_no = 1;
	pcid_rec.version = 1;
	pcid_rec.line_no = 0;
	cc = find_rec (pcid, &pcid_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (pcid_rec.co_no, comm_rec.co_no) &&
	       pcid_rec.hhbr_hash == inmr_rec.hhbr_hash &&
	       pcid_rec.hhwc_hash == pcwc_rec.hhwc_hash)
	{
		if (firstTime)
		{
			lastInstr = pcid_rec.instr_no;
			strcpy (lastText, pcid_rec.text);
			lastVersion	=	0;
			firstTime = FALSE;
		}
		if (pcid_rec.instr_no != lastInstr)
		{
			sprintf (err_str, "%2d", lastInstr);
			cc = save_rec (err_str, lastText);
			if (cc)
				break;
		}
		if (pcid_rec.version > lastVersion)
		{
			strcpy (lastText, pcid_rec.text);
			lastVersion = pcid_rec.version;
		}
		lastInstr = pcid_rec.instr_no;

		cc = find_rec (pcid, &pcid_rec, NEXT, "r");
	}
	if (firstTime == FALSE)
	{
		if (pcid_rec.version > lastVersion)
		{
			strcpy (lastText, pcid_rec.text);
			lastVersion = pcid_rec.version;
		}
		sprintf (err_str, "%2d", lastInstr);
		save_rec (err_str, lastText);
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	pcid_rec.instr_no = atoi (temp_str);
}

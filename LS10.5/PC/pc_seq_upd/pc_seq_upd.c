/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: pc_seq_upd.c,v 5.8 2002/07/24 08:38:59 scott Exp $
|  Program Name  : (pc_seq_upd.c  )                                   |
|  Program Desc  : (Production Control Sequence Update.         )     |
|                  (Receipts By-Products And Does Yield Calc.   )     |
|---------------------------------------------------------------------|
|  Date Written  : (18/02/92)      | Author       : Campbell Mander.  |
|---------------------------------------------------------------------|
| $Log: pc_seq_upd.c,v $
| Revision 5.8  2002/07/24 08:38:59  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.7  2002/07/18 06:50:07  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.6  2002/07/08 06:39:49  scott
| S/C 004487. Item number display is not aligned and date display is truncated.
|
| Revision 5.5  2002/07/08 05:42:26  kaarlo
| LS 00850. Update to fix item number alignment problem.
|
| Revision 5.4  2002/07/03 04:20:13  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.3  2001/08/09 09:14:44  scott
| Updated to add FinishProgram () function
|
| Revision 5.2  2001/08/06 23:35:03  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:18:25  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: pc_seq_upd.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PC/pc_seq_upd/pc_seq_upd.c,v 5.8 2002/07/24 08:38:59 scott Exp $";

#define	TABLINES	4

#include	<pslscr.h>
#include	<ml_std_mess.h>
#include	<ml_pc_mess.h>
#include	<MoveRec.h>
#include	<proc_sobg.h>
#include	<GlUtils.h>
#include	<twodec.h>
#include	<Costing.h>

#include	"schema"

struct comrRecord	comr_rec;
struct ccmrRecord	ccmr_rec;
struct commRecord	comm_rec;
struct inccRecord	incc_rec;
struct inwuRecord	inwu_rec;
struct inmrRecord	inmr_rec;
struct inumRecord	inum_rec;
struct pcbpRecord	pcbp_rec;
struct pcglRecord	pcgl_rec;
struct pclnRecord	pcln_rec;
struct pcmsRecord	pcms_rec;
struct pcrqRecord	pcrq_rec;
struct pcwcRecord	pcwc_rec;
struct pcwoRecord	pcwo_rec;
struct rgrsRecord	rgrs_rec;
struct esmrRecord	esmr_rec;

	int		*inei_expiry	=	&ineiRec.expiry_prd1;

	char	*data	= "data",
			*inmr2	= "inmr2",
			*inum2  = "inum2",
			*pcwo2  = "pcwo2",
			*rgrs2	= "rgrs2";

typedef	struct	_gl_acc
{
	long	hhmrHash;
	char	accountNumber [MAXLEVEL + 1];
	double	_amount;
	struct	_gl_acc	*_next;
} GL_ACC;

#define	GL_NULL	 (GL_ACC *) NULL

GL_ACC	*list_head = GL_NULL;
GL_ACC	*free_head = GL_NULL;

#define	TO_SEQ		 ((CLOSE_JOB) ? 32767 : atoi (local_rec.to_seq))
#define	CLOSE_JOB	 (local_rec.to_seq [0] == 'C')
#define	LOT_CTRL	 (inmr_rec.lot_ctrl [0] == 'Y')

char *stringIssue 	= " Materials Need Issuing Before Sequence %2d Can Start. ";
char *stringUpdate  = " Therefore Update Will Stop At Sequence %2d. ";
char *stringpSlip   = " Do You Want A Picking Slip To Be Printed ? ";
char *stringLot     = " Print Lot Details ? ";
char *stringBin     = " Print Bin Details ? ";
char *stringReq     = " Only Print Required Amounts ? ";

FILE	*pout;

struct	{
	char	name [10];
	int		noDays;
} mnths [] = {
	{"January",   31},
	{"February",  28},
	{"March",     31},
	{"April",     30},
	{"May",       31},
	{"June",      30},
	{"July",      31},
	{"August",    31},
	{"September", 30},
	{"October",   31},
	{"November",  30},
	{"December",  31},
	{"", 0}
};

	int		periodMonth;

	int		bp_col,
			bp_row,
			res_col,
			res_row,
			curr_seq,
			upd_seq,
			last_seq,
			num_bps,
			num_res,
			curr_scn,
			clear_ok,
			batch_flag;

	long	hhwoHash,
			hhwcHash,
			time_res,
			hhmrWipMat,
			hhmrWipLbr,
			hhmrWipOvh,
			hhmrWipOth,
			hhmrRcvLbr,
			hhmrRcvOvh,
			hhmrRcvOth;

	char	yld_clc [5],
			prod_class [5],
			hhmrWipMatAcc [MAXLEVEL + 1],
			hhmrWipLbrAcc [MAXLEVEL + 1],
			hhmrWipOvhAcc [MAXLEVEL + 1],
			hhmrWipOthAcc [MAXLEVEL + 1],
			hhmrRcvLbrAcc [MAXLEVEL + 1],
			hhmrRcvOvhAcc [MAXLEVEL + 1],
			hhmrRcvOthAcc [MAXLEVEL + 1];

	char	localCurrency[4];

	int		PC_GEN;
	long	currHhcc;

#define	SR		store [line_cnt]

struct storeRec
{
	char	lotControl [2];
	long	hhwhHash;
	long	hhumHash;
	long	hhbrHash;
	long	hhccHash;
	char	uom [5];
	float	cnvFct;
} store [MAXLINES];

/*============================
| Local & Screen Structures. |
============================*/
struct
{
	char	dummy [11];

	char	systemDate [11];
	long	lsystemDate;

	char	order_no [8];
	char	batch_no [11];
	char	reqBrNo [3];
	char	reqBrName [16];
	char	reqWhNo [3];
	char	reqWhName [10];
	char	recBrNo [3];
	char	recBrName [16];
	char	recWhNo [3];
	char	recWhName [10];
	long	recHhccHash;
	char	to_seq [8];
	int		prv_seq;
	int		priority;
	long	cre_dte;
	char	item_no [17];
	int		bom_alt;
	char	std_uom [5];
	char	alt_uom [5];
	char	strength [6];
	int		rtg_alt;
	char	desc [41];
	char	desc2 [41];
	float	qty_rqd;
	long	rqd_dte;
	char	std_batch_str [15];
	float	std_batch;
	float	min_batch;
	float	max_batch;

	char	rsrc [9];
	char	rsrc_desc [41];
	long	est_date;
	long	est_time;
	long	est_setup;
	long	est_run;
	long	est_clean;
	long	act_date;
	long	act_time;
	long	act_setup;
	long	act_run;
	long	act_clean;
	long	st_setup;
	long	st_run;
	long	st_clean;
	int		qty_res;
	long	hhrs_hash;
	int		new_times;

	char	work_cntr [9];
	double	tot_cost;
	long	hhbrHash;
	long	tot_time;
	char	by_prod [4];
	char	bp_item [17];
	char	bp_desc [41];
	float	bp_qty;
	char	location [11];
	char	creat_loc [2];
	long	bp_hhbr;
	long	hhwhHash;
	long	hhum_hash;
	int		ser_item;

	int		printerNumber;

	char	LL [2];
} local_rec;

extern	int	TruePosition;

static	struct	var	vars [] =
{
	{1, LIN, "order_no",	 2, 2, CHARTYPE,
		"UUUUUUU", "          ",
		" ", "", "Order Number      ", " Enter Works Order Number ",
		 NO, NO,  JUSTLEFT, "", "", local_rec.order_no},
	{1, LIN, "batch_no",	 2, 35, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", "", "Batch Number      ", " Enter Works Batch Number ",
		 NO, NO,  JUSTLEFT, "", "", local_rec.batch_no},
	{1, LIN, "prv_seq",	 2, 70, INTTYPE,
		"NN", "          ",
		" ", "", "Previous Sequence ", "",
		NA, NO,  JUSTRIGHT, "", "", (char *)&local_rec.prv_seq},
	{1, LIN, "to_seq",	 2, 105, CHARTYPE,
		"UN", "          ",
		" ", " ","Move To Sequence  ", " Enter Sequence Number To Proceed On To. (C for closing) ",
		YES, NO,  JUSTLEFT, "C0123456789", "", local_rec.to_seq},
	{1, LIN, "priority",	 4, 2, INTTYPE,
		"N", "          ",
		" ", "5", "Priority          ", " ",
		 NA, NO,  JUSTLEFT, "", "", (char *) &local_rec.priority},
	{1, LIN, "cre_dte",	 4, 38, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "",  "Date Raised       ", " ",
		 NA, NO,  JUSTLEFT, "", "", (char *) &local_rec.cre_dte},
	{1, LIN, "item_no",	 5, 2, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "",  "Item Number       ", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.item_no},
	{1, LIN, "bom_alt",	 5, 38, INTTYPE,
		"NNNNN", "          ",
		" ", "1", "BOM Alt. No.      ", " ",
		 NA, NO,  JUSTRIGHT, "1", "32767", (char *)&local_rec.bom_alt},
	{1, LIN, "std_uom",	 5, 70, CHARTYPE,
		"AAAA", "          ",
		"", "",   "Standard          ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.std_uom},
	{1, LIN, "alt_uom",	 5, 105, CHARTYPE,
		"AAAA", "          ",
		"", "",   "Alternate         ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.alt_uom},
	{1, LIN, "strength",	 6, 2, CHARTYPE,
		"AAAAA", "          ",
		" ", "",  "Strength          ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.strength},
	{1, LIN, "rtg_alt",	 6, 38, INTTYPE,
		"NNNNN", "          ",
		" ", "1", "Routing Number    ", " ",
		 NA, NO,  JUSTRIGHT, "1", "32767", (char *)&local_rec.rtg_alt},
	{1, LIN, "desc",		 7, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description       ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.desc},
	{1, LIN, "desc2",	 8, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description       ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.desc2},
	{1, LIN, "qty_rqd",	10, 2, FLOATTYPE,
		"NNNNNNN.NNNNNN", "          ",
		" ", local_rec.std_batch_str, "Quantity Required ", " ",
		NA, NO, JUSTRIGHT, "", "", (char *) &local_rec.qty_rqd},
	{1, LIN, "rqd_dte",	11, 2, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", local_rec.systemDate, "Required Date     ", " ",
		 NA, NO, JUSTLEFT, "", "", (char *) &local_rec.rqd_dte},
	{1, LIN, "reqBrNo",	 10, 38, CHARTYPE,
		"AA", "          ",
		" ", " ", "Request Branch    ", "Requesting Branch.",
		NA, NO,  JUSTRIGHT, "", "", local_rec.reqBrNo},
	{1, LIN, "reqBrName",	 10, 60, CHARTYPE,
		"UUUUUUUUUUUUUUU", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.reqBrName},
	{1, LIN, "reqWhNo",	 10, 90, CHARTYPE,
		"AA", "          ",
		" ", " ", "Request Warehouse ", "Requesting Warehouse.",
		NA, NO,  JUSTRIGHT, "", "", local_rec.reqWhNo},
	{1, LIN, "reqWhName",	 10, 110, CHARTYPE,
		"UUUUUUUUU", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.reqWhName},
	{1, LIN, "recBrNo",	 11, 38, CHARTYPE,
		"AA", "          ",
		" ", " ", "Receipt Branch    ", "Receiving Branch.",
		NA, NO,  JUSTRIGHT, "", "", local_rec.recBrNo},
	{1, LIN, "recBrName",	 11, 60, CHARTYPE,
		"UUUUUUUUUUUUUUU", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.recBrName},
	{1, LIN, "recWhNo",	 11, 90, CHARTYPE,
		"AA", "          ",
		" ", " ", "Receipt Warehouse ", "Receiving Warehouse.",
		NA, NO,  JUSTRIGHT, "", "", local_rec.recWhNo},
	{1, LIN, "recWhName",	 11, 110, CHARTYPE,
		"UUUUUUUUU", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.recWhName},

	{2, TAB, "rsrc",	MAXLINES, 0, CHARTYPE,
		"UUUUUUUU", "          ",
		" ", " ", "RESOURCE", " ",
		 NA, NO, JUSTLEFT, "", "", local_rec.rsrc},
	{2, TAB, "rsrc_desc",	0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "  D E S C R I P T I O N   ", " ",
		 NA, NO, JUSTLEFT, "", "", local_rec.rsrc_desc},
	{2, TAB, "est_date",	0, 0, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "", "   Date   ", " ",
		 NA, NO, JUSTLEFT, "", "", (char *) &local_rec.est_date},
	{2, TAB, "est_time",	0, 1, TIMETYPE,
		"NN:NN", "          ",
		" ", "", " Time ", " ",
		 NA, NO, JUSTLEFT, "0", "1439", (char *) &local_rec.est_time},
	{2, TAB, "est_setup",	0, 0, TIMETYPE,
		"NNNNN:NN", "          ",
		" ", "", " Setup  ", " ",
		 NA, NO, JUSTRIGHT, "", "", (char *) &local_rec.est_setup},
	{2, TAB, "est_run",	0, 0, TIMETYPE,
		"NNNNN:NN", "          ",
		" ", "", "  Run   ", " ",
		 NA, NO, JUSTRIGHT, "", "", (char *) &local_rec.est_run},
	{2, TAB, "est_clean",	0, 0, TIMETYPE,
		"NNNNN:NN", "          ",
		" ", "", "Cleanup ", " ",
		 NA, NO, JUSTRIGHT, "", "", (char *) &local_rec.est_clean},
	{2, TAB, "act_date",	0, 0, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", local_rec.systemDate, "   Date   ", " ",
		 YES, NO, JUSTLEFT, "", "", (char *) &local_rec.act_date},
	{2, TAB, "act_time",	0, 1, TIMETYPE,
		"NN:NN", "          ",
		" ", "0:00", " Time ", " ",
		 YES, NO, JUSTLEFT, "0", "1439", (char *) &local_rec.act_time},
	/*--------------------------------------
	| Next 3 fields are the original times |
	--------------------------------------*/
	{2, TAB, "st_setup",	0, 0, TIMETYPE,
		"NNNNN:NN", "          ",
		" ", "", "", " ",
		 ND, NO, JUSTRIGHT, "", "", (char *) &local_rec.st_setup},
	{2, TAB, "st_run",	0, 0, TIMETYPE,
		"NNNNN:NN", "          ",
		" ", "", "", " ",
		 ND, NO, JUSTRIGHT, "", "", (char *) &local_rec.st_run},
	{2, TAB, "st_clean",	0, 0, TIMETYPE,
		"NNNNN:NN", "          ",
		" ", "", "", " ",
		 ND, NO, JUSTRIGHT, "", "", (char *) &local_rec.st_clean},

	{2, TAB, "act_setup",	0, 0, TIMETYPE,
		"NNNNN:NN", "          ",
		" ", "0:00", " Setup  ", " ",
		 YES, NO, JUSTRIGHT, "0.00", "6000000", (char *) &local_rec.act_setup},
	{2, TAB, "act_run",	0, 0, TIMETYPE,
		"NNNNN:NN", "          ",
		" ", "0:00", "  Run   ", " ",
		 YES, NO, JUSTRIGHT, "0.00", "6000000", (char *) &local_rec.act_run},
	{2, TAB, "act_clean",	0, 0, TIMETYPE,
		"NNNNN:NN", "          ",
		" ", "0:00", "Cleanup ", " ",
		 YES, NO, JUSTRIGHT, "0.00", "6000000", (char *) &local_rec.act_clean},
	{2, TAB, "qty_res",	0, 0, INTTYPE,
		"NN", "          ",
		" ", "1", "Res", " ",
		 YES, NO, JUSTRIGHT, "1", "99", (char *) &local_rec.qty_res},
	{2, TAB, "hhrs_hash",	0, 0, LONGTYPE,
		"NNNNNNNNNN", "          ",
		" ", "", "", " ",
		 ND, NO, JUSTRIGHT, "", "", (char *) &local_rec.hhrs_hash},

	{3, TAB, "bp_item",	 MAXLINES, 0, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", " ", "By-Product   ", " Enter By-Product ",
		NA, NO, JUSTLEFT, "", "", local_rec.bp_item},
	{3, TAB, "bp_desc",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "D e s c r i p t i o n          ", "",
		NA, NO, JUSTLEFT, "", "", local_rec.bp_desc},
	{3, TAB, "bp_qty",	 0, 0, FLOATTYPE,
		"NNNNNNN.NNNNNN", "          ",
		" ", "", "Quantity   ", "",
		YES, NO, JUSTRIGHT, "0", "9999999.999999", (char *) &local_rec.bp_qty},
	{3, TAB, "LL", 	 0, 0, CHARTYPE, 
		"U", "          ", 
		" ", "N", "L", "Lot / Location Selection. <return> ", 
		 YES, NO,  JUSTLEFT, "", "", local_rec.LL}, 
	{3, TAB, "creat_loc",	 0, 0, CHARTYPE,
		"U", "          ",
		" ", " ", "", "",
		ND, NO, JUSTLEFT, "", "", local_rec.creat_loc},
	{3, TAB, "bp_hhbr",	 0, 0, LONGTYPE,
		"NNNNNNNNNN", "          ",
		" ", "", "", "",
		ND, NO, JUSTRIGHT, "", "", (char *)&local_rec.bp_hhbr},
	{3, TAB, "hhwhHash",	 0, 0, LONGTYPE,
		"NNNNNNNNNN", "          ",
		" ", "", "", "",
		ND, NO, JUSTRIGHT, "", "", (char *)&local_rec.hhwhHash},
	{3, TAB, "hhum_hash",	 0, 0, LONGTYPE,
		"NNNNNNNNNN", "          ",
		" ", "", "", "",
		ND, NO, JUSTRIGHT, "", "", (char *)&local_rec.hhum_hash},
	{3, TAB, "ser_item",	 0, 0, INTTYPE,
		"N", "          ",
		" ", "", "", "",
		ND, NO, JUSTRIGHT, "", "", (char *)&local_rec.ser_item},

	{0, LIN, "",		 0, 0, INTTYPE,
		"A", "          ",
		"", "", "dummy", "",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

#include	<LocHeader.h>
#include	<MoveAdd.h>
#include	<SrchPcwo2.h>

/*=====================
| function prototypes |
=====================*/
GL_ACC 	*AllocateAccount 		 (void);
double	AmountRoute 			 (long, int, int, int, char *);
double 	AmountIssue 			 (long);
int 	AddIncc 				 (void);
int 	CheckYieldCalc 			 (void);
int 	DeleteByProductLine 	 (void);
int 	DeleteResLine 			 (void);
int 	DisplayDetails 			 (int);
int 	FindIncc 				 (void);
int 	FindNextSequence 		 (void);
int 	LoadByProducts 			 (void);
int 	LoadPcrwRec 			 (void);
int 	NeedIssue 				 (void);
int 	Update 					 (int);
int 	UpdateByProducts 		 (void);
int 	UpdateRouteRecord 		 (void);
int 	ValidateLocation 		 (void);
int 	heading 				 (int);
int 	spec_valid 				 (int);
void 	AddPcgl 				 (long, char *, char *, double, char *);
void 	CloseDB 				 (void);
void 	DeAllocAccount 			 (void);
void 	EraseBitsOfScreen 		 (void);
void 	GetGlAccount 			 (double, int);
void 	ReadGlDefault 			 (char *);
void 	OpenDB 					 (void);
void 	PrintMaterialPSlip 		 (void);
void 	PrintPickingSlip 		 (void);
void 	SrchRgrs 				 (char *);
void 	SrchSeq 				 (char *);
void 	shutdown_prog 			 (int);
void 	tab_other 				 (int);

/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int  argc, 
 char *argv [])
{
	int	    i;
	char	*sptr,
			*chk_env (char *);
			
	sptr = get_env ("PC_TIME_RES");
	time_res = (sptr) ? atol (sptr) : 5L;

	/*-------------------------------------------------------
	| Works order number is M (anually or S (ystem generated. |
	-------------------------------------------------------*/
	sptr = chk_env ("PC_GEN_NUM");
	if (sptr)
		PC_GEN = (*sptr == 'M' || *sptr == 'm') ? FALSE : TRUE;
	else
		PC_GEN = TRUE;

	if (argc < 2)
	{
		print_at (0,0,"Usage [%s] [LPNO]",argv[0]);
		return (EXIT_FAILURE);
	}

	local_rec.printerNumber = atoi (argv [1]);

	res_col 	= 0;
	res_row 	= 15;
	bp_col 		= 22;
	bp_row 		= 14;

	/*-------------------------------
	| This 'disables' sub-edit mode |
	-------------------------------*/
	in_sub_edit = TRUE;

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));
	local_rec.lsystemDate = TodaysDate ();

	TruePosition	=	TRUE;
	SETUP_SCR (vars);


	OpenDB ();
	FLD ("LL") = ND;
	if (SK_BATCH_CONT || MULT_LOC)
		FLD ("LL") = YES;


	stringIssue	 = p_strsave (ML (mlPcMess140));
	stringUpdate = p_strsave (ML (mlPcMess141));
	stringpSlip  = p_strsave (ML (mlPcMess142)); 
	stringLot    = p_strsave (ML (mlPcMess143)); 
	stringBin    = p_strsave (ML (mlPcMess144)); 
	stringReq    = p_strsave (ML (mlPcMess145));

	init_scr 	();
	set_tty 	();
	swide 		();

	/*--------------------------
	| Get curr inventory month |
	--------------------------*/
	DateToDMY (comm_rec.inv_date, NULL, &periodMonth, NULL);

	/*----------------------------
	| Setup required parameters. |
	----------------------------*/
	set_masks ();
/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (3, store, sizeof (struct storeRec));
#endif
	init_vars (1);

	/*===================================
	| Beginning of input control loop . |
	===================================*/
	while (!prog_exit)
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
		lcount [2]  = 0;
		lcount [3]  = 0;

		/*-------------------------------
		| Enter screen 1 linear input . |
		-------------------------------*/
		clear_ok = TRUE;
		heading (1);
		entry (1);
		if (prog_exit || restart)
		{
			abc_unlock (pcwo);
			continue;
		}

		heading (1);
		scn_display (1);

		curr_seq = pcwo_rec.rtg_seq;

		pcln_rec.hhwo_hash = hhwoHash;
		pcln_rec.seq_no    = pcwo_rec.rtg_seq;
		pcln_rec.line_no   = 0;

		i = find_rec (pcln, &pcln_rec, GTEQ, "r");
		if
		 (
			!i &&
			pcln_rec.hhwo_hash == hhwoHash &&
			curr_seq == 0
		)
		{
			/*--------------------------------------
			| Check if materials need to be issued |
			| before the next routing sequence     |
			--------------------------------------*/
			upd_seq = pcln_rec.seq_no;
			if (NeedIssue ())
			{
				PrintPickingSlip ();
				EraseBitsOfScreen ();
				i = 1;
			}
			else
			{
				Update (pcln_rec.seq_no);
				curr_seq = pcln_rec.seq_no;
			}
		}

		cc = i;
		while
		 (
			!cc &&
			pcln_rec.hhwo_hash == hhwoHash &&
			pcln_rec.seq_no < TO_SEQ
		)
		{
			upd_seq = FindNextSequence ();

			/*--------------------------------------
			| Check if materials need to be issued |
			| before the next routing sequence     |
			--------------------------------------*/
			if (NeedIssue ())
			{
				PrintPickingSlip ();
				EraseBitsOfScreen ();
				break;
			}

			/*-----------------------------
			| Do any yield calcs required |
			-----------------------------*/
			if (!CheckYieldCalc ())
			{
				/*--------------------
				| Some error message |
				--------------------*/
				/*rv_pr ("\007 Failed Yield Calculation ",50,15,1);*/
				rv_pr (ML (mlPcMess021),50,15,1);
				sleep (sleepTime);
				break;
			}

			/*---------------------
			| Update pcrq records |
			---------------------*/
			init_vars (2);
			LoadPcrwRec ();
			if (lcount [2] != 0)
			{
				restart = FALSE;

				clear_ok = FALSE;
				heading (2);
				clear_ok = TRUE;

				scn_display (2);
				edit (2);
				if (restart)
					break;

				EraseBitsOfScreen ();
			}

			/*---------------------
			| Update pcbp records |
			---------------------*/
			init_vars (3);
			LoadByProducts ();
			if (lcount [3] != 0)
			{
				do
				{
					restart = FALSE;

					clear_ok = FALSE;
					heading (3);
					clear_mess ();
					clear_ok = TRUE;

					scn_display (3);
					edit (3);
					if (restart)
						break;

				} while (!ValidateLocation ());

				if (restart)
					break;

				EraseBitsOfScreen ();
			}

			/*------------------------------
			| Update pcrq and pcbp records |
			------------------------------*/
			UpdateRouteRecord ();
			UpdateByProducts ();

			Update (upd_seq);

			curr_seq = upd_seq;

			pcln_rec.hhwo_hash = hhwoHash;
			pcln_rec.seq_no    = curr_seq;
			pcln_rec.line_no   = 0;
			cc = find_rec (pcln, &pcln_rec, GTEQ, "r");
		}

		/*--------------------------------------
		| Don't update if FN1 has been pressed |
		--------------------------------------*/
		if (restart)
		{
			abc_unlock (pcwo);
			continue;
		}

		abc_unlock (pcwo);

	}	/* end of input control loop	*/

	shutdown_prog (0);
	return (EXIT_SUCCESS);
}

/*========================
| Program exit sequence. |
========================*/
void
shutdown_prog (
 int error)
{
	if (!error)
		recalc_sobg ();

	CloseDB (); 
	FinishProgram ();
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (void)
{
	MoveOpen	=	TRUE;

	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	abc_alias (inmr2, inmr);
	abc_alias (inum2, inum);
	abc_alias (rgrs2, rgrs);
	abc_alias (pcwo2, pcwo);

	open_rec (glmr,  glmr_list, GLMR_NO_FIELDS, "glmr_hhmr_hash");
	open_rec (incc,  incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (inmr,  inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (inmr2, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (inum,  inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	open_rec (inum2, inum_list, INUM_NO_FIELDS, "inum_uom");
	open_rec (pcbp,  pcbp_list, PCBP_NO_FIELDS, "pcbp_id_no");
	open_rec (pcgl,  pcgl_list, PCGL_NO_FIELDS, "pcgl_id_no");
	open_rec (pcln,  pcln_list, PCLN_NO_FIELDS, "pcln_id_no");
	open_rec (pcms,  pcms_list, PCMS_NO_FIELDS, "pcms_id_no");
	open_rec (pcrq,  pcrq_list, PCRQ_NO_FIELDS, "pcrq_id_no2");
	open_rec (pcwc,  pcwc_list, PCWC_NO_FIELDS, "pcwc_hhwc_hash");
	open_rec (pcwo,  pcwo_list, PCWO_NO_FIELDS, "pcwo_id_no");
	open_rec (pcwo2, pcwo_list, PCWO_NO_FIELDS, "pcwo_id_no3");
	open_rec (rgrs,  rgrs_list, RGRS_NO_FIELDS, "rgrs_hhrs_hash");
	open_rec (rgrs2, rgrs_list, RGRS_NO_FIELDS, "rgrs_id_no");
	open_rec (ccmr,  ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	open_rec (esmr,  esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec ("move",move_list, MOVE_NO_FIELDS, "move_move_hash");

	/*-----------------------------------------
	| Find ccmr record for current warehouse. |
	-----------------------------------------*/
	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, comm_rec.est_no);
	strcpy (ccmr_rec.cc_no, comm_rec.cc_no);
	cc = find_rec (ccmr, &ccmr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");
	currHhcc = ccmr_rec.hhcc_hash;

	open_rec (comr,  comr_list, COMR_NO_FIELDS, "comr_co_no");

	strcpy (comr_rec.co_no, comm_rec.co_no);
	cc = find_rec (comr, &comr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, comr, "DBFIND");

	if (!strcmp (comr_rec.base_curr, "   "))
		sprintf (localCurrency, "%-3.3s", get_env ("CURR_CODE"));
	else
		sprintf (localCurrency, "%-3.3s", comr_rec.base_curr);

	abc_fclose (comr);

	OpenLocation (ccmr_rec.hhcc_hash);
	OpenGlmr ();
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (glmr);
	abc_fclose (incc);
	abc_fclose (inmr);
	abc_fclose (inmr2);
	abc_fclose (inum);
	abc_fclose (inum2);
	abc_fclose (pcbp);
	abc_fclose (pcgl);
	abc_fclose (pcln);
	abc_fclose (pcms);
	abc_fclose (pcrq);
	abc_fclose (pcwc);
	abc_fclose (pcwo);
	abc_fclose (pcwo2);
	abc_fclose (rgrs);
	abc_fclose (rgrs2);
	abc_fclose (ccmr);
	abc_fclose (esmr);
	abc_fclose ("move");
	CloseLocation ();
	CloseCosting ();
	GL_Close ();

	SearchFindClose ();
	abc_dbclose (data);
}

void
ReadGlDefault (
	char	*category)
{
	abc_selfield (glmr, "glmr_id_no");

	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"WIP D MATL",
		" ",
		category
	);
	hhmrWipMat = glmrRec.hhmr_hash;
	strcpy (hhmrWipMatAcc, glmrRec.acc_no);

	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"WIP D LABR",
		" ",
		category
	);
	hhmrWipLbr = glmrRec.hhmr_hash;
	strcpy (hhmrWipLbrAcc, glmrRec.acc_no);

	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"WIP D MACH",
		" ",
		category
	);
	hhmrWipOvh = glmrRec.hhmr_hash;
	strcpy (hhmrWipOvhAcc, glmrRec.acc_no);

	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"WIP D OTH ",
		" ",
		category
	);
	hhmrWipOth = glmrRec.hhmr_hash;
	strcpy (hhmrWipOthAcc, glmrRec.acc_no);

	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"REC D LABR",
		" ",
		category
	);
	hhmrRcvLbr = glmrRec.hhmr_hash;
	strcpy (hhmrRcvLbrAcc, glmrRec.acc_no);

	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"REC D MACH",
		" ",
		category
	);
	hhmrRcvOvh = glmrRec.hhmr_hash;
	strcpy (hhmrRcvOvhAcc, glmrRec.acc_no);

	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"REC D OTH ",
		" ",
		category
	);
	hhmrRcvOth = glmrRec.hhmr_hash;
	strcpy (hhmrRcvOthAcc, glmrRec.acc_no);

	abc_selfield (glmr, "glmr_hhmr_hash");
}

/*========================================
| Find next sequence number to update to |
========================================*/
int
FindNextSequence (
 void)
{
	pcln_rec.hhwo_hash = hhwoHash;
	pcln_rec.seq_no    = curr_seq;
	pcln_rec.line_no   = 0;
	cc = find_rec (pcln, &pcln_rec, GTEQ, "r");
	while (!cc &&
		pcln_rec.hhwo_hash == hhwoHash &&
		pcln_rec.seq_no    == curr_seq)
	{
		cc = find_rec (pcln, &pcln_rec, NEXT, "r");
	}

	if (!cc && pcln_rec.hhwo_hash == hhwoHash)
		return (pcln_rec.seq_no);
	else
		return (TO_SEQ);
}

/*=============================================================
| Check that each BP is being receipted into a valid location |
=============================================================*/
int
ValidateLocation (void)
{
	int		err_page, 
			err_line;

	long	hhloHash	=	0L;

	if (!MULT_LOC)
		return (TRUE);

	scn_set (3);
	for (line_cnt = 0; line_cnt < lcount [3]; line_cnt++)
	{
		getval (line_cnt);

		if (local_rec.bp_qty == 0.00)
			continue;

		if (local_rec.creat_loc [0] == 'Y')
			continue;

		if (!LL_Valid (line_cnt, 0))
			cc = 1;
		else
		{
			char	lineLocation [11];

			sprintf (lineLocation, "%-10.10s", GetLoc (line_cnt, 0));

			cc = FindLocation 
				 (
					SR.hhwhHash, 
					0L, 
					lineLocation,
					ValidLocations,
					&hhloHash
				);
		}
		if (cc)
		{
			err_page = line_cnt / TABLINES;
			err_line = line_cnt - (err_page * TABLINES) + 1;
			sprintf (err_str,ML (mlPcMess015),local_rec.bp_item,err_page + 1,
				err_line);
			print_mess (err_str);
			sleep (sleepTime);
			return (FALSE);
		}
	}
	return (TRUE);
}

int
heading (
 int scn)
{
	curr_scn = scn;

	if (!restart)
	{
		if (clear_ok)
		{
			swide ();
			clear ();
		}

		print_at (0,121,"%-10.10s", local_rec.systemDate);
		rv_pr (ML (mlPcMess016), 48, 0, 1);

		box (0, 1, 132, 10);
		box (67, 3, 65, 5);
		move (0, 3); PGCHAR (10); line (67); PGCHAR (8); line (64); PGCHAR (11);
		move (67, 6); PGCHAR (10); line (64); PGCHAR (11);
		move (0, 9); PGCHAR (10); line (67); PGCHAR (9); line (64); PGCHAR (11);

		rv_pr (ML (mlPcMess014), 88, 4, FALSE);
		rv_pr (ML (mlPcMess017), 89, 7, FALSE);

		print_at (8,  69, ML (mlPcMess018), ineiRec.std_batch);
		print_at (8,  90, ML (mlPcMess019), ineiRec.min_batch);
		print_at (8, 111, ML (mlPcMess020), ineiRec.max_batch);
		if (scn != 2)
		{
			move (0, 21);
			line (132);
		}

		if (scn == 2)
		{
			if (clear_ok)
			{
				scn_set (1);
				scn_write (1);
				scn_display (1);
			}

			move (0, 21);
			cl_line ();
			box (0, 13, 131, 7);
			sprintf (err_str,ML (mlPcMess022), curr_seq, local_rec.work_cntr);
			rv_pr (err_str, (132 - strlen (err_str)) / 2, 13, TRUE);
			rv_pr (ML (mlPcMess023), 53, 14, FALSE);
			rv_pr (ML (mlPcMess024), 102, 14, FALSE);
			rv_pr (ML (mlPcMess025), 127, 14, FALSE);
			move (36, 14); PGCHAR (4);
			move (81, 14); PGCHAR (4);
			move (126, 14); PGCHAR (4);

			tab_col = res_col;
			tab_row = res_row;
		}

		if (scn == 3)
		{
			if (clear_ok)
			{
				scn_set (1);
				scn_write (1);
				scn_display (1);
			}

			box (bp_col, 13, 85, 7);
			rv_pr (ML (mlPcMess026), (bp_col + 25), 13, TRUE);
			tab_col = bp_col;
			tab_row = bp_row;
		}
		strcpy (err_str,ML (mlStdMess038));
		print_at (22,0,err_str,comm_rec.co_no, comm_rec.co_short);
		strcpy (err_str,ML (mlStdMess039));
		print_at (22,40,err_str, comm_rec.est_no,comm_rec.est_short);
		strcpy (err_str,ML (mlStdMess099));
		print_at (22,80,err_str,comm_rec.cc_no, comm_rec.cc_short);


		scn_set (scn);
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
	return (EXIT_FAILURE);
}

int
spec_valid (
 int field)
{
	int		i,
			flag;
	int		TempLine;
	long	rnd_time (long int, long int);

	if (LCHECK ("order_no"))
	{
		if (dflt_used)
		{
			FLD ("batch_no") = YES;
			return (EXIT_SUCCESS);
		}
		else
			FLD ("batch_no") = NA;

		if (SRCH_KEY)
		{
			SearchOrder (temp_str, "R", comm_rec.est_no, comm_rec.cc_no);
			return (EXIT_SUCCESS);
		}
		strcpy (pcwo_rec.co_no, comm_rec.co_no);
		strcpy (pcwo_rec.br_no, comm_rec.est_no);
		strcpy (pcwo_rec.wh_no, comm_rec.cc_no);
		if (PC_GEN)
			strcpy (pcwo_rec.order_no, zero_pad (local_rec.order_no, 7));
		else
			strcpy (pcwo_rec.order_no, local_rec.order_no);
		cc = find_rec (pcwo, &pcwo_rec, EQUAL, "u");
		if (cc)
		{
			abc_unlock (pcwo);
			print_mess (ML (mlPcMess067));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		flag = DisplayDetails (FALSE);
		if (flag)
			sprintf (local_rec.order_no, "%-7.7s", " ");

		return (flag);
	}

	if (LCHECK ("batch_no"))
	{
		if (F_NOKEY (label ("batch_no")))
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			SearchBatch (temp_str, "R", comm_rec.est_no, comm_rec.cc_no);
			abc_selfield (pcwo, "pcwo_id_no");
			return (EXIT_SUCCESS);
		}
		strcpy (pcwo_rec.co_no, comm_rec.co_no);
		strcpy (pcwo_rec.br_no, comm_rec.est_no);
		strcpy (pcwo_rec.wh_no, comm_rec.cc_no);
		strcpy (pcwo_rec.batch_no, local_rec.batch_no);
		cc = find_rec (pcwo2, &pcwo_rec, EQUAL, "u");
		if (cc)
		{
			abc_unlock (pcwo);
			print_mess (ML (mlPcMess067));
			sleep (sleepTime);
			clear_mess ();
			sprintf (local_rec.batch_no, "%-10.10s", " ");
			return (EXIT_FAILURE);
		}

		flag = DisplayDetails (TRUE);
		if (flag)
			sprintf (local_rec.batch_no, "%-10.10s", " ");

		return (flag);
	}

	if (LCHECK ("to_seq"))
	{
		if (dflt_used)
		{
			/*--------------------------------
			| Increment to next valid seq no |
			--------------------------------*/
			pcln_rec.hhwo_hash = hhwoHash;
			pcln_rec.seq_no = local_rec.prv_seq + 1;
			pcln_rec.line_no = 0;
			cc = find_rec (pcln, &pcln_rec, GTEQ, "r");
			if (cc || pcln_rec.hhwo_hash != hhwoHash)
				return (EXIT_FAILURE);

			sprintf (local_rec.to_seq, "%2d    ",pcln_rec.seq_no);
			DSP_FLD ("to_seq");
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchSeq (temp_str);
			return (EXIT_SUCCESS);
		}

		if (CLOSE_JOB)
		{
			strcpy (local_rec.to_seq, "Closing");
			DSP_FLD ("to_seq");

			/*----------------------------------
			| Find last sequence no in routing |
			----------------------------------*/
			pcln_rec.hhwo_hash 	= pcwo_rec.hhwo_hash;
			pcln_rec.seq_no 	= pcwo_rec.rtg_seq;
			pcln_rec.line_no 	= 0;
			cc = find_rec (pcln, &pcln_rec, GTEQ, "r");
			while (!cc &&
				pcln_rec.hhwo_hash == hhwoHash)
			{
				last_seq = pcln_rec.seq_no;
				cc = find_rec (pcln, &pcln_rec, NEXT, "r");
			}

			return (EXIT_SUCCESS);
		}

		if (TO_SEQ <= local_rec.prv_seq)
		{
			print_mess (ML (mlPcMess128));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		pcln_rec.hhwo_hash = hhwoHash;
		pcln_rec.seq_no = TO_SEQ;
		pcln_rec.line_no = 0;
		cc = find_rec (pcln, &pcln_rec, GTEQ, "r");
		if
		 (
			cc ||
			pcln_rec.hhwo_hash != hhwoHash ||
			pcln_rec.seq_no != TO_SEQ
		)
		{
			print_mess (ML (mlPcMess129));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("rsrc"))
	{
		if (last_char == DELLINE || dflt_used)
			return (DeleteResLine ());

		if (SRCH_KEY)
		{
			SrchRgrs (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (rgrs_rec.co_no, comm_rec.co_no);
		strcpy (rgrs_rec.br_no, comm_rec.est_no);
		sprintf (rgrs_rec.code, "%-8.8s", local_rec.rsrc);
		cc = find_rec (rgrs2, &rgrs_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlPcMess104));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		sprintf (local_rec.rsrc_desc, "%-26.26s", rgrs_rec.desc);
		DSP_FLD ("rsrc_desc");

		local_rec.hhrs_hash = rgrs_rec.hhrs_hash;

		local_rec.est_date = 0L;
		local_rec.est_time = 0L;
		local_rec.est_setup = 0L;
		local_rec.est_run = 0L;
		local_rec.est_clean = 0L;
		local_rec.st_setup = 0L;
		local_rec.st_run = 0L;
		local_rec.st_clean = 0L;
		local_rec.new_times = TRUE;
		DSP_FLD ("est_date");
		DSP_FLD ("est_time");
		DSP_FLD ("est_setup");
		DSP_FLD ("est_run");
		DSP_FLD ("est_clean");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("act_date"))
	{
		if (local_rec.act_date < local_rec.cre_dte)
		{
			print_mess (ML (mlPcMess130));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("act_time"))
	{
		if (dflt_used)
			strcpy (temp_str, TimeHHMM ());
		
		local_rec.act_time = atot (temp_str);
		local_rec.act_time = rnd_time (local_rec.act_time, time_res);
		DSP_FLD ("act_time");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("act_setup"))
	{
		local_rec.act_setup = atot (temp_str);
		local_rec.act_setup = rnd_time (local_rec.act_setup, time_res);
		DSP_FLD ("act_setup");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("act_run"))
	{
		local_rec.act_run = atot (temp_str);
		local_rec.act_run = rnd_time (local_rec.act_run, time_res);
		DSP_FLD ("act_run");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("act_clean"))
	{
		local_rec.act_clean = atot (temp_str);
		local_rec.act_clean = rnd_time (local_rec.act_clean, time_res);
		DSP_FLD ("act_clean");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("bp_item"))
	{
		if (last_char == DELLINE || dflt_used)
			return (DeleteByProductLine ());

		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}
		cc = FindInmr (comm_rec.co_no, local_rec.bp_item, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.bp_item);
			cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			print_mess (ML (mlStdMess001));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();

		if (inmr_rec.serial_item [0] == 'Y')
		{
			print_mess (ML (mlPcMess131));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		local_rec.ser_item = FALSE;
		strcpy (local_rec.bp_item, inmr_rec.item_no);
		sprintf (local_rec.bp_desc, "%-40.40s", inmr_rec.description);
		local_rec.bp_hhbr = inmr_rec.hhbr_hash;

		if (FindIncc ())
		{
			i = prmptmsg (ML (mlStdMess033),"YyNn",1,2);
			
			if (i == 'n' || i == 'N')
			{
				skip_entry = -1 ;
				return (EXIT_SUCCESS);
			}
			else
			{
				cc = AddIncc ();
				if (cc)
					file_err (cc, incc, "DBADD");
			}
		}

		strcpy (SR.uom, inmr_rec.sale_unit);
		strcpy (SR.lotControl, inmr_rec.lot_ctrl);
		SR.hhbrHash = inmr_rec.hhbr_hash;

		/*-------------------
		| Find inum record. |
		-------------------*/
		strcpy (inum_rec.uom, inmr_rec.sale_unit);
		cc = find_rec (inum, &inum_rec, EQUAL, "r");
		if (cc)
			file_err (cc, inum, "DBFIND");

		SR.hhumHash   = inum_rec.hhum_hash;
		SR.cnvFct     = inum_rec.cnv_fct;

		local_rec.hhwhHash = incc_rec.hhwh_hash;
		local_rec.hhum_hash = inmr_rec.std_uom;

		SR.hhwhHash = incc_rec.hhwh_hash;
		SR.hhccHash = incc_rec.hhcc_hash;

		DSP_FLD ("bp_item");
		DSP_FLD ("bp_desc");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("bp_qty"))
	{
		if (local_rec.ser_item == TRUE)
		{
			local_rec.bp_qty = 0.00;
			print_mess (ML (mlPcMess131));
			sleep (sleepTime);
			clear_mess ();
			DSP_FLD ("bp_qty");
			return (EXIT_SUCCESS);
		}

		return (EXIT_SUCCESS);
	}

	/*------------------------------
	| Validate lots and locations. |
	------------------------------*/
	if (LCHECK ("LL"))
	{
		if (FLD ("LL") == ND)
			return (EXIT_SUCCESS);

		TempLine	=	lcount[3];
		cc = DisplayLL
			 (										/*----------------------*/
				line_cnt,							/*	Line number.		*/
				tab_row + 3 + (line_cnt % TABLINES),/*  Row for window		*/
				tab_col + 8,						/*  Col for window		*/
				4,									/*  length for window	*/
				SR.hhwhHash, 						/*	Warehouse hash.		*/
				SR.hhumHash,						/*	UOM hash			*/
				SR.hhccHash,						/*	CC hash.			*/
				SR.uom,							/* UOM					*/
				local_rec.bp_qty,					/* Quantity.			*/
				SR.cnvFct,						/* Conversion factor.	*/
				TodaysDate (),						/* Expiry Date.			*/
				FALSE,								/* Silent mode			*/
				 (local_rec.LL[0] == 'Y'),			/* Input Mode.			*/
				SR.lotControl							/* Lot controled item. 	*/
													/*----------------------*/
			);
		/*-----------------
		| Redraw screens. |
		-----------------*/
		strcpy (local_rec.LL, "Y");
		putval (line_cnt);

		lcount[3] = (line_cnt + 1 > lcount[3]) ? line_cnt + 1 : lcount[3];

		heading (1);
		scn_display (1);

		heading (3);
		scn_display (3);
		lcount[3] = TempLine;
		if (cc)
			return (EXIT_FAILURE);
		
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

int 
DisplayDetails (
 int flag)
{
	batch_flag = flag;

	if (pcwo_rec.order_status [0] != 'R')
	{
		abc_unlock (pcwo);
		print_mess (ML (mlPcMess132));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}

	if (flag)
	{
		strcpy (local_rec.order_no, pcwo_rec.order_no);
		DSP_FLD ("order_no");
	}
	else
	{
		strcpy (local_rec.batch_no, pcwo_rec.batch_no);
		DSP_FLD ("batch_no");
	}

	strcpy (local_rec.reqBrNo, pcwo_rec.req_br_no);
	strcpy (local_rec.reqWhNo, pcwo_rec.req_wh_no);
	strcpy (local_rec.recBrNo, pcwo_rec.rec_br_no);
	strcpy (local_rec.recWhNo, pcwo_rec.rec_wh_no);

	/* find requesting branch name */
	strcpy (esmr_rec.co_no, comm_rec.co_no);
	strcpy (esmr_rec.est_no, local_rec.reqBrNo);
	cc = find_rec (esmr, &esmr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, esmr, "DBFIND");
	strcpy (local_rec.reqBrName, esmr_rec.short_name);
	DSP_FLD ("reqBrNo");
	DSP_FLD ("reqBrName");

	/* find receiving branch name */
	strcpy (esmr_rec.co_no, comm_rec.co_no);
	strcpy (esmr_rec.est_no, local_rec.recBrNo);
	cc = find_rec (esmr, &esmr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, esmr, "DBFIND");
	strcpy (local_rec.recBrName, esmr_rec.short_name);
	DSP_FLD ("recBrNo");
	DSP_FLD ("recBrName");

	/* find requesting warehouse name */
	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, local_rec.reqBrNo);
	strcpy (ccmr_rec.cc_no, local_rec.reqWhNo);
	cc = find_rec (ccmr, &ccmr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");
	strcpy (local_rec.reqWhName, ccmr_rec.acronym);
	DSP_FLD ("reqWhNo");
	DSP_FLD ("reqWhName");

	/* find receiving warehouse name */
	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, local_rec.recBrNo);
	strcpy (ccmr_rec.cc_no, local_rec.recWhNo);
	cc = find_rec (ccmr, &ccmr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");
	strcpy (local_rec.recWhName, ccmr_rec.acronym);
	local_rec.recHhccHash = ccmr_rec.hhcc_hash;
	DSP_FLD ("recWhNo");
	DSP_FLD ("recWhName");

	hhwoHash = pcwo_rec.hhwo_hash;
	local_rec.prv_seq = pcwo_rec.rtg_seq;
	DSP_FLD ("prv_seq");

	local_rec.cre_dte = pcwo_rec.create_date;
	DSP_FLD ("cre_dte");

	local_rec.priority = pcwo_rec.priority;
	DSP_FLD ("priority");

	inmr_rec.hhbr_hash	=	pcwo_rec.hhbr_hash;
	cc = find_rec (inmr2, &inmr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, inmr, "DBFIND");

	ReadGlDefault	(inmr_rec.category);

	strcpy (local_rec.item_no, inmr_rec.item_no);
	sprintf (local_rec.strength, "%-5.5s", inmr_rec.description + 35);
	sprintf (local_rec.desc, "%-35.35s", inmr_rec.description);
	sprintf (local_rec.desc2, "%-40.40s", inmr_rec.description2);
	local_rec.hhbrHash = inmr_rec.hhbr_hash;
	DSP_FLD ("item_no");
	DSP_FLD ("strength");
	DSP_FLD ("desc");
	DSP_FLD ("desc2");

	inum_rec.hhum_hash	=	inmr_rec.std_uom;
	cc = find_rec (inum,&inum_rec,EQUAL, "r");
	strcpy (local_rec.std_uom, (cc) ? "    " : inum_rec.uom);

	inum_rec.hhum_hash	=	inmr_rec.alt_uom;
	cc = find_rec (inum,&inum_rec,EQUAL, "r");
	strcpy (local_rec.alt_uom, (cc) ? "    " : inum_rec.uom);
	DSP_FLD ("std_uom");
	DSP_FLD ("alt_uom");

	cc = FindInei (inmr_rec.hhbr_hash, comm_rec.est_no, "r");
	if (cc)
	{
		ineiRec.std_batch = 1;
		ineiRec.min_batch = 1;
		ineiRec.max_batch = 1;
		ineiRec.prd_multiple = 0;
	}
	local_rec.std_batch	= ineiRec.std_batch;
	local_rec.min_batch	= ineiRec.min_batch;
	local_rec.max_batch	= ineiRec.max_batch;
	sprintf (local_rec.std_batch_str, "%14.6f", ineiRec.std_batch);

	print_at (8,  69, "Std. %14.6f", ineiRec.std_batch);
	print_at (8,  90, "Min. %14.6f", ineiRec.min_batch);
	print_at (8, 111, "Max. %14.6f", ineiRec.max_batch);

	local_rec.bom_alt = pcwo_rec.bom_alt;
	DSP_FLD ("bom_alt");

	local_rec.rtg_alt = pcwo_rec.rtg_alt;
	DSP_FLD ("rtg_alt");

	local_rec.rqd_dte = pcwo_rec.reqd_date;
	DSP_FLD ("rqd_dte");

	local_rec.qty_rqd = pcwo_rec.prod_qty;
	DSP_FLD ("qty_rqd");

	/*--------------------------------------
	| Check whether job can only be closed |
	| or whether there is a valid next seq |
	--------------------------------------*/
	FLD ("to_seq") = YES;
	pcln_rec.hhwo_hash = pcwo_rec.hhwo_hash;
	pcln_rec.seq_no = pcwo_rec.rtg_seq + 1;
	pcln_rec.line_no = 0;
	cc = find_rec (pcln, &pcln_rec, GTEQ, "r");
	if (cc || pcln_rec.hhwo_hash != hhwoHash)
	{
		FLD ("to_seq") = NA;
		strcpy (local_rec.to_seq, "Closing");
		last_seq = pcwo_rec.rtg_seq;
		DSP_FLD ("to_seq");
	}
	return (EXIT_SUCCESS);
}

/*---------------------------
| Search for valid sequence |
---------------------------*/
void
SrchSeq (
 char *key_val)
{
	char	seq_str [10];
	int	seq_no;

	_work_open (3,0,40);
	save_rec ("#Seq.", "#Work Center");
	pcln_rec.hhwo_hash = hhwoHash;
	pcln_rec.seq_no = local_rec.prv_seq;
	pcln_rec.line_no = 0;
	cc = find_rec (pcln, &pcln_rec, GTEQ, "r");
	seq_no = -1;
	while (!cc && pcln_rec.hhwo_hash == hhwoHash)
	{
		if (pcln_rec.seq_no != seq_no &&
			pcln_rec.seq_no > local_rec.prv_seq)
		{
			sprintf (seq_str, "%2d ", pcln_rec.seq_no);

			pcwc_rec.hhwc_hash	= pcln_rec.hhwc_hash;
			cc = find_rec (pcwc, &pcwc_rec, COMPARISON, "r");
			if (cc)
				strcpy (pcwc_rec.work_cntr, "UNKNOWN ");

			cc = save_rec (seq_str, pcwc_rec.work_cntr);
			if (cc)
				break;

			seq_no = pcln_rec.seq_no;
		}
		cc = find_rec (pcln, &pcln_rec, NEXT, "r");
	}
	cc = save_rec ("C  ", "Closing ");
	if (cc)
		return;

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	if (temp_str [0] == 'C')
		strcpy (local_rec.to_seq, "C     ");
	else
		sprintf (local_rec.to_seq, "%2d    ", atoi (temp_str));
}

/*=======================
| Search for Resource	|
=======================*/
void
SrchRgrs (
 char *key_val)
{
	_work_open (8,0,40);
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

/*-----------------------------------------
| Delete a line from the queue TAB screen |
-----------------------------------------*/
int
DeleteResLine (
 void)
{
	int	i,
		this_page = line_cnt / TABLINES;

	if (prog_status == ENTRY)
	{
		/*print_mess (" Cannot Delete Lines on Entry ");*/
		print_mess (ML (mlStdMess005));
		return (EXIT_FAILURE);
	}

	lcount [2]--;

	for (i = line_cnt; line_cnt < lcount [2]; line_cnt++)
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
		/*print_mess (" Cannot Delete Lines on Entry ");*/
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

void
tab_other (
 int line_no)
{
	switch (curr_scn)
	{
	case 2:
		FLD ("rsrc") = NA;

		FLD ("act_date") = NA;
		FLD ("act_time") = NA;
		FLD ("qty_res") = NA;

		if (line_no >= num_res)
			FLD ("rsrc") = YES;

		if (local_rec.new_times == TRUE)
		{
			FLD ("act_date") = YES;
			FLD ("act_time") = YES;
			FLD ("qty_res") = YES;
		}

		break;

	case 3:
		if (line_no >= num_bps)
			FLD ("bp_item") = NO;
		else
			FLD ("bp_item") = NA;

		break;
	}
}

/*===============================================
| Does the user wish to print the picking list. |
===============================================*/
void
PrintPickingSlip (
 void)
{
	int		i;

	box (35, 13, 58, 3);
	sprintf (err_str, stringIssue, upd_seq);
	rv_pr (err_str, 37, 14, 0);

	sprintf (err_str, stringUpdate, curr_seq);
	rv_pr (err_str, 43, 15, 0);

	crsr_on ();
	rv_pr (stringpSlip, 40, 16, 1);

	i = prmptmsg (stringpSlip, "YyNn", 40,16);
	if (i == 'Y' || i == 'y')
	{
		print_at (16, 86, "%c", (i == 'Y') ? 'Y' : 'y');
		PrintMaterialPSlip ();
	}
}

/*-------------------------------------------
| Print picking slip for required materials |
-------------------------------------------*/
void
PrintMaterialPSlip (
 void)
{
	char	pipe_name [31];
	int		i;

	/*-----------------
	| Call pc_chk_iss |
	-----------------*/
	sprintf (pipe_name, "pc_chk_iss %2d %2d Y", local_rec.printerNumber, upd_seq);

	/*-----------------------------------------------------
	| Full details required for lot and location details. |
	-----------------------------------------------------*/
	box (40, 17, 48, (MULT_LOC) ? 3 : 2);
	crsr_on ();

	i = prmptmsg (stringLot, "YyNn", 52,18);
	if (i == 'Y' || i == 'y')
		strcat (pipe_name, " Y");
	else
		strcat (pipe_name, " N");

	if (MULT_LOC)
	{
		i = prmptmsg (stringBin, "YyNn", 52,19);
		if (i == 'Y' || i == 'y')
			strcat (pipe_name, " Y");
		else
			strcat (pipe_name, " N");

		i = prmptmsg (stringReq, "YyNn", 52,20);
		if (i == 'Y' || i == 'y')
			strcat (pipe_name, " Y");
		else
			strcat (pipe_name, " N");
	}
	else
	{
		strcat (pipe_name, " N");
		i = prmptmsg (stringReq, "YyNn", 47,19);
		if (i == 'Y' || i == 'y')
			strcat (pipe_name, " Y");
		else
			strcat (pipe_name, " N");
	}

	if (i == 'Y' || i == 'y')
		strcat (pipe_name, " Y");
	else
		strcat (pipe_name, " N");

	rv_pr (ML (mlStdMess035), 43, 21, 1);

	/*-------------------------
	| Open Pipe To pc_chk_iss |
	-------------------------*/
	if ((pout = popen (pipe_name, "w")) == (FILE *)0)
		sys_err ("Error in pc_chk_iss During (POPEN)", errno, PNAME);

	fprintf (pout, "%010ld\n", hhwoHash);

	fflush (pout);
	pclose (pout);

	return;
}

/*---------------
| Load pcrq_rec |
---------------*/
int
LoadPcrwRec (
 void)
{
	int	fst_pcrq;

	scn_set (2);
	lcount [2] = 0;

	/*------------------
	| Find work centre |
	------------------*/
	sprintf (local_rec.work_cntr, "%-8.8s", " ");

	pcln_rec.hhwo_hash = hhwoHash;
	pcln_rec.seq_no = curr_seq;
	pcln_rec.line_no = 0;
	cc = find_rec (pcln, &pcln_rec, COMPARISON, "r");
	if (!cc &&
		pcln_rec.hhwo_hash == hhwoHash &&
		pcln_rec.seq_no == curr_seq)
	{
		/*--------------------------------
		| Store work centre related info |
		| to be used at update/add time  |
		--------------------------------*/
		sprintf (yld_clc, "%-4.4s", pcln_rec.yld_clc);
		hhwcHash = pcln_rec.hhwc_hash;

		pcwc_rec.hhwc_hash	= hhwcHash;
		cc = find_rec (pcwc, &pcwc_rec, COMPARISON, "r");
		if (!cc)
		{
			sprintf (local_rec.work_cntr,
				"%-8.8s",
				pcwc_rec.work_cntr);
		}
	}

	/*--------------------
	| Load queue records |
	--------------------*/
	fst_pcrq = TRUE;
	pcrq_rec.hhwo_hash = hhwoHash;
	pcrq_rec.seq_no = curr_seq;
	pcrq_rec.line_no = 0;

	cc = find_rec (pcrq, &pcrq_rec, GTEQ, "r");
	while (!cc &&
		pcrq_rec.hhwo_hash == hhwoHash &&
		pcrq_rec.seq_no == curr_seq)
	{
		if (fst_pcrq)
			sprintf (prod_class, "%-4.4s", pcrq_rec.prod_class);

		rgrs_rec.hhrs_hash	=	pcrq_rec.hhrs_hash;
		cc = find_rec (rgrs, &rgrs_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, rgrs, "DBFIND");

		sprintf (local_rec.rsrc, "%-8.8s", rgrs_rec.code);
		sprintf (local_rec.rsrc_desc, "%-26.26s", rgrs_rec.desc);

		local_rec.est_date = pcrq_rec.est_date;
		local_rec.est_time = pcrq_rec.est_time;
		local_rec.est_setup = pcrq_rec.est_setup;
		local_rec.est_run = pcrq_rec.est_run;
		local_rec.est_clean = pcrq_rec.est_clean;

		if (pcrq_rec.act_date == 0L)
		{
			local_rec.act_date	= pcrq_rec.est_date;
			local_rec.act_time	= pcrq_rec.est_time;
			local_rec.act_setup	= pcrq_rec.est_setup;
			local_rec.act_run	= pcrq_rec.est_run;
			local_rec.act_clean	= pcrq_rec.est_clean;
			local_rec.st_setup	= 0L;
			local_rec.st_run	= 0L;
			local_rec.st_clean	= 0L;
			local_rec.new_times	= TRUE;
		}
		else
		{
			local_rec.act_date	= pcrq_rec.act_date;
			local_rec.act_time	= pcrq_rec.act_time;
			local_rec.act_setup	= pcrq_rec.act_setup;
			local_rec.act_run	= pcrq_rec.act_run;
			local_rec.act_clean	= pcrq_rec.act_clean;
			local_rec.st_setup	= pcrq_rec.act_setup;
			local_rec.st_run	= pcrq_rec.act_run;
			local_rec.st_clean	= pcrq_rec.act_clean;
			local_rec.new_times	= FALSE;
		}

		local_rec.qty_res = pcrq_rec.qty_rsrc;

		local_rec.hhrs_hash = pcrq_rec.hhrs_hash;

		putval (lcount [2]++);

		fst_pcrq = FALSE;

		cc = find_rec (pcrq, &pcrq_rec, NEXT, "r");
	}

	num_res = lcount [2];

	return (EXIT_SUCCESS);
}

/*----------------------------
| Load by-products from pcbp |
----------------------------*/
int
LoadByProducts (void)
{
	scn_set (3);
	lcount [3] = 0;

	pcbp_rec.hhwo_hash = hhwoHash;
	pcbp_rec.seq_no = curr_seq;
	pcbp_rec.hhbr_hash = 0L;
	cc = find_rec (pcbp, &pcbp_rec, GTEQ, "r");
	while (!cc &&
		pcbp_rec.hhwo_hash == hhwoHash &&
		pcbp_rec.seq_no == curr_seq)
	{
		inmr_rec.hhbr_hash = pcbp_rec.hhbr_hash;
		cc = find_rec (inmr2, &inmr_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (pcbp, &pcbp_rec, NEXT, "r");
			continue;
		}

		strcpy (store [lcount [3]].uom, inmr_rec.sale_unit);
		strcpy (store [lcount [3]].lotControl, inmr_rec.lot_ctrl);
		store [lcount [3]].hhbrHash = inmr_rec.hhbr_hash;

		/*-------------------
		| Find inum record. |
		-------------------*/
		strcpy (inum_rec.uom, inmr_rec.sale_unit);
		cc = find_rec (inum2, &inum_rec, EQUAL, "r");
		if (cc)
			file_err (cc, inum2, "DBFIND");

		store [lcount [3]].hhumHash   = inum_rec.hhum_hash;
		store [lcount [3]].cnvFct     = inum_rec.cnv_fct;

		sprintf (local_rec.bp_item, "%-16.16s", inmr_rec.item_no);
		sprintf (local_rec.bp_desc, "%-40.40s", inmr_rec.description);
		local_rec.bp_hhbr = pcbp_rec.hhbr_hash;
		if (inmr_rec.serial_item [0] == 'Y')
		{
			local_rec.bp_qty = 0.00;
			local_rec.ser_item = TRUE;
		}
		else
		{
			local_rec.bp_qty = pcbp_rec.qty;
			local_rec.ser_item = FALSE;
		}

		if (FindIncc ())
		{
			cc = AddIncc ();
			if (cc)
				file_err (cc, incc, "DBADD");
		}
		local_rec.hhwhHash = incc_rec.hhwh_hash;
		local_rec.hhum_hash = inmr_rec.std_uom;

		store [lcount [3]].hhwhHash = incc_rec.hhwh_hash;
		store [lcount [3]].hhccHash = incc_rec.hhcc_hash;

		/*------------------------------------
		| Load lot / location info for line. |
		------------------------------------*/
		cc = DisplayLL
			 (										/*----------------------*/
				lcount [3],							/*	Line number.		*/
				tab_row + 3 + (line_cnt % TABLINES),/*  Row for window		*/
				tab_col + 8,						/*  Col for window		*/
				4,									/*  length for window	*/
				store [lcount [3]].hhwhHash, 		/*	Warehouse hash.		*/
				store [lcount [3]].hhumHash,		/*	UOM hash			*/
				store [lcount [3]].hhccHash,		/*	CC hash.			*/
				store [lcount [3]].uom,			/* UOM					*/
				local_rec.bp_qty,					/* Quantity.			*/
				store [lcount [3]].cnvFct,		/* Conversion factor.	*/
				TodaysDate (),						/* Expiry Date.			*/
				TRUE,								/* Silent mode			*/
				TRUE,								/* Input Mode.			*/
				store [lcount [3]].lotControl			/* Lot controled item. 	*/
													/*----------------------*/
			);

		putval (lcount [3]++);

		cc = find_rec (pcbp, &pcbp_rec, NEXT, "r");
	}

	num_bps = lcount [3];

	return (EXIT_SUCCESS);
}

/*--------------------------------
| Update pcln recs and pcrq recs |
--------------------------------*/
int
UpdateRouteRecord (
 void)
{
	GL_ACC	*tmpAccount;
	int	i;
	double	tmp_diff = 0.00,
		tmp_tot = 0.00,
		ovh_tot = 0.00;
	long	old_time,
		new_time;

	scn_set (2);

	if (lcount [2] == 0)
		return (FALSE);

	/*-------------------------------
	| Read in pcwc for later update	|
	| to pcgl.			|
	-------------------------------*/
	pcwc_rec.hhwc_hash	= pcln_rec.hhwc_hash;
	cc = find_rec (pcwc, &pcwc_rec, EQUAL, "r");
	if (cc)
		file_err (cc, pcwc, "DBFIND");

	for (i = 0; i < lcount [2]; i++)
	{
		getval (i);

		/*---------------------------
		| Get info from rgrs record |
		---------------------------*/
		rgrs_rec.hhrs_hash = local_rec.hhrs_hash;
		cc = find_rec (rgrs, &rgrs_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, rgrs, "DBFIND");

		/*--------------------
		| Update or Add pcln |
		--------------------*/
		pcln_rec.hhwo_hash 	= hhwoHash;
		pcln_rec.seq_no 	= curr_seq;
		pcln_rec.line_no 	= i;
		cc = find_rec (pcln, &pcln_rec, COMPARISON, "u");
		pcln_rec.qty_rsrc = local_rec.qty_res;

		if (cc)
		{
			pcln_rec.hhrs_hash = rgrs_rec.hhrs_hash;
			pcln_rec.hhwc_hash = hhwcHash;
			pcln_rec.rate = rgrs_rec.rate;
			pcln_rec.ovhd_var = rgrs_rec.ovhd_var;
			pcln_rec.ovhd_fix = rgrs_rec.ovhd_fix;
			pcln_rec.instr_no = 0;
			sprintf (pcln_rec.yld_clc, "%-4.4s", yld_clc);
			strcpy (pcln_rec.can_split, "Y");
			strcpy (pcln_rec.act_qty_in, "N");

			cc = abc_add (pcln, &pcln_rec);
			if (cc)
				file_err (cc, pcln, "DBADD");
		}
		else
		{
			cc = abc_update (pcln, &pcln_rec);
			if (cc)
				file_err (cc, pcln, "DBUPDATE");
		}

		/*--------------------
		| Update or Add pcrq |
		--------------------*/
		pcrq_rec.hhwo_hash = hhwoHash;
		pcrq_rec.seq_no = curr_seq;
		pcrq_rec.line_no = i;
		cc = find_rec (pcrq, &pcrq_rec, COMPARISON, "u");

		pcrq_rec.act_date  = local_rec.act_date;
		pcrq_rec.act_time  = local_rec.act_time;
		pcrq_rec.act_setup = local_rec.act_setup;
		pcrq_rec.act_run   = local_rec.act_run;
		pcrq_rec.act_clean = local_rec.act_clean;

		pcrq_rec.qty_rsrc = local_rec.qty_res;
		strcpy (pcrq_rec.stat_flag, "A");

		if (cc)
		{
			pcrq_rec.hhrs_hash = local_rec.hhrs_hash;
			sprintf (pcrq_rec.prod_class, "%-4.4s", prod_class);
			pcrq_rec.priority = pcwo_rec.priority;

			pcrq_rec.last_date  = 0L;
			pcrq_rec.last_time = 0L;
			pcrq_rec.est_date  = 0L;
			pcrq_rec.est_time = 0L;
			pcrq_rec.est_setup = 0L;
			pcrq_rec.est_run   = 0L;
			pcrq_rec.est_clean = 0L;

			strcpy (pcrq_rec.can_split, "Y");
			strcpy (pcrq_rec.firm_sched, "Y");

			cc = abc_add (pcrq, &pcrq_rec);
			if (cc)
				file_err (cc, pcrq, "DBADD");
		}
		else
		{
			cc = abc_update (pcrq, &pcrq_rec);
			if (cc)
				file_err (cc, pcrq, "DBUPDATE");
		}
		old_time =	local_rec.st_setup +
				local_rec.st_run +
				local_rec.st_clean;
		new_time =	local_rec.act_setup +
				local_rec.act_run +
				local_rec.act_clean;
		tmp_diff = (double) new_time - (double) old_time;
		ovh_tot = tmp_diff * pcln_rec.ovhd_var;
		ovh_tot /= 60.00;
		ovh_tot += pcln_rec.ovhd_fix;
		ovh_tot *= pcln_rec.qty_rsrc;

		ovh_tot = no_dec (ovh_tot);
		AddPcgl
		 (
			hhmrWipOvh,
			hhmrWipOvhAcc,
			pcwc_rec.work_cntr,
			ovh_tot,
			"1"
		);
		AddPcgl
		 (
			hhmrRcvOvh,
			hhmrRcvOvhAcc,
			pcwc_rec.work_cntr,
			ovh_tot,
			"2"
		);

		tmp_tot = tmp_diff * pcln_rec.rate;
		tmp_tot /= 60.00;
		tmp_tot *= pcln_rec.qty_rsrc;
		DeAllocAccount ();
		GetGlAccount (tmp_tot, TRUE);

		tmpAccount = list_head;
		while (tmpAccount != GL_NULL)
		{
			tmp_tot = no_dec (tmpAccount->_amount);
			if (tmp_tot < 0.00)
			{
				tmp_tot = 0.00 - tmp_tot;
				AddPcgl
				 (
					tmpAccount->hhmrHash,
					tmpAccount->accountNumber,
					pcwc_rec.work_cntr,
					tmp_tot,
					"2"
				);
			}
			else
			{
				AddPcgl
				 (
					tmpAccount->hhmrHash,
					tmpAccount->accountNumber,
					pcwc_rec.work_cntr,
					tmp_tot,
					"1"
				);
			}
			tmpAccount = tmpAccount->_next;
		}
	}

	return (TRUE);
}

/*---------------------------------
| Update pcbp records and receipt |
| by-products into stock          |
---------------------------------*/
int
UpdateByProducts (
 void)
{
	int		j;
	int		mth_idx;
	long	expiry_date;
	char	wk_ref [11];

	scn_set (3);

	if (lcount [3] == 0)
		return (FALSE);

	for (line_cnt = 0; line_cnt < lcount [3]; line_cnt++)
	{
		int		i;

		/*---------------------
		| Update pcbp records |
		---------------------*/
		getval (line_cnt);
		if (local_rec.ser_item == TRUE)
			continue;

		inmr_rec.hhbr_hash	= local_rec.bp_hhbr;
		cc = find_rec (inmr2, &inmr_rec, COMPARISON, "u");
		if (cc)
			local_rec.bp_qty = 0.00;

		pcbp_rec.hhwo_hash = hhwoHash;
		pcbp_rec.seq_no = curr_seq;
		pcbp_rec.hhbr_hash = local_rec.bp_hhbr;
		cc = find_rec (pcbp, &pcbp_rec, COMPARISON, "u");

		pcbp_rec.qty = local_rec.bp_qty;
		strcpy (pcbp_rec.act_qty_in, "Y");

		if (cc)
		{
			if (pcbp_rec.qty != 0.00)
			{
				cc = abc_add (pcbp, &pcbp_rec);
				if (cc)
					file_err (cc, pcbp, "DBADD");
			}
		}
		else
		{
			cc = abc_update (pcbp, &pcbp_rec);
			if (cc)
				file_err (cc, pcbp, "DBUPDATE");
		}

		if (local_rec.bp_qty == 0.00)
			continue;

		/*--------------------
		| Update inmr record |
		--------------------*/
		inmr_rec.on_hand += local_rec.bp_qty;
		cc = abc_update (inmr2, &inmr_rec);
		if (cc)
			file_err (cc, inmr2, "DBUPDATE");
		/*------------------
		| Find inei record |
		------------------*/
		cc = FindInei (local_rec.bp_hhbr, comm_rec.est_no, "r");
		if (cc)
			file_err (cc, inei, "DBFIND");

		/*-----------------------
		| Calculate expiry date |
		-----------------------*/
		expiry_date = comm_rec.inv_date;
		for (j = 0; j < inei_expiry [0]; j++)
		{
			mth_idx = (periodMonth + j) % 12;
			expiry_date += (long) (mnths [mth_idx].noDays);
		}

		/*--------------------
		| Update incc record |
		--------------------*/
		if (FindIncc ())
		{
			cc = AddIncc ();
			if (cc)
				file_err (cc, incc, "DBADD");
		}
		incc_rec.receipts  += local_rec.bp_qty;
		incc_rec.ytd_receipts += local_rec.bp_qty;
		incc_rec.closing_stock = incc_rec.opening_stock
								  + incc_rec.pur
								  + incc_rec.receipts
								  + incc_rec.adj
								  - incc_rec.issues
								  - incc_rec.sales;
		cc = abc_update (incc, &incc_rec);
		if (cc)
			file_err (cc, incc, "DBFIND");

		open_rec (inwu, inwu_list, INWU_NO_FIELDS, "inwu_id_no");

		/*--------------------------------------
		| Find Warehouse unit of measure file. |
		--------------------------------------*/
		inwu_rec.hhwh_hash	=	incc_rec.hhwh_hash;
		inwu_rec.hhum_hash	=	inmr_rec.std_uom;
		cc = find_rec (inwu, &inwu_rec, COMPARISON,"u");
		if (cc)
		{
			memset (&inwu_rec, 0, sizeof (inwu_rec));
			inwu_rec.hhwh_hash	=	incc_rec.hhwh_hash;
			inwu_rec.hhum_hash	=	inmr_rec.std_uom;
			cc = abc_add (inwu, &inwu_rec);
			if (cc)
				file_err (cc, inwu, "DBADD");

			inwu_rec.hhwh_hash	=	incc_rec.hhwh_hash;
			inwu_rec.hhum_hash	=	inmr_rec.std_uom;
			cc = find_rec (inwu, &inwu_rec, COMPARISON,"u");
			if (cc)
				file_err (cc, inwu, "DBFIND");
		}
		inwu_rec.receipts  += local_rec.bp_qty;
		inwu_rec.closing_stock = inwu_rec.opening_stock +
								 inwu_rec.pur +
								 inwu_rec.receipts +
								 inwu_rec.adj -
								 inwu_rec.issues -
								 inwu_rec.sales;

		cc = abc_update (inwu,&inwu_rec);
		if (cc)
			file_err (cc, "inwu", "DBUPDATE");

		abc_fclose (inwu);

		/*--------------------
		| Add FIFO/LIFO cost |
		--------------------*/
		if (inmr_rec.costing_flag [0] == 'F' ||
		    inmr_rec.costing_flag [0] == 'I')
		{
			cc = AddIncf 
				(	
					incc_rec.hhwh_hash,
					comm_rec.inv_date,
					0.00, 0.00,
					local_rec.bp_qty,
					" ",
					0.00, 0.00, 0.00, 0.00, 0.00, 0.00,
					"A"
				);
			if (cc)
				file_err (cc, incf, "DBADD");
		}

		sprintf (wk_ref, "Seq. %2d   ", curr_seq);
		/*----------------
		| Create moverec |
		----------------*/
		for (i = 0; i < MAX_LOTS; i++)
		{
			if (!LL_Valid (line_cnt, i))
				break;

			MoveAdd 
			 (
				comm_rec.co_no,
				local_rec.recBrNo,
				local_rec.recBrNo,
				incc_rec.hhbr_hash,
				local_rec.recHhccHash,
				inmr_rec.std_uom,
				comm_rec.inv_date,
				10,			/* RECEIPT */
				pcwo_rec.order_no,
				inmr_rec.inmr_class,
				inmr_rec.category,
				"BY PRODUCT",
				wk_ref,
				GetBaseQty (line_cnt, i),
				0.00,
				0.00
			);
		}

		/*-----------------
		| Add sobg record |
		-----------------*/
		add_hash 
		 (
			comm_rec.co_no,
			local_rec.recBrNo,
			"RC",
			1,
			local_rec.bp_hhbr,
			local_rec.recHhccHash,
			0L,
			0.00
		);

		UpdateLotLocation (line_cnt, FALSE);
	}

	return (TRUE);
}

/*------------------------------------------------
| Add incc record and read back to get hhwh hash |
------------------------------------------------*/
int
AddIncc (void)
{
	char	temp_sort [29];

	incc_rec.hhcc_hash = currHhcc;
	incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
	sprintf (temp_sort, "%s%11.11s%-16.16s",
										inmr_rec.inmr_class,
										inmr_rec.category,
										inmr_rec.item_no);

	strcpy (incc_rec.sort,temp_sort);
	strcpy (incc_rec.stocking_unit,inmr_rec.sale_unit);
	incc_rec.opening_stock = 0.00;
	incc_rec.receipts = 0.00;
	incc_rec.pur = 0.00;
	incc_rec.issues = 0.00;
	incc_rec.adj = 0.00;
	incc_rec.sales = 0.00;
	incc_rec.closing_stock = 0.00;
	incc_rec.ytd_receipts = 0.00;

	incc_rec.first_stocked = local_rec.lsystemDate;
	strcpy (incc_rec.stat_flag,"0");

	cc = abc_add (incc,&incc_rec);
	if (cc)
		return (EXIT_FAILURE);

	cc = FindIncc ();
	if (cc)
		return (EXIT_FAILURE);

	return (EXIT_SUCCESS);
}

/*========================================================================
| Routine to find incc record. Returns 0 if found ok, 1 if not on file . |
========================================================================*/
int
FindIncc (
 void)
{
	incc_rec.hhcc_hash = currHhcc;
	incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
	cc = find_rec (incc, &incc_rec, COMPARISON, "r");
	if (cc)
		return (EXIT_FAILURE);

	return (EXIT_SUCCESS);
}

/*----------------------------
| Check if any materials are |
| required for next seq no.  |
----------------------------*/
int
NeedIssue (
 void)
{
	int	need_materials = FALSE;

	/*----------------------------------
	| Check upd_seq for material issue |
	----------------------------------*/
	pcms_rec.hhwo_hash = hhwoHash;
	pcms_rec.uniq_id = 0;
	cc = find_rec (pcms, &pcms_rec, GTEQ, "r");
	while (!cc && pcms_rec.hhwo_hash == hhwoHash)
	{
		if (pcms_rec.act_qty_in [0] == 'Y')
		{
			cc = find_rec (pcms, &pcms_rec, NEXT, "r");
			continue;
		}

		if (pcms_rec.iss_seq <= upd_seq)
		{
			need_materials = TRUE;
			break;
		}

		cc = find_rec (pcms, &pcms_rec, NEXT, "r");
	}

	return (need_materials);
}

/*----------------------------
| Call yield calc program if |
| a yield calc is required.  |
----------------------------*/
int
CheckYieldCalc (
 void)
{

	return (TRUE);
}

/*----------------------------
| Update pcwo to next seq no.|
----------------------------*/
int
Update (
 int new_seq)
{
	int		old_seq = pcwo_rec.rtg_seq;
	char	old_wc [9],
			new_wc [9];
    double	mat_out,
			oth_out,
			mat_in;

	/*---------------------------------
	| Update pcwo to new sequence no. |
	---------------------------------*/
	if (CLOSE_JOB && new_seq >= last_seq)
	{
		strcpy (pcwo_rec.order_status, "C");
		pcwo_rec.rtg_seq = last_seq;
		strcpy (err_str, "\007 JOB IS NOW CLOSING ");
	}
	else
	{
		pcwo_rec.rtg_seq = new_seq;
		sprintf (err_str,
			"\007 UPDATE TO SEQUENCE %2d SUCCESSFUL ",
			new_seq);
	}

	if (batch_flag)
		cc = abc_update (pcwo2, &pcwo_rec);
	else
		cc = abc_update (pcwo, &pcwo_rec);
	if (cc)
		file_err (cc, pcwo, "DBUPDATE");

	crsr_off ();
	box (46, 14, 38, 1);
	rv_pr (err_str, 48, 15, 1);
	sleep (sleepTime);

	if (old_seq == 0)
		return (EXIT_SUCCESS);
	if (CLOSE_JOB && new_seq > last_seq)
		return (EXIT_SUCCESS);

	/*-----------------------
	| Add G/L transactions.	|
	-----------------------*/
	pcln_rec.hhwo_hash	= pcwo_rec.hhwo_hash;
	pcln_rec.seq_no	= old_seq;
	pcln_rec.line_no	= 0;
	cc = find_rec (pcln, &pcln_rec, GTEQ, "r");
	if (	cc ||
		pcln_rec.hhwo_hash != pcwo_rec.hhwo_hash ||
		pcln_rec.seq_no    != old_seq)
	{
		file_err (1, pcln, "DBFIND");
	}

	pcwc_rec.hhwc_hash = pcln_rec.hhwc_hash;
	cc = find_rec (pcwc, &pcwc_rec, EQUAL, "r");
	strcpy (old_wc, pcwc_rec.work_cntr);

	pcln_rec.hhwo_hash	= pcwo_rec.hhwo_hash;
	pcln_rec.seq_no	= new_seq;
	pcln_rec.line_no	= 0;
	cc = find_rec (pcln, &pcln_rec, GTEQ, "r");
	if (cc ||
	    pcln_rec.hhwo_hash != pcwo_rec.hhwo_hash ||
	    pcln_rec.seq_no != new_seq)
	{
		file_err (1, pcln, "DBFIND");
	}

	pcwc_rec.hhwc_hash = pcln_rec.hhwc_hash;
	strcpy (new_wc, pcwc_rec.work_cntr);

	mat_out  = AmountIssue (pcwo_rec.hhwo_hash);
	mat_out += AmountRoute (pcwo_rec.hhwo_hash,FALSE,0,old_seq - 1,old_wc);
	AddPcgl (hhmrWipMat, hhmrWipMatAcc, old_wc, mat_out, "2");

	oth_out  = AmountRoute (pcwo_rec.hhwo_hash,TRUE,old_seq,old_seq,old_wc);

	mat_in   = mat_out + oth_out;
	AddPcgl (hhmrWipMat, hhmrWipMatAcc, new_wc, mat_in, "1");

	return (EXIT_SUCCESS);
}

void
EraseBitsOfScreen (
 void)
{
	int	i;

	for (i = 12; i < 21; i++)
	{
		move (0, i);
		cl_line ();
	}
	move (0, 21);
	line (132);
}

/*-----------------------
| Return the $ value	|
| of issues (to-date).	|
-----------------------*/
double	
AmountIssue (
 long int hhwoHash)
{
	double	iss_amt = 0.00;

	pcms_rec.hhwo_hash = hhwoHash;
	pcms_rec.uniq_id = 0;
	cc = find_rec (pcms, &pcms_rec, GTEQ, "r");
	while (!cc && pcms_rec.hhwo_hash == hhwoHash)
	{
		iss_amt += pcms_rec.amt_issued;
		cc = find_rec (pcms, &pcms_rec, NEXT, "r");
	}

	return (iss_amt);
}

/*-------------------------------
| Return the $val of work done	|
| between the given seq nos.	|
| If flag == TRUE then perform	|
| each individual posting to	|
| the pcgl table.		|
| Either way, return total.	|
-------------------------------*/
double	
AmountRoute (
 long int hhwoHash, 
 int      flag, 
 int      beg_seq_no, 
 int      end_seq_no, 
 char     *wrk_cnt)
{
	GL_ACC	*tmpAccount;
	double	cons_amt = 0.00,
		tmp_amt;

	pcln_rec.hhwo_hash = hhwoHash;
	pcln_rec.seq_no = beg_seq_no;
	pcln_rec.line_no = 0;
	cc = find_rec (pcln, &pcln_rec, GTEQ, "r");
	while
	 (
		!cc &&
		pcln_rec.hhwo_hash == hhwoHash &&
		pcln_rec.seq_no <= end_seq_no
	)
	{
		pcrq_rec.hhwo_hash	= pcln_rec.hhwo_hash;
		pcrq_rec.seq_no	= pcln_rec.seq_no;
		pcrq_rec.line_no	= pcln_rec.line_no;
		cc = find_rec (pcrq, &pcrq_rec, EQUAL, "r");
		if (cc)
			file_err (cc, pcrq, "DBFIND");

		rgrs_rec.hhrs_hash	= pcln_rec.hhrs_hash;
		cc = find_rec (rgrs, &rgrs_rec, EQUAL, "r");
		if (cc)
			file_err (cc, rgrs, "DBFIND");
		tmp_amt	= pcrq_rec.act_setup +
			  pcrq_rec.act_run +
			  pcrq_rec.act_clean;
		tmp_amt	*= pcln_rec.ovhd_var;
		tmp_amt	/= (double) 60.00;
		tmp_amt	+= pcln_rec.ovhd_fix;
		tmp_amt *= pcln_rec.qty_rsrc;
		cons_amt += tmp_amt;
		if (flag)
		    AddPcgl (hhmrWipOvh, hhmrWipOvhAcc, wrk_cnt, tmp_amt, "2");

		tmp_amt	= pcrq_rec.act_setup +
			  pcrq_rec.act_run +
			  pcrq_rec.act_clean;
		tmp_amt	*= pcln_rec.rate;
		tmp_amt *= pcln_rec.qty_rsrc;
		tmp_amt	/= (double) 60.00;
		cons_amt += tmp_amt;
		if (flag)
		{
		    DeAllocAccount ();
		    GetGlAccount (tmp_amt, FALSE);

		    tmpAccount = list_head;
		    while (tmpAccount != GL_NULL)
		    {
			tmp_amt = no_dec (tmpAccount->_amount);
			if (tmp_amt < 0.00)
			{
				tmp_amt = 0.00 - tmp_amt;
				AddPcgl
				 (
					tmpAccount->hhmrHash,
					tmpAccount->accountNumber,
					wrk_cnt,
					tmp_amt,
					"1"
				);
			}
			else
			{
				AddPcgl
				 (
					tmpAccount->hhmrHash,
					tmpAccount->accountNumber,
					wrk_cnt,
					tmp_amt,
					"2"
				);
			}
			tmpAccount = tmpAccount->_next;
		    }
		}

		cc = find_rec (pcln, &pcln_rec, NEXT, "r");
	}

	return (cons_amt);
}

/*===============================
| Add a trans to the pcgl file.	|
| NB: amount should be in cents	|
===============================*/
void
AddPcgl 
 (
	long	hash,
	char	*accountNumber,
	char	*wc,
	double	amount,
	char	*type
)
{
	int		monthPeriod;

	if (amount == 0.00)
		return;

	strcpy (pcgl_rec.co_no, comm_rec.co_no);
	strcpy (pcgl_rec.tran_type, "19");
	pcgl_rec.post_date = comm_rec.inv_date;
	pcgl_rec.tran_date = comm_rec.inv_date;

	DateToDMY (comm_rec.inv_date, NULL, &monthPeriod, NULL);
	sprintf (pcgl_rec.period_no, "%02d", monthPeriod);

	sprintf (pcgl_rec.sys_ref, "%5.1d", comm_rec.term);
	sprintf (pcgl_rec.user_ref, "%8.8s", wc);
	strcpy (pcgl_rec.stat_flag, "2");
	sprintf (pcgl_rec.narrative, "%-20.20s", pcwo_rec.order_no);
	pcgl_rec.amount 	= amount;
	pcgl_rec.loc_amount = amount;
	pcgl_rec.exch_rate 	= 1.00;
	strcpy (pcgl_rec.currency, localCurrency);

	strcpy (pcgl_rec.acc_no, accountNumber);
	pcgl_rec.hhgl_hash = hash;

	strcpy (pcgl_rec.jnl_type, type);
	cc = abc_add (pcgl, &pcgl_rec);
	if (cc)
		file_err (cc, pcgl, "DBADD");
}

void
GetGlAccount (
 double amount, 
 int    flag)
{
	GL_ACC	*tmpAccount;
	char	accountNumber [MAXLEVEL + 1];
	long	hhmrHash;

	if (rgrs_rec.type [0] == 'L')
	{
		hhmrHash = hhmrWipLbr;
		strcpy (accountNumber, hhmrWipLbrAcc);
	}
	else
	{
		hhmrHash = hhmrWipOth;
		strcpy (accountNumber, hhmrWipOthAcc);
	}

	tmpAccount = list_head;
	while (tmpAccount != GL_NULL)
	{
		if (tmpAccount->hhmrHash == hhmrHash)
		{
			tmpAccount->_amount += amount;
			break;
		}
		tmpAccount = tmpAccount->_next;
	}

	if (tmpAccount == GL_NULL)
	{
		tmpAccount = AllocateAccount ();
		tmpAccount->hhmrHash = hhmrHash;
		strcpy (tmpAccount->accountNumber, accountNumber);
		tmpAccount->_amount = amount;
		tmpAccount->_next = list_head;
		list_head = tmpAccount;
	}

	if (!flag)
		return;

	if (rgrs_rec.type [0] == 'L')
	{
		hhmrHash = hhmrRcvLbr;
		strcpy (accountNumber, hhmrRcvLbrAcc);
	}
	else
	{
		hhmrHash = hhmrRcvOth;
		strcpy (accountNumber, hhmrRcvOthAcc);
	}

	tmpAccount = list_head;
	while (tmpAccount != GL_NULL)
	{
		if (tmpAccount->hhmrHash == hhmrHash)
		{
			tmpAccount->_amount -= amount;
			break;
		}
		tmpAccount = tmpAccount->_next;
	}

	if (tmpAccount == GL_NULL)
	{
		tmpAccount = AllocateAccount ();
		tmpAccount->hhmrHash = hhmrHash;
		strcpy (tmpAccount->accountNumber, accountNumber);
		tmpAccount->_amount = 0.00 - amount;
		tmpAccount->_next = list_head;
		list_head = tmpAccount;
	}
}

GL_ACC	*
AllocateAccount (
 void)
{
	GL_ACC	*tmpAccount;
	int	i;

	if (free_head != GL_NULL)
	{
		tmpAccount = free_head;
		free_head = free_head->_next;
		return (tmpAccount);
	}

	for (i = 0; i < 100; i++)
	{
		tmpAccount = (GL_ACC *) malloc ((unsigned) sizeof (GL_ACC));
		if (tmpAccount != GL_NULL)
			return (tmpAccount);
	}

	sys_err ("Error in AllocateAccount () During (MALLOC)", 12, PNAME);
	return (NULL);
}

void
DeAllocAccount (
 void)
{
	GL_ACC	*tmpAccount = list_head;

	while (tmpAccount != GL_NULL)
	{
		list_head = list_head->_next;
		tmpAccount->_next = free_head;
		free_head = tmpAccount;
		tmpAccount = list_head;
	}
}


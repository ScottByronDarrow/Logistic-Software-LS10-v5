/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: pc_womaint.c,v 5.6 2002/07/24 08:39:01 scott Exp $
|  Program Name  : (pc_womaint.c  )                                   |
|  Program Desc  : (Works Order Maintenance.                    )     |
|---------------------------------------------------------------------|
|  Date Written  : 04/02/92        | Author       : Trevor van Bremen |
|---------------------------------------------------------------------|
| $Log: pc_womaint.c,v $
| Revision 5.6  2002/07/24 08:39:01  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.5  2002/07/08 04:40:01  scott
| S/C 004079 - Moved date field
|
| Revision 5.4  2002/07/03 04:20:15  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.3  2001/10/17 09:30:59  robert
| Updated from scott's machine
|
| Revision 5.2  2001/08/09 09:14:58  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:35:22  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:10:41  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.1  2001/04/04 07:35:26  scott
| Updated to adjust screen to look better with LS10-GUI
| Updated to perform routine maintenance to ensure standards are maintained.
| Added print_mess rather than dsp_screen for LS10-GUI
| Updated to re-new works order lines on maintenance, update did not perform
| consistantly in fact it performance was quite strange.
|
| Revision 4.0  2001/03/09 02:31:46  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.2  2001/01/25 07:07:00  cha
| Updated as while was missing a !cc
|
| Revision 3.1  2001/01/25 00:46:20  scott
| Updated to remove changed made by NZ regarding names pipe stuff.
| Does not work with LS10-GUI and is very non standard.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: pc_womaint.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PC/pc_womaint/pc_womaint.c,v 5.6 2002/07/24 08:39:01 scott Exp $";

#define	TXT_REQD
#define	TABLINES	5
#include	<pslscr.h>
#include	<proc_sobg.h>
#include	<twodec.h>
#include	<pr_format3.h>
#include	<ml_pc_mess.h>
#include	<ml_std_mess.h>
#define		NDEC(x)			n_dec (x, inmr_rec.dec_pt)
#define LINES	25

#include	"schema"

struct bmmsRecord	bmms_rec;
struct bmmsRecord	bmms2_rec;
struct ccmrRecord	ccmr_rec;
struct commRecord	comm_rec;
struct cumrRecord	cumr_rec;
struct ineiRecord	inei_rec;
struct inmrRecord	inmr_rec;
struct inmrRecord	inmr2_rec;
struct inumRecord	inum_rec;
struct pcbpRecord	pcbp_rec;
struct pclnRecord	pcln_rec;
struct pcmsRecord	pcms_rec;
struct pcoiRecord	pcoi_rec;
struct pcwlRecord	pcwl_rec;
struct pcwoRecord	pcwo_rec;
struct pcwoRecord	pcwo2_rec;
struct rgbpRecord	rgbp_rec;
struct rghrRecord	rghr_rec;
struct rglnRecord	rgln_rec;
struct inccRecord	incc_rec;
struct esmrRecord	esmr_rec;

	char	*bmms2	= "bmms2",
			*data	= "data",
			*inmr2	= "inmr2",
			*pcwo2	= "pcwo2",
			*pcwo3	= "pcwo3";

	char	*scn_desc [] =
	{
		"Maintain Production Details",
		"Maintain Customer(s) and Quantities",
		"Maintain Special Instructions"
	};
	int		newOrder 			= FALSE,
			newBomAlt 			= TRUE,
			newRtgAlt 			= TRUE,
			envVarDbCo			= TRUE,
			envVarPcGenNum		= TRUE,
			batchFlag 			= FALSE,
			worksOrderReleased 	= FALSE;

	float	qtyReqCalc 		= 0.00,
			stdBatchSize 	= 0.00;

	long	currentHhccHash;
	char	*currentUser;

	FILE	*fout,
			*fin;

struct storeRec
{
	long	hhcuHash;
	float	customerQty;
} store [MAXLINES];

/*============================
| Local & Screen Structures. |
============================*/
struct
{
	char	mfgBrNo [3];
	char	mfgBrName [16];
	char	mfgWhNo [3];
	char	mfgWhName [10];
	char	reqBrNo [3];
	char	reqBrName [16];
	char	reqWhNo [3];
	char	reqWhName [10];
	char	recBrNo [3];
	char	recBrName [16];
	char	recWhNo [3];
	char	recWhName [10];
	char	orderNumber [8];
	char	batchNumber [11];
	char	status [21];
	int		priority;
	char	systemDate [11];
	long	lsystemDate;
	long	createDate;
	long	requiredDate;
	long	hhbrHash;
	char	itemNumber [17];
	int		bomAlternate;
	int		rtgAlternate;
	int		bomAlternate_old;
	int		rtgAlternate_old;
	char	strength [6];
	char	desc [36];
	char	desc2 [41];
	char	standardUOM [5];
	char	alternateUOM [5];
	double	qtyRequired;
	char	stdBatchString [15];
	double	stdBatch;
	double	minBatch;
	double	maxBatch;
	float	prdMult;
	char	customerNumber [7];
	char	customerName [41];
	float	customerQty;
	char	textLine [71];
	char	dummy [11];
	int		printerNumber;
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "mfgBrNo",	 2, 18, CHARTYPE,
		"AA", "          ",
		" ", comm_rec.est_no, "Mfg Branch  :", "Manufacturing Branch.",
		NE, NO,  JUSTRIGHT, "", "", local_rec.mfgBrNo},
	{1, LIN, "mfgBrName",	 2, 25, CHARTYPE,
		"UUUUUUUUUUUUUUU", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.mfgBrName},
	{1, LIN, "mfgWhNo",	 2, 75, CHARTYPE,
		"AA", "          ",
		" ", comm_rec.cc_no, "Mfg Warehouse:", "Manufacturing Warehouse.",
		NE, NO,  JUSTRIGHT, "", "", local_rec.mfgWhNo},
	{1, LIN, "mfgWhName",	 2, 82, CHARTYPE,
		"UUUUUUUUU", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.mfgWhName},
	{1, LIN, "orderNumber",	 3, 18, CHARTYPE,
		"UUUUUUU", "          ",
		" ", " ", "Order Number:", " ",
		NO, NO,  JUSTLEFT, "", "", local_rec.orderNumber},
	{1, LIN, "batchNumber",	 3, 45, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", " ", "Batch Number:", " ",
		NE, NO,  JUSTLEFT, "", "", local_rec.batchNumber},
	{1, LIN, "priority",	 3, 95, INTTYPE,
		"N", "          ",
		" ", "5", "Priority:", " ",
		NO, NO,  JUSTLEFT, "", "", (char *) &local_rec.priority},
	{1, LIN, "createDate",	 3, 116, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "", "Date Raised:", " ",
		NA, NO,  JUSTLEFT, "", "", (char *) &local_rec.createDate},
	{1, LIN, "itemNumber",	 5, 18, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "", "Item Number:", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.itemNumber},
	{1, LIN, "bomAlternate",	 5, 55, INTTYPE,
		"NNNNN", "          ",
		" ", "0", "BOM Alt. No. : ", " ",
		NO, NO,  JUSTRIGHT, "0", "32767", (char *)&local_rec.bomAlternate},
	{1, LIN, "rtgAlternate",	 6, 55, INTTYPE,
		"NNNNN", "          ",
		" ", "0", "Routing No.  : ", " ",
		NO, NO,  JUSTRIGHT, "0", "32767", (char *)&local_rec.rtgAlternate},
	{1, LIN, "strength",	 6, 18, CHARTYPE,
		"AAAAA", "          ",
		" ", "", "Strength   :", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.strength},
	{1, LIN, "desc",		 8, 18, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description:", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.desc},
	{1, LIN, "desc2",	 9, 18, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description:", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.desc2},
	{1, LIN, "standardUOM",	 6, 88, CHARTYPE,
		"AAAA", "          ",
		"", "", "Standard:", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.standardUOM},
	{1, LIN, "alternateUOM",	 6, 110, CHARTYPE,
		"AAAA", "          ",
		"", "", "  Alternate:", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.alternateUOM},
	{1, LIN, "qtyRequired",	11, 18, DOUBLETYPE,
		"NNNNNNN.NNNNNN", "          ",
		" ", local_rec.stdBatchString, "Quantity Reqd:", " ",
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.qtyRequired},
	{1, LIN, "requiredDate",	12, 18, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", local_rec.systemDate, "Reqd Date    :", " ",
		NO, NO, JUSTLEFT, "", "", (char *) &local_rec.requiredDate},
	{1, LIN, "reqBrNo",	 11, 60, CHARTYPE,
		"AA", "          ",
		" ", comm_rec.est_no, "Request Br:", "Requesting Branch.",
		YES, NO,  JUSTRIGHT, "", "", local_rec.reqBrNo},
	{1, LIN, "reqBrName",	 11, 65, CHARTYPE,
		"UUUUUUUUUUUUUUU", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.reqBrName},
	{1, LIN, "reqWhNo",	 11, 105, CHARTYPE,
		"AA", "          ",
		" ", comm_rec.cc_no, "Wh:", "Requesting Warehouse.",
		YES, NO,  JUSTRIGHT, "", "", local_rec.reqWhNo},
	{1, LIN, "reqWhName",	 11, 110, CHARTYPE,
		"UUUUUUUUU", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.reqWhName},
	{1, LIN, "recBrNo",	 12, 60, CHARTYPE,
		"AA", "          ",
		" ", local_rec.mfgBrNo, "Receive Br:", "Receiving Branch.",
		YES, NO,  JUSTRIGHT, "", "", local_rec.recBrNo},
	{1, LIN, "recBrName",	 12, 65, CHARTYPE,
		"UUUUUUUUUUUUUUU", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.recBrName},
	{1, LIN, "recWhNo",	 12, 105, CHARTYPE,
		"AA", "          ",
		" ", local_rec.mfgWhNo, "Wh:", "Receiving Warehouse.",
		YES, NO,  JUSTRIGHT, "", "", local_rec.recWhNo},
	{1, LIN, "recWhName",	 12, 110, CHARTYPE,
		"UUUUUUUUU", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.recWhName},

	{2, TAB, "customerNumber",	MAXLINES, 2, CHARTYPE,
		"UUUUUU", "          ",
		" ", "000000", "Customer", " ",
		YES, NO, JUSTLEFT, "", "", local_rec.customerNumber},
	{2, TAB, "customerName",	 0, 1, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "               Customer Name              ", " ",
		NA, NO, JUSTLEFT, "", "", local_rec.customerName},
	{2, TAB, "customerQty",	 0, 1, FLOATTYPE,
		"NNNNNNN.NNNNNN", "          ",
		" ", "0.00", "       Quantity ", " ",
		NO, NO, JUSTRIGHT, "", "", (char *) &local_rec.customerQty},

#ifdef GVISION
	{3, TXT, "spec_inst",	14, 70, 0,
		"", "          ",
		" ", " ", "S P E C I A L   I N S T R U C T I O N S", " ",
		6, 60, 100, "", "", local_rec.textLine},
#else
	{3, TXT, "spec_inst",	13, 70, 0,
		"", "          ",
		" ", " ", "S P E C I A L   I N S T R U C T I O N S", " ",
		7, 60, 100, "", "", local_rec.textLine},
#endif	/* GVISION */

	{0, LIN, "",		 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};
#include	<SrchPcwo2.h>
#include    <pslscr.h>
#include    <FindCumr.h>

/*====================
| function prototype |
====================*/
int 	AddIncc 				(long);
int 	CheckPcwo 				(long);
int 	DeleteLine 				(void);
int 	DisplayDetails 			(int);
int 	HeadOutput 				(void);
int 	PageHead 				(void);
int 	PageTrail 				(void);
int 	PrintRequistion 		(void);
int 	ValidQuantity 			(double, int);
int 	heading 				(int);
int 	spec_valid 				(int);
void	SrchAltBom 				(void);
void 	CheckIncc 				(long, long);
void 	CloseDB 				(void);
void 	LoadCustomerInfo 		(void);
void 	LoadPcoi 				(void);
void 	OpenDB 					(void);
void 	SrchAltRtg 				(void);
void 	SrchCcmr 				(char *, char *);
void 	SrchEsmr 				(char *);
void 	Update 					(void);
void 	UpdatePcbp 				(void);
void 	UpdatePcln 				(void);
void 	UpdatePcms 				(void);
void 	UpdatePcoi 				(void);
void 	UpdatePcwl 				(void);
void 	UpdatePcwo 				(void);
void 	shutdown_prog 			(void);

extern	int		manufacturingSrch;

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int  argc, 
 char *argv[])
{
	char	*sptr;
	int		i;

	manufacturingSrch	=	TRUE;

	/* check program name */
	sptr = strrchr (argv [0], '/');
	if (sptr == (char *) 0)
		sptr = argv [0];
	else
		sptr ++;
	if (!strncmp (sptr, "pc_womaint", 10))
		worksOrderReleased = FALSE;
	else
		worksOrderReleased = TRUE;

	if (!worksOrderReleased && argc != 2)
	{
		print_at (0,0, mlStdMess036, argv [0]);
		return (EXIT_FAILURE);
	}
	if (!worksOrderReleased)
		local_rec.printerNumber = atoi (argv [1]);

	/*-------------------------------------------------------
	| Works order number is M(anually or S(ystem generated. |
	-------------------------------------------------------*/
	sptr = chk_env ("PC_GEN_NUM");
	if (sptr)
		envVarPcGenNum = (*sptr == 'M' || *sptr == 'm') ? FALSE : TRUE;
	else
		envVarPcGenNum = TRUE;
	/* get user if */
	currentUser = getenv ("LOGNAME");

	SETUP_SCR (vars);


	/*-------------------------------
	| Setup required parameters	|
	-------------------------------*/
	init_scr ();
	set_tty ();
	set_masks ();
/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (2, store, sizeof (struct storeRec));
#endif

	OpenDB ();
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	envVarDbCo = atoi (get_env ("DB_CO"));

	strcpy (local_rec.systemDate, DateToString (TodaysDate()));
	local_rec.lsystemDate = TodaysDate ();

	swide ();

	tab_row = 14;
	tab_col = 0;
	for (i = 0; i < 3; i++)
		tab_data [i]._desc = scn_desc [i];

	/*=======================================
	| Beginning of input control loop	|
	=======================================*/
	while (prog_exit == 0)
	{
		/*-----------------------
		| Reset control flags	|
		-----------------------*/
		entry_exit = 0;
		edit_exit = 0;
		prog_exit = 0;
		restart = 0;
		search_ok = 1;

		lcount [2] = 0;
		lcount [3] = 0;
		edit_ok (1);
		edit_ok (2);
		edit_ok (3);
		init_vars (3);
		init_vars (2);
		init_vars (1);
		local_rec.stdBatch = 0.00;
		local_rec.minBatch = 0.00;
		local_rec.maxBatch = 0.00;

		newBomAlt = TRUE;
		newRtgAlt = TRUE;
		local_rec.bomAlternate_old = 0;
		local_rec.rtgAlternate_old = 0;
		qtyReqCalc 	 = 0.00;
		stdBatchSize = 0.00;

		FLD ("orderNumber") = NO;
		FLD ("batchNumber") = NE;
		/*-------------------------------
		| Enter screen 1 linear input	|
		-------------------------------*/
		strcpy (local_rec.status, "                    ");
		heading (1);
		entry (1);
		if (restart || prog_exit)
		{
			abc_unlock (pcwo);
			continue;
		}

		/*-------------------------------
		| Edit screens as required.	|
		-------------------------------*/
		if (pcwo_rec.order_status [0] == 'P')
			FLD ("rtgAlternate") = NO;
		if (envVarPcGenNum)
			FLD ("orderNumber") = NA;
		edit_all ();
		if (restart)
		{
			abc_unlock (pcwo);
			continue;
		}

		if (worksOrderReleased)
		{
			newBomAlt = FALSE;
			newRtgAlt = FALSE;
		}

		Update ();

		if (envVarPcGenNum && newOrder && !worksOrderReleased)
		{
			clear ();
			print_at (0,0, ML(mlPcMess096), pcwo_rec.order_no);
			print_at (2,0, ML(mlPcMess113)); 
			PauseForKey (3,0, ML(mlStdMess042),0);
		}

		if (!worksOrderReleased)
			PrintRequistion ();

	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*=======================
| Program exit sequence	|
=======================*/
void
shutdown_prog (void)
{
	CloseDB (); 
	FinishProgram ();
}

/*=======================
| Open data base files	|
=======================*/
void
OpenDB (void)
{
	abc_dbopen (data);

	abc_alias (bmms2, bmms);
	abc_alias (inmr2, inmr);
	abc_alias (pcwo2, pcwo);
	abc_alias (pcwo3, pcwo);

	open_rec (bmms,  bmms_list, BMMS_NO_FIELDS, "bmms_id_no");
	open_rec (bmms2, bmms_list, BMMS_NO_FIELDS, "bmms_id_no_2");
	open_rec (cumr,  cumr_list, CUMR_NO_FIELDS, "cumr_id_no");
	open_rec (inei,  inei_list, INEI_NO_FIELDS, "inei_id_no");
	open_rec (inmr,  inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (inmr2, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (inum,  inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	open_rec (pcbp,  pcbp_list, PCBP_NO_FIELDS, "pcbp_id_no");
	open_rec (pcln,  pcln_list, PCLN_NO_FIELDS, "pcln_id_no");
	open_rec (pcms,  pcms_list, PCMS_NO_FIELDS, "pcms_id_no");
	open_rec (pcoi,  pcoi_list, PCOI_NO_FIELDS, "pcoi_id_no");
	open_rec (pcwl,  pcwl_list, PCWL_NO_FIELDS, "pcwl_id_no");
	open_rec (pcwo,  pcwo_list, PCWO_NO_FIELDS, "pcwo_id_no");
	open_rec (pcwo2, pcwo_list, PCWO_NO_FIELDS, "pcwo_id_no3");
	open_rec (rgbp,  rgbp_list, RGBP_NO_FIELDS, "rgbp_id_no");
	open_rec (rghr,  rghr_list, RGHR_NO_FIELDS, "rghr_id_no");
	open_rec (rgln,  rgln_list, RGLN_NO_FIELDS, "rgln_id_no");
	open_rec (ccmr,  ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	open_rec (incc,  incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (esmr,  esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
}

/*=======================
| Close data base files	|
=======================*/
void
CloseDB (void)
{
	abc_fclose (bmms);
	abc_fclose (bmms2);
	abc_fclose (cumr);
	abc_fclose (inei);
	abc_fclose (inmr);
	abc_fclose (inmr2);
	abc_fclose (inum);
	abc_fclose (pcbp);
	abc_fclose (pcln);
	abc_fclose (pcms);
	abc_fclose (pcoi);
	abc_fclose (pcwl);
	abc_fclose (pcwo);
	abc_fclose (pcwo2);
	abc_fclose (rgbp);
	abc_fclose (rghr);
	abc_fclose (rgln);
	abc_fclose (ccmr);
	abc_fclose (incc);
	abc_fclose (esmr);

	SearchFindClose ();
	abc_dbclose (data);
}

/*=======================
| Display heading scrn	|
=======================*/
int
heading (
 int scn)
{
	if (!restart)
	{
		swide ();
		clear ();
		if (!worksOrderReleased)
		{
			/*-" Works Order Maintenance "-*/
			sprintf (err_str, " %s ", ML(mlPcMess097));
			rv_pr (err_str, 53, 0, 1);
		}
		else
		{
			/*-" Released Works Order Maintenance "-*/
			sprintf (err_str, " %s ", ML(mlPcMess098));
			rv_pr (err_str, 49, 0, 1);

		}

		box (0, 1, 132, 11);
		box (67, 4, 65, 5);
		move (0, 4); PGCHAR (10); line (67); PGCHAR (8); line (64); PGCHAR (11);
		move (67, 7); PGCHAR (10); line (64); PGCHAR (11);
		move (0, 10); PGCHAR (10); line (67); PGCHAR (9); line (64); PGCHAR (11);

		rv_pr (local_rec.status, 59, 3, FALSE);
		/*--" UNITS OF MEASURE "--*/
		rv_pr (ML(mlPcMess014), 88, 5, FALSE);
		/*-" BATCH SIZES "-*/
		rv_pr (ML(mlPcMess017), 89, 8, FALSE);

		/*-"Std. %14.6f"-*/
		print_at (9,  69, ML(mlPcMess018));
		print_at (9,  74, "%14.6f", local_rec.stdBatch);

		/*-"Min. %14.6f"-*/
		print_at (9,  90, ML(mlPcMess019)); 
		print_at (9,  95, "%14.6f", local_rec.minBatch);

		/*--"Max. %14.6f"-*/
		print_at (9, 111, ML(mlPcMess020));
		print_at (9, 116, "%14.6f", local_rec.maxBatch);

		scn_write (1);
		if (scn != 1)
			scn_display (1);

		move (0, 21);
		line (132);
		/*-" Co. : %s %s Br. : %s %s Wh. : %s %s",-*/
		print_at (22, 0, ML(mlStdMess038),comm_rec.co_no, comm_rec.co_short);
		print_at (22,50, ML(mlStdMess039),comm_rec.est_no,comm_rec.est_short);
		print_at (22,89, ML(mlStdMess099),comm_rec.cc_no, comm_rec.cc_short);

		/*-" C U S T O M E R S / Q U A N T I T I E S "-*/
		rv_pr (ML(mlPcMess099), 13, 13, TRUE);
		scn_write (2);
		scn_display (2);
		move (0, 13); PGCHAR (10);
		move (0, 21); PGCHAR (2);
		move (69, 13); PGCHAR (8);
		move (69, 21); PGCHAR (9);

		scn_display (3);
		move (70, 13); PGCHAR (8);
		move (131, 13); PGCHAR (11);
		move (70, 21); PGCHAR (9);
		move (131, 21); PGCHAR (3);

		scn_set (scn);
		scn_write (scn);

		/* reset this variable for new screen NOT page	*/
		line_cnt = 0;
	}
	return (EXIT_FAILURE);
}

/*===============================
| Validate entered field (s)	|
===============================*/
int
spec_valid (
 int field)
{
	int		i;
	float	tmp_qty;
	long	mltpl;
	float	qty1,
			qty2;
	float	qtyRequired;
	char	currBrNo [3];
	char	currWhNo [3];

	if (LCHECK ("reqBrNo") ||
		LCHECK ("mfgBrNo") ||
		LCHECK ("recBrNo"))
	{
		if (SRCH_KEY)
		{
			SrchEsmr (temp_str);
			return (EXIT_SUCCESS);
		}

		if (LCHECK ("reqBrNo"))
			strcpy (currBrNo, local_rec.reqBrNo);
		else
		if (LCHECK ("mfgBrNo"))
			strcpy (currBrNo, local_rec.mfgBrNo);
		else
		if (LCHECK ("recBrNo"))
			strcpy (currBrNo, local_rec.recBrNo);

		strcpy (esmr_rec.co_no, comm_rec.co_no);
		strcpy (esmr_rec.est_no, currBrNo);
		cc = find_rec (esmr, &esmr_rec, EQUAL, "r");
		if (cc)
		{
			/*-------------------
			| Branch Not found. |
			-------------------*/
			print_mess (ML(mlStdMess073));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (LCHECK ("reqBrNo"))
		{
			strcpy (local_rec.reqBrName, esmr_rec.short_name);
			DSP_FLD ("reqBrName");
		}
		else
		if (LCHECK ("mfgBrNo"))
		{
			strcpy (local_rec.mfgBrName, esmr_rec.short_name);
			DSP_FLD ("mfgBrName");
		}
		else
		if (LCHECK ("recBrNo"))
		{
			strcpy (local_rec.recBrName, esmr_rec.short_name);
			DSP_FLD ("recBrName");
		}

		if (prog_status != ENTRY && 
			!LCHECK ("mfgBrNo"))
		{
			/* prompt user for warehouse, even in edit mode */
			do
			{
				if (LCHECK ("reqBrNo"))
				{
					get_entry (label ("reqWhNo"));
					cc = spec_valid (label ("reqWhNo"));
				}
				else
				{
					get_entry (label ("recWhNo"));
					cc = spec_valid (label ("recWhNo"));
				}
			} while (cc && !restart);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("reqWhNo") ||
		LCHECK ("mfgWhNo") ||
		LCHECK ("recWhNo"))
	{
		if (LCHECK ("reqWhNo"))
		{
			strcpy (currBrNo, local_rec.reqBrNo);
			strcpy (currWhNo, local_rec.reqWhNo);
		}
		else
		if (LCHECK ("mfgWhNo"))
		{
			strcpy (currBrNo, local_rec.mfgBrNo);
			strcpy (currWhNo, local_rec.mfgWhNo);
		}
		else
		if (LCHECK ("recWhNo"))
		{
			strcpy (currBrNo, local_rec.recBrNo);
			strcpy (currWhNo, local_rec.recWhNo);
		}

		if (SRCH_KEY)
		{
			SrchCcmr (temp_str, currBrNo);
			return (EXIT_SUCCESS);
		}

		strcpy (ccmr_rec.co_no, comm_rec.co_no);
		strcpy (ccmr_rec.est_no, currBrNo);
		strcpy (ccmr_rec.cc_no, currWhNo);
		cc = find_rec (ccmr, &ccmr_rec, EQUAL, "r");
		if (cc)
		{
			/*---------------------
			| Warehouse not found.|
			----------------------*/
			print_mess (ML(mlStdMess100));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (LCHECK ("reqWhNo"))
		{
			strcpy (local_rec.reqWhName, ccmr_rec.acronym);
			DSP_FLD ("reqWhNo");
			DSP_FLD ("reqWhName");
		}
		else
		if (LCHECK ("mfgWhNo"))
		{
			strcpy (local_rec.mfgWhName, ccmr_rec.acronym);
			DSP_FLD ("mfgWhNo");
			DSP_FLD ("mfgWhName");
			currentHhccHash = ccmr_rec.hhcc_hash;
		}
		else
		if (LCHECK ("recWhNo"))
		{
			strcpy (local_rec.recWhName, ccmr_rec.acronym);
			DSP_FLD ("recWhNo");
			DSP_FLD ("recWhName");
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("orderNumber"))
	{
		if (F_NOKEY (field))
			strcpy (local_rec.orderNumber, "0000000");

		if (SRCH_KEY)
		{
			if (worksOrderReleased)
				SearchOrder 
				(
					temp_str,
					"R",
					local_rec.mfgBrNo,
					local_rec.mfgWhNo
				);
			else
				SearchOrder 
				(
					temp_str,
					"PFIARC",
					local_rec.mfgBrNo,
					local_rec.mfgWhNo
				);
			return (EXIT_SUCCESS);
		}

		if (dflt_used ||
			!strncmp (local_rec.orderNumber, "       ", 7))
		{
			if (!envVarPcGenNum)
			{
				FLD ("batchNumber") = YES;
				newOrder = FALSE;
			}
			else
			{
				strcpy (local_rec.orderNumber, "0000000");
				DSP_FLD ("orderNumber");
				FLD ("batchNumber") = NE;
				newOrder = TRUE;
			}

			return (EXIT_SUCCESS);
		}

		strcpy (pcwo_rec.co_no, comm_rec.co_no);
		strcpy (pcwo_rec.br_no, local_rec.mfgBrNo);
		strcpy (pcwo_rec.wh_no, local_rec.mfgWhNo);
		if (envVarPcGenNum)
			strcpy (pcwo_rec.order_no, zero_pad (local_rec.orderNumber, 7));
		else
			strcpy (pcwo_rec.order_no, local_rec.orderNumber);
		cc = find_rec (pcwo, &pcwo_rec, EQUAL, "u");
		if (cc)
		{
			if (worksOrderReleased)
			{
				/*---------------------------------
				|  Released Works Order not found.|
				---------------------------------*/
				print_mess (ML(mlPcMess090));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			if (envVarPcGenNum)
			{
				/*--------------------------------
				| Works Order Does Not found. 	 |
				| Number Generated By The System |
				--------------------------------*/
				print_mess (ML(mlPcMess094));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			abc_unlock (pcwo);
			newOrder = TRUE;
			batchFlag = FALSE;
			FLD ("itemNumber") = YES;
			FLD ("bomAlternate") = NO;
			FLD ("rtgAlternate") = NO;
			FLD ("batchNumber") = NE;

			strcpy (pcwo_rec.order_status, "P");
			strcpy (local_rec.status, "STATUS: Planned     ");
			rv_pr (ML(mlPcMess100), 59, 3, FALSE);
			local_rec.createDate = local_rec.lsystemDate;
			DSP_FLD ("createDate");

			local_rec.requiredDate = local_rec.lsystemDate;
			DSP_FLD ("requiredDate");

			local_rec.bomAlternate_old = -1;
			local_rec.rtgAlternate_old = -1;

			return (EXIT_SUCCESS);
		}
		if (worksOrderReleased && pcwo_rec.order_status [0] != 'R')
		{
			/*---------------------------
			| Works Order Not Released. |
			---------------------------*/
			print_mess (ML(mlPcMess091));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (pcwo_rec.order_status [0] == 'P')
			FLD ("batchNumber") = NO;
		else
			FLD ("batchNumber") = NE;

		return (DisplayDetails (FALSE));
	}

	if (LCHECK ("batchNumber"))
	{
		if (SRCH_KEY)
		{
			if (worksOrderReleased)
				SearchBatch 
				(
					temp_str,
					"R",
					local_rec.mfgBrNo,
					local_rec.mfgWhNo
				);
			else
				SearchBatch 
				(
					temp_str,
					"PFIARC",
					local_rec.mfgBrNo,
					local_rec.mfgWhNo
				);
			abc_selfield (pcwo, "pcwo_id_no");
			return (EXIT_SUCCESS);
		}

		if (dflt_used || !strncmp (local_rec.batchNumber, "          ", 10))
		{
			if (worksOrderReleased)
			{
				/*---------------------------------
				| Batch Number should be entered. |
				---------------------------------*/
				print_mess (ML(mlPcMess095));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			if (envVarPcGenNum && prog_status == ENTRY)
			{
				newOrder = TRUE;
				batchFlag = FALSE;
				FLD ("itemNumber") = YES;
				FLD ("bomAlternate") = NO;
				FLD ("rtgAlternate") = NO;
				FLD ("batchNumber") = NE;

				strcpy (pcwo_rec.order_status, "P");
				strcpy (local_rec.status, "STATUS: Planned     ");
				rv_pr (ML(mlPcMess100), 59, 3, FALSE);

				local_rec.createDate = local_rec.lsystemDate;
				DSP_FLD ("createDate");
				local_rec.requiredDate = local_rec.lsystemDate;
				DSP_FLD ("requiredDate");

				return (EXIT_SUCCESS);
			}
			if (newOrder)
				return (EXIT_SUCCESS);
			if (!envVarPcGenNum && prog_status == ENTRY)
				return (EXIT_FAILURE);

			batchFlag = FALSE;
			strcpy (pcwo_rec.co_no, comm_rec.co_no);	
			strcpy (pcwo_rec.br_no, local_rec.mfgBrNo);
			strcpy (pcwo_rec.wh_no, local_rec.mfgWhNo);
			strcpy (pcwo_rec.order_no, local_rec.orderNumber);	
			cc = find_rec (pcwo, &pcwo_rec, EQUAL, "u");
			if (cc)
				file_err (cc, pcwo, "DBFIND");

			strcpy (local_rec.batchNumber, pcwo_rec.batch_no);
			DSP_FLD ("batchNumber");
			return (EXIT_SUCCESS);
		}

		strcpy (pcwo_rec.co_no, comm_rec.co_no);
		strcpy (pcwo_rec.br_no, local_rec.mfgBrNo);
		strcpy (pcwo_rec.wh_no, local_rec.mfgWhNo);
		strcpy (pcwo_rec.batch_no, local_rec.batchNumber);
		cc = find_rec (pcwo2, &pcwo_rec, EQUAL, "u");
		if (cc)
		{
			if (worksOrderReleased)
			{
				/*---------------------------------
				| Released Works Order not found. |
				---------------------------------*/
				print_mess (ML(mlPcMess090));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			if (envVarPcGenNum)
			{
				if (prog_status == ENTRY)
				{
					strcpy (pcwo_rec.batch_no, local_rec.batchNumber);
					newOrder = TRUE;
					batchFlag = FALSE;
					FLD ("itemNumber") = YES;
					FLD ("bomAlternate") = NO;
					FLD ("rtgAlternate") = NO;
					FLD ("batchNumber") = NE;

					strcpy (pcwo_rec.order_status, "P");
					strcpy (local_rec.status, "STATUS: Planned     ");
					rv_pr (ML(mlPcMess100), 59, 3, FALSE);

					local_rec.createDate = local_rec.lsystemDate;
					DSP_FLD ("createDate");
					local_rec.requiredDate = local_rec.lsystemDate;
					DSP_FLD ("requiredDate");
					return (EXIT_SUCCESS);
				}
				else /* prog_status != ENTRY */
				{
					batchFlag = FALSE;
					strcpy (pcwo_rec.co_no, comm_rec.co_no);
					strcpy (pcwo_rec.br_no, local_rec.mfgBrNo);
					strcpy (pcwo_rec.wh_no, local_rec.mfgWhNo);
					strcpy (pcwo_rec.order_no, local_rec.orderNumber);
					if ((cc = find_rec (pcwo, &pcwo_rec, EQUAL, "u")))
						file_err (cc, pcwo, "DBFIND");
				}
			}
			else
			{
				if (!newOrder &&
					prog_status == ENTRY)
				{
					/*-------------------------------------------
					| Work Order Does Not Exist - Batch No [%s] |
					-------------------------------------------*/
					print_mess (ML(mlPcMess067));
					sleep (sleepTime);
					clear_mess ();
					sprintf (local_rec.batchNumber, "%-10.10s", " ");
					return (EXIT_FAILURE);
				}
			}
		}
		if ((!envVarPcGenNum && newOrder) ||
			(!newOrder && prog_status != ENTRY))
		{
			if (!strcmp (pcwo_rec.order_no, local_rec.orderNumber))
				return (EXIT_SUCCESS);
			if (strcmp (pcwo_rec.batch_no, "          ") > 0 ||
				strcmp (pcwo_rec.batch_no, "          ") < 0)
			{
				/*------------------------------------------
				| Work Order Already Exist - Batch No [%s] |
				------------------------------------------*/
				print_mess (ML(mlPcMess119));
				sleep (sleepTime);
				clear_mess ();
				sprintf (local_rec.batchNumber, "%-10.10s", " ");
				return (EXIT_FAILURE);
			}
		}
		if (newOrder)
			strcpy (pcwo_rec.batch_no, local_rec.batchNumber);
		if (worksOrderReleased && pcwo_rec.order_status [0] != 'R')
		{
			/*---------------------------
			| Works Order Not Released. |
			---------------------------*/
			print_mess (ML(mlPcMess091));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		return (DisplayDetails (TRUE));
	}

	if (LCHECK ("itemNumber"))
	{
		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}

		abc_selfield (inmr, "inmr_id_no");


		cc = FindInmr (comm_rec.co_no, local_rec.itemNumber, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.itemNumber);
			cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			/*-----------------
			| Item not found. |
			-----------------*/
			print_mess (ML(mlStdMess001));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();

		if (strcmp (inmr_rec.source, "BP") &&
			strcmp (inmr_rec.source, "BM") &&
			strcmp (inmr_rec.source, "MC") &&
			strcmp (inmr_rec.source, "MP"))
		{
			/*---------------------------------
			| Item should be BP, BM, MC or MP |
			---------------------------------*/
			print_mess (ML(mlPcMess006));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		incc_rec.hhcc_hash = currentHhccHash;
		incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
		if (find_rec (incc, &incc_rec, EQUAL, "r"))
		{
			/*-------------------------------------------
			| Item is Not at this Warehouse - Create ? |
			-------------------------------------------*/
			i = prmptmsg (ML(mlStdMess033), "YyNn", 1, 20);
			move (1, 20);
			cl_line ();
			if (i == 'n' || i == 'N')
			{
				skip_entry = -1;
				return (EXIT_FAILURE);
			}
			else
			{
				if ((cc = AddIncc (currentHhccHash)))
					file_err (cc, incc, "DBFIND");
			}
		}

		cc = find_hash (inum, &inum_rec, EQUAL, "r", inmr_rec.std_uom);
		strcpy (local_rec.standardUOM, (cc) ? "    " : inum_rec.uom);
		cc = find_hash (inum, &inum_rec, EQUAL, "r", inmr_rec.alt_uom);
		strcpy (local_rec.alternateUOM, (cc) ? "    " : inum_rec.uom);

		sprintf (local_rec.itemNumber, "%-16.16s", inmr_rec.item_no);
		DSP_FLD ("itemNumber");
		sprintf (local_rec.desc, "%-35.35s", inmr_rec.description);
		sprintf (local_rec.strength, "%-5.5s", inmr_rec.description + 35);
		sprintf (local_rec.desc2, "%-40.40s", inmr_rec.description2);
		DSP_FLD ("itemNumber");
		DSP_FLD ("strength");
		DSP_FLD ("desc");
		DSP_FLD ("desc2");
		DSP_FLD ("standardUOM");
		DSP_FLD ("alternateUOM");

		strcpy (inei_rec.est_no, comm_rec.est_no);
		inei_rec.hhbr_hash = inmr_rec.hhbr_hash;
		cc = find_rec (inei, &inei_rec, EQUAL, "r");
		if (cc)
		{
			inei_rec.std_batch = 1.00;
			inei_rec.min_batch = 1.00;
			inei_rec.max_batch = 1.00;
			inei_rec.prd_multiple = 0.00;
		}
		if (inei_rec.std_batch == 0.00)
			inei_rec.std_batch = 1.00;
		local_rec.stdBatch	= (double) inei_rec.std_batch;
		local_rec.minBatch	= (double) inei_rec.min_batch;
		local_rec.maxBatch	= (double) inei_rec.max_batch;
		local_rec.prdMult	= inei_rec.prd_multiple;
		sprintf (local_rec.stdBatchString, "%14.6f", inei_rec.std_batch);

		/*-Std. %14.6f"-*/
		print_at (9,  69, ML(mlPcMess018)); 
		print_at (9,  74, "%14.6f", inei_rec.std_batch);

		/*--"Min. %14.6f"-*/
		print_at (9,  90, ML(mlPcMess019));
		print_at (9,  95, "%14.6f", inei_rec.min_batch);

		/*-"Max. %14.6f"-*/
		print_at (9, 111, ML(mlPcMess020));
		print_at (9, 116, "%14.6f", inei_rec.max_batch);

		local_rec.hhbrHash = inmr_rec.hhbr_hash;

		if (prog_status != ENTRY)
		{
			cc = 0;
			do {
				get_entry (label ("bomAlternate"));
				cc = spec_valid (label ("bomAlternate"));
				DSP_FLD ("bomAlternate");
			} while (cc && !restart);				/* get BOM number */
			if (restart)
				return (EXIT_SUCCESS);
			do {
				get_entry (label ("rtgAlternate"));
				cc = spec_valid (label ("rtgAlternate"));
				DSP_FLD ("rtgAlternate");
			} while (cc && !restart);				/* get routing number */
			if (restart)
				return (EXIT_SUCCESS);
			do {
				get_entry (label ("qtyRequired"));
				cc = spec_valid (label ("qtyRequired"));
				DSP_FLD ("qtyRequired");
			} while (cc && !restart);				/* update qty required */
			if (restart)
				return (EXIT_SUCCESS);
			do {
				get_entry (label ("requiredDate"));
				cc = spec_valid (label ("requiredDate"));
				DSP_FLD ("requiredDate");
			} while (cc && !restart);				/* update required date */
			if (restart)
				return (EXIT_SUCCESS);
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("bomAlternate"))
	{
		if (SRCH_KEY)
		{
			SrchAltBom ();
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			/* read warehouse record for default bom no */
			incc_rec.hhcc_hash = currentHhccHash;
			incc_rec.hhbr_hash = local_rec.hhbrHash;
			cc = find_rec (incc, &incc_rec, EQUAL, "r");
			/* if warehouse default is 0, read branch for default bom no */
			if (cc || incc_rec.dflt_bom <= 0)
			{
				strcpy (inei_rec.est_no, local_rec.mfgBrNo);
				inei_rec.hhbr_hash = local_rec.hhbrHash;
				cc = find_rec (inei, &inei_rec, EQUAL, "r");
				/* if branch default is 0, read company for default bom no */
				if (cc || inei_rec.dflt_bom <= 0)
				{
					cc = find_hash (inmr2, &inmr_rec, EQUAL,
							"r", local_rec.hhbrHash);
					if (cc || inmr_rec.dflt_bom <= 0)
					{
						/*-------------------
						| No Default Setup. |
						-------------------*/
						print_mess (ML(mlStdMess007)); 
						sleep (sleepTime);
						clear_mess ();
						return (EXIT_FAILURE);
					}
					local_rec.bomAlternate = inmr_rec.dflt_bom;
				}
				else
					local_rec.bomAlternate = inei_rec.dflt_bom;
			}
			else
				local_rec.bomAlternate = incc_rec.dflt_bom;

			DSP_FLD ("bomAlternate");
		}

		strcpy (bmms_rec.co_no, comm_rec.co_no);
		bmms_rec.hhbr_hash = local_rec.hhbrHash;
		bmms_rec.alt_no = local_rec.bomAlternate;
		bmms_rec.line_no = 0;
		cc = find_rec (bmms, &bmms_rec, GTEQ, "r");
		if (cc || strcmp (bmms_rec.co_no, comm_rec.co_no) ||
			bmms_rec.hhbr_hash != local_rec.hhbrHash ||
			bmms_rec.alt_no != local_rec.bomAlternate)
		{
			/*---------------------------
			| Alternate Item not found. | 
			---------------------------*/
			print_mess (ML(mlStdMess002));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		newBomAlt = (local_rec.bomAlternate == local_rec.bomAlternate_old) ? FALSE : TRUE;

		/*--------------------------------
		| Check routing alternate number |
		--------------------------------*/
		strcpy (rghr_rec.co_no, comm_rec.co_no);
		strcpy (rghr_rec.br_no, comm_rec.est_no);
		rghr_rec.hhbr_hash = local_rec.hhbrHash;
		rghr_rec.alt_no = local_rec.bomAlternate;
		cc = find_rec (rghr, &rghr_rec, COMPARISON, "r");
		if (!cc &&
			!strcmp (rghr_rec.co_no, comm_rec.co_no) &&
			!strcmp (rghr_rec.br_no, comm_rec.est_no) &&
			rghr_rec.hhbr_hash == local_rec.hhbrHash &&
			rghr_rec.alt_no == local_rec.bomAlternate)
		{
			local_rec.rtgAlternate = local_rec.bomAlternate;
			if (prog_status == ENTRY)
				FLD ("rtgAlternate") = NA;
			else
				FLD ("rtgAlternate") = NO;
			DSP_FLD ("rtgAlternate");
		}
		else
		{
			if (local_rec.status [0] == 'P')
				FLD ("rtgAlternate") = NO;
			else
				FLD ("rtgAlternate") = NE;
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("rtgAlternate"))
	{
		if (SRCH_KEY)
		{
			SrchAltRtg ();
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			/* read warehouse record for default rtg no */
			incc_rec.hhcc_hash = currentHhccHash;
			incc_rec.hhbr_hash = local_rec.hhbrHash;
			cc = find_rec (incc, &incc_rec, EQUAL, "r");
			/* if warehouse default is 0, read branch for default rtg no */
			if (cc || incc_rec.dflt_rtg <= 0)
			{
				strcpy (inei_rec.est_no, local_rec.mfgBrNo);
				inei_rec.hhbr_hash = local_rec.hhbrHash;
				cc = find_rec (inei, &inei_rec, EQUAL, "r");
				/* if branch default is 0, read company for default rtg no */
				if (cc || inei_rec.dflt_rtg <= 0)
				{
					cc = find_hash (inmr2, &inmr_rec, EQUAL,
							"r", local_rec.hhbrHash);
					if (cc || inmr_rec.dflt_rtg <= 0)
					{
						/*-------------------
						| No Default Setup. |
						-------------------*/
						print_mess (ML(mlStdMess007)); 
						sleep (sleepTime);
						clear_mess ();
						return (EXIT_FAILURE);
					}
					local_rec.rtgAlternate = inmr_rec.dflt_rtg;
				}
				else
					local_rec.rtgAlternate = inei_rec.dflt_rtg;
			}
			else
				local_rec.rtgAlternate = incc_rec.dflt_rtg;

			DSP_FLD ("rtgAlternate");
		}

		strcpy (rghr_rec.co_no, comm_rec.co_no);
		strcpy (rghr_rec.br_no, local_rec.mfgBrNo);
		rghr_rec.hhbr_hash = local_rec.hhbrHash;
		rghr_rec.alt_no = local_rec.rtgAlternate;
		cc = find_rec (rghr, &rghr_rec, EQUAL, "r");
		if (cc ||
			strcmp (rghr_rec.co_no, comm_rec.co_no) ||
			strcmp (rghr_rec.br_no, local_rec.mfgBrNo) ||
			rghr_rec.hhbr_hash != local_rec.hhbrHash ||
			rghr_rec.alt_no != local_rec.rtgAlternate)
		{
			/*---------------------------
			| Alternate item not found. |
			---------------------------*/
			print_mess (ML(mlStdMess002));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		newRtgAlt = (local_rec.rtgAlternate == local_rec.rtgAlternate_old) ? FALSE : TRUE;
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("qtyRequired"))
	{
		local_rec.qtyRequired = NDEC (local_rec.qtyRequired);
		DSP_FLD ("qtyRequired");
		if (!ValidQuantity (local_rec.qtyRequired, inmr_rec.dec_pt))
			return (EXIT_FAILURE);

		if (local_rec.qtyRequired <= 0.00)
		{
			/* -------------------
			| Illegal Quantity!! |
			---------------------*/
			print_mess (ML(mlStdMess190));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (local_rec.prdMult != 0.00)
		{
			tmp_qty = (float) (local_rec.qtyRequired - local_rec.stdBatch);
			mltpl = (long)(tmp_qty / local_rec.prdMult);
			tmp_qty -= (mltpl * local_rec.prdMult);
			if (tmp_qty > 0L)
				local_rec.qtyRequired = (double) (local_rec.stdBatch +
						local_rec.prdMult +
						mltpl * local_rec.prdMult);
			if (tmp_qty < 0L)
				local_rec.qtyRequired = (double) (local_rec.stdBatch +
						mltpl * local_rec.prdMult);
			DSP_FLD ("qtyRequired");
		}

		qtyRequired = (float) local_rec.qtyRequired;
		if (qtyRequired < local_rec.minBatch)
		{
			/*---------------------------------------------
			| Quantity is less than minimum batch size!! |
			---------------------------------------------*/
			print_mess (ML(mlPcMess107)); 
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (qtyRequired > (float) local_rec.maxBatch && local_rec.maxBatch != 0.00)
		{
			/*-----------------------------------------------
			| Quantity is greater than maximum batch size!! |
			-----------------------------------------------*/
			print_mess (ML(mlPcMess108)); 
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		tmp_qty = 0.00;
		for (i = 0; i < lcount [2]; i++)
			tmp_qty += store [i].customerQty;
		if (tmp_qty > qtyRequired)
		{
			/*------------------------------------
			| Batch size cannot be lowered below |
			| the allocated quantity! 			 |
			------------------------------------*/
			print_mess (ML(mlPcMess092));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (newOrder)
		{
			strcpy (bmms_rec.co_no, comm_rec.co_no);
			bmms_rec.hhbr_hash = inmr_rec.hhbr_hash;
			bmms_rec.alt_no = 0;
			bmms_rec.line_no = 0;
			cc = find_rec (bmms, &bmms_rec, GTEQ, "r");
			while (!cc &&
				!strcmp (bmms_rec.co_no, comm_rec.co_no) &&
				bmms_rec.hhbr_hash == inmr_rec.hhbr_hash)
			{
				cc = find_hash (inmr2, &inmr2_rec, COMPARISON,
						"r", bmms_rec.mabr_hash);
				if (cc)
					file_err (cc, inmr2, "DBFIND");

				if (inmr2_rec.serial_item [0] == 'Y')
				{
					/*-----------------------------------------------
					| Calculate quantity required for the serial    |
					| item and round to no_dec, compare totals      |
					| if not the same, quantity required is not a   |
					| whole number therefore item cannot be issued. |
					| Print error message to screen and allow user  |
					| to enter quantity again.                      |
					-----------------------------------------------*/
					qty1 = bmms_rec.matl_qty;
					bmms_rec.matl_wst_pc += 100;
					bmms_rec.matl_wst_pc /= 100;
					qty1 *= bmms_rec.matl_wst_pc;

					qty1 *= (float) NDEC (local_rec.qtyRequired);
					qty1 /= (float) NDEC (local_rec.stdBatch);

					qty2 =(float) no_dec (qty1);

					if (qty1 != qty2)
					{
						/*------------------------------------------------
						| Cannot issue the Total Quantity Required [%f]. |
						| BOM Specification has a Serial Item [%s]. 	 |
						-----------------------------------------------*/
						sprintf (err_str, ML(mlPcMess093),
													qty1,
													inmr2_rec.item_no);
						print_mess (err_str);
						sleep (sleepTime);
						clear_mess ();
						return (EXIT_FAILURE);
					}
				}

				cc = find_rec (bmms, &bmms_rec, NEXT, "r");
			}
		}
		else
		{
			pcms_rec.hhwo_hash = pcwo_rec.hhwo_hash;
			pcms_rec.uniq_id = 0;
			cc = find_rec (pcms, &pcms_rec, GTEQ, "r");
			while (!cc &&
				pcms_rec.hhwo_hash == pcwo_rec.hhwo_hash)
			{
				cc = find_hash (inmr2, &inmr2_rec, COMPARISON,
						"r", pcms_rec.mabr_hash);
				if (cc)
					file_err (cc, inmr, "DBFIND");

				if (inmr2_rec.serial_item [0] == 'Y')
				{
					/*-----------------------------------------------
					| Calculate quantity required for the serial    |
					| item and round to no_dec, compare totals      |
					| if not the same, quantity required is not a   |
					| whole number therefore item cannot be issued. |
					| Print error message to screen and allow user  |
					| to enter quantity again.                      |
					-----------------------------------------------*/
					qty1 = pcms_rec.matl_qty;
					pcms_rec.matl_wst_pc += 100;
					pcms_rec.matl_wst_pc /= 100;
					qty1 *= pcms_rec.matl_wst_pc;

					qty1 *=(float) NDEC (local_rec.qtyRequired);
					qty1 /=(float) NDEC (pcwo_rec.prod_qty);

					qty2 = (float)no_dec (qty1);

					if (qty1 != qty2)
					{
						/*------------------------------------------------
						| Cannot issued the Total Quantity Required [%f] |
						| BOM Specification has a Serial Item [%s].		 | 
						------------------------------------------------*/
						sprintf (err_str, ML(mlPcMess093),
											qty1,
											inmr2_rec.item_no);
						print_mess (err_str);
						sleep (sleepTime);
						clear_mess ();
						return (EXIT_FAILURE);
					}
				}

				cc = find_rec (pcms, &pcms_rec, NEXT, "r");
			}
		}

		clear_mess ();
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("customerNumber"))
	{
		if (last_char == DELLINE || dflt_used)
			return (DeleteLine ());

		if (SRCH_KEY)
		{
			CumrSearch 
			(
				comm_rec.co_no,
				(envVarDbCo) ? local_rec.mfgBrNo : " 0",
				temp_str
			);
			abc_selfield (cumr, "cumr_id_no");
			return (EXIT_SUCCESS);
		}

		clear_mess ();

		strcpy (cumr_rec.co_no, comm_rec.co_no);
		strcpy (cumr_rec.est_no, (envVarDbCo) ? local_rec.mfgBrNo:" 0");
		strcpy (cumr_rec.dbt_no, pad_num (clip (local_rec.customerNumber)));
		cc = find_rec (cumr, &cumr_rec, EQUAL, "r");
		if (cc)
		{
			/*---------------------
			| Customer NOT found. |
			---------------------*/
			print_mess (ML(mlStdMess021));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		for (i = 0; i < lcount [2]; i++)
		{
			if (i != line_cnt && store [i].hhcuHash == cumr_rec.hhcu_hash)
			{
				/*------------------------------
				| Customer is already allocated. |
				------------------------------*/
				print_mess (ML(mlPcMess120));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}
		store [line_cnt].hhcuHash = cumr_rec.hhcu_hash;
		strcpy (local_rec.customerNumber, cumr_rec.dbt_no);
		strcpy (local_rec.customerName, cumr_rec.dbt_name);
		DSP_FLD ("customerNumber");
		DSP_FLD ("customerName");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("customerQty"))
	{
		if (local_rec.customerQty < 0.00)
		{
			/*--------------------
			| Illegal quantity!! |
			--------------------*/
			print_mess (ML(mlStdMess190));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		tmp_qty = 0.00;
		for (i = 0; i < lcount [2]; i++)
		{
			if (i != line_cnt)
				tmp_qty += store [i].customerQty;
		}
		tmp_qty += local_rec.customerQty;
		qtyRequired = (float) local_rec.qtyRequired;
		if (tmp_qty > qtyRequired)
		{
			/*---------------------------------
			| Allocated is more than produced |
			---------------------------------*/
			print_mess (ML(mlPcMess109));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		store [line_cnt].customerQty = local_rec.customerQty;
	}

	if (LCHECK ("requiredDate"))
	{
		if (local_rec.requiredDate < local_rec.lsystemDate)
		{
			/*---------------------------------
			| This is required BEFORE today!! |
			---------------------------------*/
			print_mess (ML(mlPcMess110)); 
			sleep (sleepTime);
			clear_mess ();
		}
		else
			clear_mess ();
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

int
AddIncc (
	long	hhccHash)
{
	incc_rec.hhcc_hash = hhccHash;
	incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
	strcpy (incc_rec.ff_option, "A");
	strcpy (incc_rec.allow_repl, "E");
	strcpy (incc_rec.abc_code, "A");
	strcpy (incc_rec.abc_update, "Y");

	cc = abc_add (incc, &incc_rec);

	return (cc);
}

void
SrchEsmr (
	char	*keyValue)
{
	work_open ();
	save_rec ("#Branch No", "#Branch Description");

	strcpy (esmr_rec.co_no, comm_rec.co_no);
	sprintf (esmr_rec.est_no, "%-2.2s", keyValue);
	cc = find_rec (esmr, &esmr_rec, GTEQ, "r");
	while (!cc &&
		!strcmp (esmr_rec.co_no, comm_rec.co_no) &&
		!strncmp (esmr_rec.est_no, keyValue, strlen (keyValue)))
	{
		sprintf (err_str,
				"(%-15.15s) %-40.40s",
				esmr_rec.short_name,
				esmr_rec.est_name);
		save_rec (esmr_rec.est_no, err_str);

	    cc = find_rec (esmr, &esmr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
	    return;

	strcpy (esmr_rec.co_no, comm_rec.co_no);
	sprintf (esmr_rec.est_no, "%-2.2s", temp_str);
	cc = find_rec (esmr, &esmr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, esmr, "DBFIND");
}

void
SrchCcmr (
	char	*keyValue, 
	char	*br_no)
{
	work_open ();
	save_rec ("#Branch No", "#Branch Description");

	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, br_no);
	sprintf (ccmr_rec.cc_no, "%-2.2s", keyValue);
	cc = find_rec (ccmr, &ccmr_rec, GTEQ, "r");
	while (!cc &&
		!strcmp (ccmr_rec.co_no, comm_rec.co_no) &&
		!strcmp (ccmr_rec.est_no, br_no) &&
		!strncmp (ccmr_rec.cc_no, keyValue, strlen (keyValue)))
	{
		sprintf (err_str,
				"(%-9.9s) %-40.40s",
				ccmr_rec.acronym,
				ccmr_rec.name);
		save_rec (ccmr_rec.cc_no, err_str);

	    cc = find_rec (ccmr, &ccmr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
	    return;

	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, br_no);
	sprintf (ccmr_rec.cc_no, "%-2.2s", keyValue);
	cc = find_rec (ccmr, &ccmr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");
}

int 
DisplayDetails (
	int		flag)
{
	if (prog_status == ENTRY)
		batchFlag = flag;

	newOrder = FALSE;
	FLD ("orderNumber") = NE;
	FLD ("itemNumber") 	= NE;
	switch (pcwo_rec.order_status [0])
	{
	case	'P':
		strcpy (local_rec.status, "STATUS: Planned     ");
		FLD ("bomAlternate") = NO;
		FLD ("rtgAlternate") = NO;
		rv_pr (ML(mlPcMess100), 59, 3, FALSE);
		break;

	case	'F':
		strcpy (local_rec.status, "STATUS: Firm Planned");
		FLD ("bomAlternate") = NE;
		FLD ("rtgAlternate") = NE;
		rv_pr (ML(mlPcMess114), 59, 3, FALSE);
		break;

	case	'I':
		strcpy (local_rec.status, "STATUS: Issuing     ");
		FLD ("bomAlternate") = NE;
		FLD ("rtgAlternate") = NE;
		rv_pr (ML(mlPcMess115), 59, 3, FALSE);
		break;

	case	'A':
		strcpy (local_rec.status, "STATUS: Allocated   ");
		FLD ("bomAlternate") = NE;
		FLD ("rtgAlternate") = NE;
		rv_pr (ML(mlPcMess116), 59, 3, FALSE);
		break;

	case	'R':
		strcpy (local_rec.status, "STATUS: Released    ");
		if (worksOrderReleased)
		{
			edit_ok (1);
			no_edit (2);
			no_edit (3);
			FLD ("batchNumber")		= NE;
			FLD ("priority")		= NE;
			FLD ("bomAlternate")	= NE;
			FLD ("rtgAlternate")	= NE;
			FLD ("reqBrNo")			= NE;
			FLD ("reqWhNo")			= NE;
			FLD ("recBrNo")			= NE;
			FLD ("recWhNo")			= NE;
		}
		else
		{
			no_edit (1);
			no_edit (3);
		}
		rv_pr (ML(mlPcMess117), 59, 3, FALSE);
		break;

	case	'C':
		strcpy (local_rec.status, "STATUS: Closing     ");
		no_edit (1);
		no_edit (3);
		rv_pr (ML(mlPcMess118), 59, 3, FALSE);
		break;

	case	'Z':
		/*-----------------------
		| Order has been Closed |
		-----------------------*/
		print_mess (ML(mlPcMess111));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);

	default:
		/*------------------------
		| Order has been Deleted |
		------------------------*/
		print_mess (ML(mlPcMess112)); 
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	};


	if (flag)
	{
		strcpy (local_rec.orderNumber, pcwo_rec.order_no);
		DSP_FLD ("orderNumber");
	}
	else
	{
		strcpy (local_rec.batchNumber, pcwo_rec.batch_no);
		DSP_FLD ("batchNumber");
	}

	strcpy (local_rec.reqBrNo, pcwo_rec.req_br_no);
	strcpy (local_rec.reqWhNo, pcwo_rec.req_wh_no);
	strcpy (local_rec.recBrNo, pcwo_rec.rec_br_no);
	strcpy (local_rec.recWhNo, pcwo_rec.rec_wh_no);

	/* find requesting branch name */
	strcpy (esmr_rec.co_no, comm_rec.co_no);
	strcpy (esmr_rec.est_no, local_rec.reqBrNo);
	if ((cc = find_rec (esmr, &esmr_rec, EQUAL, "r")))
		file_err (cc, esmr, "DBFIND");
	strcpy (local_rec.reqBrName, esmr_rec.short_name);
	DSP_FLD ("reqBrNo");
	DSP_FLD ("reqBrName");

	/* find receiving branch name */
	strcpy (esmr_rec.co_no, comm_rec.co_no);
	strcpy (esmr_rec.est_no, local_rec.recBrNo);
	if ((cc = find_rec (esmr, &esmr_rec, EQUAL, "r")))
		file_err (cc, esmr, "DBFIND");
	strcpy (local_rec.recBrName, esmr_rec.short_name);
	DSP_FLD ("recBrNo");
	DSP_FLD ("recBrName");

	/* find requesting warehouse name */
	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, local_rec.reqBrNo);
	strcpy (ccmr_rec.cc_no, local_rec.reqWhNo);
	if ((cc = find_rec (ccmr, &ccmr_rec, EQUAL, "r")))
		file_err (cc, ccmr, "DBFIND");
	strcpy (local_rec.reqWhName, ccmr_rec.acronym);
	DSP_FLD ("reqWhNo");
	DSP_FLD ("reqWhName");

	incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
	cc = find_rec (incc, &incc_rec, COMPARISON, "r");
	if (cc)
	{
		cc = AddIncc (ccmr_rec.hhcc_hash);
		if (cc)
			file_err (cc, incc, "DBADD");
	}

	/* find receiving warehouse name */
	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, local_rec.recBrNo);
	strcpy (ccmr_rec.cc_no, local_rec.recWhNo);
	if ((cc = find_rec (ccmr, &ccmr_rec, EQUAL, "r")))
		file_err (cc, ccmr, "DBFIND");
	strcpy (local_rec.recWhName, ccmr_rec.acronym);
	DSP_FLD ("recWhNo");
	DSP_FLD ("recWhName");

	incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
	cc = find_rec (incc, &incc_rec, COMPARISON, "r");
	if (cc)
	{
		cc = AddIncc (ccmr_rec.hhcc_hash);
		if (cc)
			file_err (cc, incc, "DBADD");
	}

	local_rec.createDate = pcwo_rec.create_date;
	DSP_FLD ("createDate");

	local_rec.priority = pcwo_rec.priority;
	DSP_FLD ("priority");

	abc_selfield (inmr, "inmr_hhbr_hash");
	cc = find_hash (inmr, &inmr_rec, EQUAL, "r", pcwo_rec.hhbr_hash);
	if (cc)
		file_err (cc, inmr, "DBFIND");
	strcpy (local_rec.itemNumber, inmr_rec.item_no);
	sprintf (local_rec.strength, "%-5.5s", inmr_rec.description + 35);
	sprintf (local_rec.desc, "%-35.35s", inmr_rec.description);
	sprintf (local_rec.desc2, "%-40.40s", inmr_rec.description2);
	local_rec.hhbrHash = inmr_rec.hhbr_hash;
	DSP_FLD ("itemNumber");
	DSP_FLD ("strength");
	DSP_FLD ("desc");
	DSP_FLD ("desc2");
	abc_selfield (inmr, "inmr_id_no");

	cc = find_hash (inum,&inum_rec,EQUAL, "r", inmr_rec.std_uom);
	strcpy (local_rec.standardUOM, (cc) ? "    " : inum_rec.uom);
	cc = find_hash (inum,&inum_rec,EQUAL, "r", inmr_rec.alt_uom);
	strcpy (local_rec.alternateUOM, (cc) ? "    " : inum_rec.uom);
	DSP_FLD ("standardUOM");
	DSP_FLD ("alternateUOM");

	strcpy (inei_rec.est_no, local_rec.mfgBrNo);
	inei_rec.hhbr_hash = inmr_rec.hhbr_hash;
	cc = find_rec (inei, &inei_rec, EQUAL, "r");
	if (cc)
	{
		inei_rec.std_batch = 1.00;
		inei_rec.min_batch = 1.00;
		inei_rec.max_batch = 1.00;
		inei_rec.prd_multiple = 0.00;
	}
	if (inei_rec.std_batch == 0.00)
		inei_rec.std_batch = 1.00;
	local_rec.stdBatch	= (double) inei_rec.std_batch;
	local_rec.minBatch	= (double) inei_rec.min_batch;
	local_rec.maxBatch	= (double) inei_rec.max_batch;
	local_rec.prdMult	= inei_rec.prd_multiple;
	sprintf (local_rec.stdBatchString, "%14.6f", inei_rec.std_batch);

	/*-"Std. %14.6f"-*/
	print_at (9,  69, ML(mlPcMess018)); 
	print_at (9,  74, "%14.6f", inei_rec.std_batch);

	/*--"Min. %14.6f"-*/
	print_at (9,  90, ML(mlPcMess019));
	print_at (9,  95, "%14.6f", inei_rec.min_batch);

	/*-"Max. %14.6f"-*/
	print_at (9, 111, ML(mlPcMess020));
	print_at (9, 116, "%14.6f", inei_rec.max_batch);

	local_rec.bomAlternate 		= pcwo_rec.bom_alt;
	local_rec.bomAlternate_old 	= pcwo_rec.bom_alt;
	local_rec.rtgAlternate 		= pcwo_rec.rtg_alt;
	local_rec.rtgAlternate_old 	= pcwo_rec.rtg_alt;
	local_rec.requiredDate 		= pcwo_rec.reqd_date;
	local_rec.qtyRequired 		= NDEC (pcwo_rec.prod_qty);

	DSP_FLD ("rtgAlternate");
	DSP_FLD ("bomAlternate");
	DSP_FLD ("requiredDate");
	DSP_FLD ("qtyRequired");

	LoadCustomerInfo ();

	LoadPcoi ();

	entry_exit = TRUE;
	return (EXIT_SUCCESS);
}

int
DeleteLine (void)
{
	int		i,
			this_page = line_cnt / TABLINES;

	if (prog_status == ENTRY)
	{
		/*------------------------------
		| Cannot Delete Lines on Entry |
		------------------------------*/
		print_mess (ML(mlStdMess005));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}

	lcount [2]--;

	for (i = line_cnt; line_cnt < lcount [2]; line_cnt++)
	{
		getval (line_cnt + 1);
		store [line_cnt].hhcuHash	= store [line_cnt + 1].hhcuHash;
		store [line_cnt].customerQty	= store [line_cnt + 1].customerQty;
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
LoadCustomerInfo (void)
{
	scn_set (2);
	lcount [2] = 0;

	abc_selfield (cumr, "cumr_hhcu_hash");
	pcwl_rec.hhwo_hash = pcwo_rec.hhwo_hash;
	pcwl_rec.hhcu_hash = 0L;
	cc = find_rec (pcwl, &pcwl_rec, GTEQ, "r");
	while (!cc && pcwl_rec.hhwo_hash == pcwo_rec.hhwo_hash)
	{
		cc = find_hash (cumr, &cumr_rec, EQUAL, "r", pcwl_rec.hhcu_hash);
		if (!cc)
		{
			store [lcount [2]].hhcuHash = pcwl_rec.hhcu_hash;
			store [lcount [2]].customerQty = pcwl_rec.cust_qty;
			strcpy (local_rec.customerNumber, cumr_rec.dbt_no);
			strcpy (local_rec.customerName, cumr_rec.dbt_name);
			local_rec.customerQty = pcwl_rec.cust_qty;
			putval (lcount [2]++);
		}
		cc = find_rec (pcwl, &pcwl_rec, NEXT, "r");
	}
	scn_display (2);
	abc_selfield (cumr, "cumr_id_no");

	scn_set (1);
	return;
}

void
LoadPcoi (void)
{
	scn_set (3);
	lcount [3] = 0;

	pcoi_rec.hhwo_hash = pcwo_rec.hhwo_hash;
	pcoi_rec.line_no = 0;
	cc = find_rec (pcoi, &pcoi_rec, GTEQ, "r");
	while (!cc && pcoi_rec.hhwo_hash == pcwo_rec.hhwo_hash)
	{
		strcpy (local_rec.textLine, pcoi_rec.text);
		putval (-1);
		lcount [3]++;
		cc = find_rec (pcoi, &pcoi_rec, NEXT, "r");
	}
	scn_display (3);
	move (70, 13); PGCHAR (8);
	move (131, 13); PGCHAR (11);
	move (70, 21); PGCHAR (9);
	move (131, 21); PGCHAR (3);

	scn_set (1);
	return;
}

/*=======================
| Search for Alternates	|
=======================*/
void
SrchAltBom (void)
{
	char	alt_str [6];
	int		curr_alt;

	work_open ();
	save_rec ("#Alt.", "#");

	curr_alt = 0;
	strcpy (bmms_rec.co_no, comm_rec.co_no);
	bmms_rec.hhbr_hash = local_rec.hhbrHash;
	bmms_rec.alt_no = 1;
	bmms_rec.line_no = 0;
	cc = find_rec (bmms, &bmms_rec, GTEQ, "r");
	while (!cc &&
		!strcmp (bmms_rec.co_no, comm_rec.co_no) &&
		bmms_rec.hhbr_hash == local_rec.hhbrHash)
	{
		sprintf (alt_str, "%5d", bmms_rec.alt_no);
		if (curr_alt == bmms_rec.alt_no)
		{
			cc = find_rec (bmms, &bmms_rec, NEXT, "r");
			continue;
		}

		curr_alt = bmms_rec.alt_no;
		cc = save_rec (alt_str, "");
		if (cc)
			break;
		cc = find_rec (bmms, &bmms_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (bmms_rec.co_no, comm_rec.co_no);
	bmms_rec.hhbr_hash = inmr_rec.hhbr_hash;
	bmms_rec.alt_no = atoi (temp_str);
	local_rec.bomAlternate = bmms_rec.alt_no;
	cc = find_rec (bmms, &bmms_rec, GTEQ, "r");
}

/*=======================
| Search for Alternates	|
=======================*/
void
SrchAltRtg (void)
{
	char	alt_str [6];

	work_open ();
	save_rec ("#Alt.", "#     Time     Cost");

	strcpy (rghr_rec.co_no, comm_rec.co_no);
	strcpy (rghr_rec.br_no, local_rec.mfgBrNo);
	rghr_rec.hhbr_hash = inmr_rec.hhbr_hash;
	rghr_rec.alt_no = 0;
	cc = find_rec (rghr, &rghr_rec, GTEQ, "r");
	while (!cc &&
		!strcmp (rghr_rec.co_no, comm_rec.co_no) &&
		!strcmp (rghr_rec.br_no, local_rec.mfgBrNo) &&
		rghr_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		sprintf (alt_str, "%5d", rghr_rec.alt_no);
		cc = save_rec (alt_str, " ");
		if (cc)
			break;
		cc = find_rec (rghr, &rghr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (rghr_rec.co_no, comm_rec.co_no);
	strcpy (rghr_rec.br_no, local_rec.mfgBrNo);
	rghr_rec.hhbr_hash = inmr_rec.hhbr_hash;
	rghr_rec.alt_no = atoi (temp_str);
	local_rec.rtgAlternate = rghr_rec.alt_no;
	cc = find_rec (rghr, &rghr_rec, GTEQ, "r");
}

void
Update (void)
{
	UpdatePcwo ();

	strcpy (rghr_rec.co_no, comm_rec.co_no);
	strcpy (rghr_rec.br_no, comm_rec.est_no);
	rghr_rec.hhbr_hash = pcwo_rec.hhbr_hash;
	rghr_rec.alt_no = pcwo_rec.rtg_alt;
	cc = find_rec (rghr, &rghr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, rghr, "DBFIND");

	UpdatePcwl ();
	UpdatePcoi ();

	/*-------------------------------
	| Step 1: Update pcms as needed	|
	-------------------------------*/
	UpdatePcms ();

	/*-------------------------------
	| Step 2: Update pcln as needed	|
	-------------------------------*/
	UpdatePcln ();

	/*-------------------------------
	| Step 3: Update pcbp if needed	|
	-------------------------------*/
	UpdatePcbp ();

	abc_unlock (pcwo);
	recalc_sobg ();
}

/*========================================
| Checks if the item has an incc record, |
| if the record is not found the record  |
| is then created.                       |
========================================*/
void
CheckIncc (
	long	hhccHash, 
	long	hhbrHash)
{
	incc_rec.hhcc_hash = hhccHash;
	incc_rec.hhbr_hash = hhbrHash;
	cc = find_rec (incc, &incc_rec, COMPARISON, "r");
	if (cc)
	{
		incc_rec.hhcc_hash = hhccHash;
		incc_rec.hhbr_hash = hhbrHash;
		strcpy (incc_rec.ff_option, "A");
		strcpy (incc_rec.allow_repl, "E");
		strcpy (incc_rec.abc_code, "A");
		strcpy (incc_rec.abc_update, "Y");

		cc = abc_add (incc, &incc_rec);
		if (cc)
			file_err (cc, incc, "DBADD");
	}

	return;
}

void
UpdatePcwo (void)
{
	stdBatchSize = (float) NDEC (local_rec.stdBatch);
	if (newOrder)
	{
		strcpy (pcwo_rec.co_no, comm_rec.co_no);
		strcpy (pcwo_rec.br_no, comm_rec.est_no);
		strcpy (pcwo_rec.wh_no, comm_rec.cc_no);

		if (!envVarPcGenNum)
			strcpy (pcwo_rec.order_no, local_rec.orderNumber);
		else
		{
			/*----------------------------------------------------
			| get the next works order number from the ccmr file |
			----------------------------------------------------*/
			strcpy (ccmr_rec.co_no, comm_rec.co_no);
			strcpy (ccmr_rec.est_no, comm_rec.est_no);
			strcpy (ccmr_rec.cc_no, comm_rec.cc_no);
			cc = find_rec (ccmr, &ccmr_rec, EQUAL, "u");
			if (cc)
				file_err (cc, ccmr, "DBFIND");

			open_rec (pcwo3, pcwo_list, PCWO_NO_FIELDS, "pcwo_id_no");
			while (TRUE)
			{
				if (CheckPcwo (ccmr_rec.nx_wo_num))
					break;
				ccmr_rec.nx_wo_num ++;
			}
			abc_fclose (pcwo3);

			sprintf (pcwo_rec.order_no, "%07ld", ccmr_rec.nx_wo_num);

			ccmr_rec.nx_wo_num ++;
			cc = abc_update (ccmr, &ccmr_rec);
			if (cc)
				file_err (cc, ccmr, "DBUPDATE");
		}

		strcpy (pcwo_rec.req_br_no, local_rec.reqBrNo);
		strcpy (pcwo_rec.req_wh_no, local_rec.reqWhNo);
		strcpy (pcwo_rec.rec_br_no, local_rec.recBrNo);
		strcpy (pcwo_rec.rec_wh_no, local_rec.recWhNo);
		strcpy (pcwo_rec.batch_no, local_rec.batchNumber);
		pcwo_rec.reqd_date 		= local_rec.requiredDate;
		pcwo_rec.rtg_seq 		= 0;
		pcwo_rec.priority 		= local_rec.priority;
		sprintf (pcwo_rec.op_id, "%-14.14s", currentUser);
		strcpy (pcwo_rec.create_time, TimeHHMM());
		pcwo_rec.create_date 	= local_rec.createDate;
		pcwo_rec.mfg_date 		= 0L;
		pcwo_rec.hhbr_hash 		= local_rec.hhbrHash;
		pcwo_rec.bom_alt 		= local_rec.bomAlternate;
		pcwo_rec.rtg_alt 		= local_rec.rtgAlternate;
		pcwo_rec.hhcc_hash 		= currentHhccHash;
		pcwo_rec.prod_qty 		= (float) NDEC (local_rec.qtyRequired);
		pcwo_rec.act_prod_qty 	= (float) 0.00;
		pcwo_rec.act_rej_qty 	= (float) 0.00;
		strcpy (pcwo_rec.order_status, "P");
		strcpy (pcwo_rec.stat_flag, "0");
		cc = abc_add (pcwo, &pcwo_rec);
		if (cc)
			file_err (cc, pcwo, "DBADD");

		cc = find_rec (pcwo, &pcwo_rec, EQUAL, "u");
		if (cc)
			file_err (cc, pcwo, "DBFIND");

		qtyReqCalc = (float) NDEC (local_rec.qtyRequired);

		/*----------------------------------------------
		| Find hhcc_hash for requesting branch and     |
		| warehouse. Add an sobg record for hhcc_hash. |
		----------------------------------------------*/
		strcpy (ccmr_rec.co_no, comm_rec.co_no);
		strcpy (ccmr_rec.est_no, local_rec.reqBrNo);
		strcpy (ccmr_rec.cc_no, local_rec.reqWhNo);
		cc = find_rec (ccmr, &ccmr_rec, EQUAL, "u");
		if (cc)
			file_err (cc, ccmr, "DBFIND");

		/* add incc record if one does not exist for receiving br/wh */
		incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
		incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
		cc = find_rec (incc, &incc_rec, EQUAL, "r");
		if (cc)
		{
			cc = AddIncc (ccmr_rec.hhcc_hash);
			if (cc)
				file_err (cc, incc, "DBADD");
		}

		add_hash 
		(
			pcwo_rec.co_no,
			local_rec.reqBrNo,
			"RC",
			0,
			pcwo_rec.hhbr_hash,
			ccmr_rec.hhcc_hash,
			0L,
			(double) 0.00
		);
	}
	else
	{
		if (strcmp (pcwo_rec.req_br_no, local_rec.reqBrNo) ||
			strcmp (pcwo_rec.req_wh_no, local_rec.reqWhNo))
		{
			/*----------------------------------------------
			| Find hhcc_hash for requesting branch and     |
			| warehouse. Add an sobg record for hhcc_hash. |
			----------------------------------------------*/
			strcpy (ccmr_rec.co_no, comm_rec.co_no);
			strcpy (ccmr_rec.est_no, pcwo_rec.req_br_no);
			strcpy (ccmr_rec.cc_no, pcwo_rec.req_wh_no);
			cc = find_rec (ccmr, &ccmr_rec, EQUAL, "u");
			if (cc)
				file_err (cc, ccmr, "DBFIND");

			add_hash 
			(
				pcwo_rec.co_no,
				pcwo_rec.req_br_no,
				"RC",
				0,
				pcwo_rec.hhbr_hash,
				ccmr_rec.hhcc_hash,
				0L,
				(double) 0.00
			);
		}
		/*----------------------------------------------
		| Find hhcc_hash for requesting branch and     |
		| warehouse. Add an sobg record for hhcc_hash. |
		----------------------------------------------*/
		strcpy (ccmr_rec.co_no, comm_rec.co_no);
		strcpy (ccmr_rec.est_no, local_rec.reqBrNo);
		strcpy (ccmr_rec.cc_no, local_rec.reqWhNo);
		cc = find_rec (ccmr, &ccmr_rec, EQUAL, "u");
		if (cc)
			file_err (cc, ccmr, "DBFIND");

		add_hash 
		(
			pcwo_rec.co_no,
			local_rec.reqBrNo,
			"RC",
			0,
			pcwo_rec.hhbr_hash,
			ccmr_rec.hhcc_hash,
			0L,
			(double) 0.00
		);

		strcpy (pcwo_rec.order_no, 	local_rec.orderNumber);
		strcpy (pcwo_rec.batch_no, 	local_rec.batchNumber);
		strcpy (pcwo_rec.req_br_no, local_rec.reqBrNo);
		strcpy (pcwo_rec.req_wh_no, local_rec.reqWhNo);
		strcpy (pcwo_rec.rec_br_no, local_rec.recBrNo);
		strcpy (pcwo_rec.rec_wh_no, local_rec.recWhNo);
		qtyReqCalc = (float) NDEC (local_rec.qtyRequired);
		pcwo_rec.reqd_date 	= local_rec.requiredDate;
		pcwo_rec.priority 	= local_rec.priority;
		pcwo_rec.hhbr_hash 	= local_rec.hhbrHash;
		pcwo_rec.bom_alt 	= local_rec.bomAlternate;
		pcwo_rec.rtg_alt 	= local_rec.rtgAlternate;
		pcwo_rec.prod_qty 	= (float) NDEC (local_rec.qtyRequired);
		if (batchFlag)
			cc = abc_update (pcwo2, &pcwo_rec);
		else
			cc = abc_update (pcwo, &pcwo_rec);
		if (cc)
			file_err (cc, batchFlag ? pcwo2 : pcwo, "DBUPDATE");
	}

	CheckIncc (pcwo_rec.hhcc_hash, pcwo_rec.hhbr_hash);

	/*-------------------------------
	| Add sobg record to re-calc.	|
	| the primary destination prod.	|
	-------------------------------*/
	add_hash 
	(
		pcwo_rec.co_no,
		pcwo_rec.br_no,
		"RC",
		0,
		pcwo_rec.hhbr_hash,
		pcwo_rec.hhcc_hash,
		0L,
		(double) 0.00
	);
}

int
CheckPcwo (
	long	orderNo)
{
	strcpy (pcwo2_rec.co_no, comm_rec.co_no);
	strcpy (pcwo2_rec.br_no, comm_rec.est_no);
	strcpy (pcwo2_rec.wh_no, comm_rec.cc_no);
	sprintf (pcwo2_rec.order_no, "%07ld", orderNo);
	return (find_rec (pcwo3, &pcwo2_rec, COMPARISON, "r"));
}

void
UpdatePcwl (void)
{
	int		i;

	/*-------------------------------
	| Remove old customer linking.	|
	-------------------------------*/
	pcwl_rec.hhwo_hash	= 	pcwo_rec.hhwo_hash;
	pcwl_rec.hhcu_hash	=	0L;
	cc = find_rec (pcwl, &pcwl_rec, GTEQ, "u");
	while (!cc && pcwl_rec.hhwo_hash == pcwo_rec.hhwo_hash)
	{
		abc_delete (pcwl);
		pcwl_rec.hhwo_hash = pcwo_rec.hhwo_hash;
		pcwl_rec.hhcu_hash = 0L;
		cc = find_rec (pcwl, &pcwl_rec, GTEQ, "u");
	}
	abc_unlock (pcwl);

	/*---------------------------
	| Create customer linking.	|
	---------------------------*/
	for (i = 0; i < lcount [2]; i++)
	{
		pcwl_rec.hhwo_hash 	= pcwo_rec.hhwo_hash;
		pcwl_rec.hhcu_hash 	= store [i].hhcuHash;
		pcwl_rec.cust_qty 	= store [i].customerQty;
		strcpy (pcwl_rec.stat_flag, "0");
		cc = abc_add (pcwl, &pcwl_rec);
		if (cc)
			file_err (cc, pcwl, "DBADD");
	}
}

void
UpdatePcoi (void)
{
	int		i;

	/*-------------------------------
	| Remove Trailing Blank Instr.	|
	-------------------------------*/
	scn_set (3);
	while (lcount [3] > 0)
	{
		getval (lcount [3] - 1);
		if (strlen (clip (local_rec.textLine)) == 0)
			lcount [3]--;
		else
			break;
	}

	/*---------------------------
	| Add/Update Special Instr.	|
	---------------------------*/
	for (i = 0; i < lcount [3]; i++)
	{
		getval (i);
		pcoi_rec.hhwo_hash = pcwo_rec.hhwo_hash;
		pcoi_rec.line_no = i;
		cc = find_rec (pcoi, &pcoi_rec, EQUAL, "r");
		if (cc)
		{
			pcoi_rec.hhwo_hash = pcwo_rec.hhwo_hash;
			pcoi_rec.line_no = i;
			strcpy (pcoi_rec.text, local_rec.textLine);
			cc = abc_add (pcoi, &pcoi_rec);
			if (cc)
				file_err (cc, pcoi, "DBADD");
		}
		else
		{
			strcpy (pcoi_rec.text, local_rec.textLine);
			cc = abc_update (pcoi, &pcoi_rec);
			if (cc)
				file_err (cc, pcoi, "DBUPDATE");
		}
	}
	pcoi_rec.hhwo_hash = pcwo_rec.hhwo_hash;
	pcoi_rec.line_no = lcount [3];
	cc = find_rec (pcoi, &pcoi_rec, GTEQ, "r");
	while (!cc && pcoi_rec.hhwo_hash == pcwo_rec.hhwo_hash)
	{
		cc = abc_delete (pcoi);
		if (cc)
			file_err (cc, pcoi, "DBDELETE");

		pcoi_rec.hhwo_hash 	= pcwo_rec.hhwo_hash;
		pcoi_rec.line_no 	= lcount [3];
		cc = find_rec (pcoi, &pcoi_rec, GTEQ, "r");
	}
}

void
UpdatePcms (
 void)
{
	/*-------------------------------
	| Remove old records from pcms	|
	-------------------------------*/
	pcms_rec.hhwo_hash 	= pcwo_rec.hhwo_hash;
	pcms_rec.uniq_id 	= 0;
	cc = find_rec (pcms, &pcms_rec, GTEQ, "u");
	while (!cc && pcms_rec.hhwo_hash == pcwo_rec.hhwo_hash)
	{
		/*-----------------------------
		| Add sobg record to re-calc. |
		| the material committed qty. |
		-----------------------------*/
		add_hash 
		(
			pcwo_rec.co_no,
			pcwo_rec.br_no,
			"RC",
			0,
			pcms_rec.mabr_hash,
			currentHhccHash,
			0L,
			(double) 0
		);

		cc = abc_delete (pcms);
		if (cc)
			file_err (cc, pcms, "DBDELETE");

		pcms_rec.hhwo_hash 	= pcwo_rec.hhwo_hash;
		pcms_rec.uniq_id 	= 0;
		cc = find_rec (pcms, &pcms_rec, GTEQ, "u");
	}
	abc_unlock (pcms);

	/*-------------------------------
	| Copy new records from bmms	|
	-------------------------------*/
	strcpy (pcms_rec.co_no, pcwo_rec.co_no);
	pcms_rec.hhbr_hash 	= pcwo_rec.hhbr_hash;
	pcms_rec.alt_no 	= local_rec.bomAlternate;
	pcms_rec.line_no 	= 0;
	cc = find_rec (bmms, &pcms_rec, GTEQ, "r");
	while (!cc &&
		!strcmp (pcms_rec.co_no, pcwo_rec.co_no)&&
		pcms_rec.hhbr_hash == pcwo_rec.hhbr_hash&&
		pcms_rec.alt_no == local_rec.bomAlternate)
	{
		strcpy (pcms_rec.act_qty_in, "N");
		pcms_rec.hhwo_hash = pcwo_rec.hhwo_hash;
		pcms_rec.matl_qty *= qtyReqCalc;
		pcms_rec.matl_qty /= stdBatchSize;
		pcms_rec.uniq_id = pcms_rec.line_no;
		cc = abc_add (pcms, &pcms_rec);
		if (cc)
			file_err (cc, pcms, "DBADD");

		/*-----------------------------
		| Add sobg record to re-calc. |
		| the material committed qty. |
		-----------------------------*/
		add_hash 
		(
			pcwo_rec.co_no,
			pcwo_rec.br_no,
			"RC",
			0,
			pcms_rec.mabr_hash,
			currentHhccHash,
			0L,
			(double) 0
		);

		cc = find_rec (bmms, &pcms_rec, NEXT, "r");
	}
}

void
UpdatePcln (
 void)
{
	int		last_seq = -1,
			cur_line = 0;

	/*-------------------------------
	| Remove existing routing lines	|
	-------------------------------*/
	pcln_rec.hhwo_hash 	= pcwo_rec.hhwo_hash;
	pcln_rec.seq_no 	= 0;
	pcln_rec.line_no 	= 0;
	cc = find_rec (pcln, &pcln_rec, GTEQ, "u");
	while (!cc && pcln_rec.hhwo_hash == pcwo_rec.hhwo_hash)
	{
		cc = abc_delete (pcln);
		if (cc)
			file_err (cc, pcln, "DBDELETE");
		pcln_rec.hhwo_hash 	= pcwo_rec.hhwo_hash;
		pcln_rec.seq_no 	= 0;
		pcln_rec.line_no 	= 0;
		cc = find_rec (pcln, &pcln_rec, GTEQ, "u");
	}

	/*---------------------------
	| Copy Master Rtg to pcln.	|
	---------------------------*/
	pcln_rec.hhgr_hash 	= rghr_rec.hhgr_hash;
	pcln_rec.seq_no 	= 0;
	cc = find_rec (rgln, &pcln_rec, GTEQ, "r");
	while (!cc && pcln_rec.hhgr_hash == rghr_rec.hhgr_hash)
	{
		if (pcln_rec.seq_no != last_seq)
		{
			last_seq = pcln_rec.seq_no;
			cur_line = 0;
		}
		else
			cur_line++;
		/*---------------------------------------
		| NB: setup/cleanup is independent of	|
		| the actual quantity produced!!		|
		---------------------------------------*/
		pcln_rec.run *= (long) qtyReqCalc;
		pcln_rec.run /= (long) stdBatchSize;
		strcpy (pcln_rec.act_qty_in, "N");
		pcln_rec.hhwo_hash = pcwo_rec.hhwo_hash;
		pcln_rec.line_no = cur_line;
		pcln_rec.amt_recptd = 0.00;
		cc = abc_add (pcln, &pcln_rec);
		if (cc)
			file_err (cc, pcln, "DBADD");

		cc = find_rec (rgln, &pcln_rec, NEXT, "r");
	}
	abc_unlock (pcln);
}

void
UpdatePcbp (
 void)
{
	/*-------------------------------
	| Remove old pcbp record (s)	|
	-------------------------------*/
	pcbp_rec.hhwo_hash 	= pcwo_rec.hhwo_hash;
	pcbp_rec.seq_no 	= 0;
	pcbp_rec.hhbr_hash 	= 0L;
	cc = find_rec (pcbp, &pcbp_rec, GTEQ, "u");
	while (!cc && pcbp_rec.hhwo_hash == pcwo_rec.hhwo_hash)
	{
		add_hash 
		(
			pcwo_rec.co_no,
			pcwo_rec.br_no,
			"RC",
			0,
			pcbp_rec.hhbr_hash,
			pcwo_rec.hhcc_hash,
			0L,
			(double) 0.00
		);
		cc = abc_delete (pcbp);
		if (cc)
			file_err (cc, pcbp, "DBDELETE");

		pcbp_rec.hhwo_hash 	= pcwo_rec.hhwo_hash;
		pcbp_rec.seq_no 	= 0;
		pcbp_rec.hhbr_hash 	= 0L;
		cc = find_rec (pcbp, &pcbp_rec, GTEQ, "u");
	}
	/*-------------------------------
	| Create new pcbp record (s)	|
	-------------------------------*/
	pcbp_rec.hhgr_hash 	= rghr_rec.hhgr_hash;
	pcbp_rec.seq_no 	= 0;
	pcbp_rec.hhbr_hash 	= 0L;
	cc = find_rec (rgbp, &pcbp_rec, GTEQ, "r");
	while (!cc && pcbp_rec.hhgr_hash == rghr_rec.hhgr_hash)
	{
		strcpy (pcbp_rec.act_qty_in, "N");
		pcbp_rec.hhwo_hash = pcwo_rec.hhwo_hash;
		pcbp_rec.qty *= qtyReqCalc;
		pcbp_rec.qty /= stdBatchSize;
		cc = abc_add (pcbp, &pcbp_rec);
		if (cc)
			file_err (cc, pcbp, "DBADD");

		add_hash
		(
			pcwo_rec.co_no,
			pcwo_rec.br_no,
			"RC",
			0,
			pcbp_rec.hhbr_hash,
			pcwo_rec.hhcc_hash,
			0L,
			(double) 0.00
		);
		cc = find_rec (rgbp, &pcbp_rec, NEXT, "r");
	}
}

/*--------------------------------------------
| Checks if the quantity entered by the user |
| valid quantity that can be saved to a      |
| float variable without any problems of     |
| losing figures after the decimal point.    |
| eg. if _dec_pt is 2 then the greatest      |
| quantity the user can enter is 99999.99    |
--------------------------------------------*/
int
ValidQuantity (
	double _qty, 
	int    _dec_pt)
{
	/*--------------------------------
	| Quantities to be compared with |
	| with the user has entered.     |
	--------------------------------*/
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
		/*--------------------------------------------------
		| Quantity %14.6f Greater Than Allowed Quantity %f |
		--------------------------------------------------*/
		sprintf (err_str, ML(mlPcMess083), _qty, compare [_dec_pt]);
		print_mess (err_str);
		sleep (sleepTime);
		clear_mess ();
		return (FALSE);
	}

	return (TRUE);
}

int
PrintRequistion (void)
{
	double	extend	=	0.00;

	print_mess (ML ("Printing Works Order Requistion "));

	HeadOutput ();

	/*-----------------------
	| read for item details |
	-----------------------*/
	cc = find_hash (inmr2, &inmr_rec, COMPARISON, "r", pcwo_rec.hhbr_hash);
	if (cc)
		file_err (cc, inmr, "DBFIND");

	if (pcwo_rec.prod_qty == 0.00)
		return (EXIT_SUCCESS);

	/*-----------------------
	| read for standard UOM |
	-----------------------*/
	cc = find_hash (inum, &inum_rec, EQUAL, "r", inmr_rec.std_uom);

	/*------------------------
	| read for standard cost |
	------------------------*/
	strcpy (inei_rec.est_no, comm_rec.est_no);
	inei_rec.hhbr_hash = inmr_rec.hhbr_hash;
	cc = find_rec (inei, &inei_rec, COMPARISON,"r");
	if (cc)
		file_err (cc, inei, "DBFIND");

	/*----------------
	| calculate cost |
	----------------*/
	extend = pcwo_rec.prod_qty;
	extend *= out_cost (inei_rec.std_cost, inmr_rec.outer_size);

	pr_format (fin, fout, "PO_LINE", 1, 1);
	pr_format (fin, fout, "PO_LINE", 2, inmr_rec.item_no);
	pr_format (fin, fout, "PO_LINE", 3, " ");
	pr_format (fin, fout, "PO_LINE", 4, inmr_rec.description);
	pr_format (fin, fout, "PO_LINE", 5, pcwo_rec.prod_qty);
	pr_format (fin, fout, "PO_LINE", 6, inum_rec.uom);
	pr_format (fin, fout, "PO_LINE", 7, inei_rec.std_cost);
	pr_format (fin, fout, "PO_LINE", 8, extend);

	sprintf (err_str, "           %7.1f", inmr_rec.outer_size);
	pr_format (fin, fout, "INEX", 1, " ");
	pr_format (fin, fout, "INEX", 2, "Pricing Conv:");
	pr_format (fin, fout, "INEX", 3, " ");
	pr_format (fin, fout, "INEX", 4, err_str);
	pr_format (fin, fout, "INEX", 5, " ");
	pr_format (fin, fout, "INEX", 6, " ");
	pr_format (fin, fout, "INEX", 7, " ");
	pr_format (fin, fout, "INEX", 8, " ");

	/*-------------------
	| print batch sizes |
	-------------------*/
	sprintf (err_str, "Standard %14.6f", inei_rec.std_batch);
	pr_format (fin, fout, "INEX", 1, " ");
	pr_format (fin, fout, "INEX", 2, "Batch Sizes :");
	pr_format (fin, fout, "INEX", 3, " ");
	pr_format (fin, fout, "INEX", 4, err_str);
	pr_format (fin, fout, "INEX", 5, " ");
	pr_format (fin, fout, "INEX", 6, " ");
	pr_format (fin, fout, "INEX", 7, " ");
	pr_format (fin, fout, "INEX", 8, " ");

	sprintf (err_str, "Minimum  %14.6f", inei_rec.min_batch);
	pr_format (fin, fout, "INEX", 1, " ");
	pr_format (fin, fout, "INEX", 2, "Batch Sizes :");
	pr_format (fin, fout, "INEX", 3, " ");
	pr_format (fin, fout, "INEX", 4, err_str);
	pr_format (fin, fout, "INEX", 5, " ");
	pr_format (fin, fout, "INEX", 6, " ");
	pr_format (fin, fout, "INEX", 7, " ");
	pr_format (fin, fout, "INEX", 8, " ");

	sprintf (err_str, "Maximum  %14.6f", inei_rec.max_batch);
	pr_format (fin, fout, "INEX", 1, " ");
	pr_format (fin, fout, "INEX", 2, "Batch Sizes :");
	pr_format (fin, fout, "INEX", 3, " ");
	pr_format (fin, fout, "INEX", 4, err_str);
	pr_format (fin, fout, "INEX", 5, " ");
	pr_format (fin, fout, "INEX", 6, " ");
	pr_format (fin, fout, "INEX", 7, " ");
	pr_format (fin, fout, "INEX", 8, " ");

	pr_format (fin, fout, "VBLE_BLANK", 1, LINES - 4 +1);
	PageTrail ();

	fprintf (fout, ".EOF\n");
	pclose (fout);

	return (EXIT_SUCCESS);
}

int
HeadOutput (
 void)
{
	if ((fin = pr_open ("pc_womaint.p")) == NULL)
		file_err (errno, "pc_womaint.p", "pr_open");

	if ((fout = popen ("pformat", "w")) == NULL)
		file_err (errno, "pformat", "popen");

	fprintf (fout, ".START\n");
	fprintf (fout, ".OP\n");
	fprintf (fout, ".PL0\n");
	fprintf (fout, ".LP%d\n", local_rec.printerNumber);
	fprintf (fout, ".NC%d\n", 1);

	fprintf (fout, ".2\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L158\n");

	PageHead ();

	return (EXIT_SUCCESS);
}

int
PageHead (void)
{
	pr_format (fin, fout, "HMARGIN", 0, 0);
	pr_format (fin, fout, "ORDR_NUM", 1, pcwo_rec.order_no);
	pr_format (fin, fout, "MARGIN1", 0, 0);

	pr_format (fin, fout, "REQ_DATE", 1, local_rec.systemDate);
	pr_format (fin, fout, "MARGIN2", 0, 0);
	pr_format (fin, fout, "REQ_DATE", 1, DateToString (pcwo_rec.reqd_date));
	pr_format (fin, fout, "MARGIN3", 0, 0);

	pr_format (fin, fout, "SUPP_NAME", 1, "WORKS ORDER REQUISTION");
	pr_format (fin, fout, "SUPP_NAME", 2, " ");
	pr_format (fin, fout, "SUPP_NAME", 3, " ");

	sprintf (err_str,
			"%-2.2s  %-36.36s",
			pcwo_rec.br_no,
			comm_rec.est_name);
	pr_format (fin, fout, "SUPP_ADDR1", 1, err_str);
	sprintf (err_str, "BATCH NO: %-10.10s", pcwo_rec.batch_no);
	pr_format (fin, fout, "SUPP_ADDR1", 2, err_str);

	sprintf (err_str, "%-2.2s  %-36.36s", pcwo_rec.wh_no, comm_rec.cc_name);
	pr_format (fin, fout, "SUPP_ADDR2", 1, err_str);
	pr_format (fin, fout, "SUPP_ADDR2", 2, local_rec.status);
	pr_format (fin, fout, "SUPP_ADDR2", 3, " ");
	pr_format (fin, fout, "SUPP_ADDR3", 1, " ");
	pr_format (fin, fout, "SUPP_ADDR3", 2, " ");
	pr_format (fin, fout, "MARGIN4", 0, 0);

	return (EXIT_SUCCESS);
}

int
PageTrail (void)
{
	int		i = 0;

	pr_format (fin, fout, "MARGIN5", 0, 0);
	pr_format (fin, fout, "PO_TOT", 1, "0.00");
	pr_format (fin, fout, "MARGIN6", 0, 0);

	pcoi_rec.hhwo_hash = pcwo_rec.hhwo_hash;
	pcoi_rec.line_no = 0;
	cc = find_rec (pcoi, &pcoi_rec, GTEQ, "r");
	while (!cc && pcoi_rec.hhwo_hash == pcwo_rec.hhwo_hash)
	{
		i ++;
		if (i > 3)
			break;
		pr_format (fin, fout, "SPECIAL", 1, clip (pcoi_rec.text));

		cc = find_rec (pcoi, &pcoi_rec, NEXT, "r");
	}

	pr_format (fin, fout, "NEXT_PAGE", 0, 0);

	fflush (fout);

	return (EXIT_SUCCESS);
}

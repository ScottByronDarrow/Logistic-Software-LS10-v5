/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: pc_issue.c,v 5.10 2002/10/07 03:35:45 robert Exp $
|  Program Name  : (pc_issue.c    )                                   |
|  Program Desc  : (Production Control Issue raw-materials.     )     |
|---------------------------------------------------------------------|
|  Date Written  : (11/02/92)      | Author       : Trevor van Bremen |
|---------------------------------------------------------------------|
| $Log: pc_issue.c,v $
| Revision 5.10  2002/10/07 03:35:45  robert
| SC 4304 - fixed checkbox field and box alignment
|
| Revision 5.9  2002/07/24 08:38:57  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.8  2002/07/08 06:43:04  scott
| S/C 004085 - Item Number and  Serial Number are not aligned
|
| Revision 5.7  2002/07/03 04:20:10  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.6  2002/01/09 01:17:17  scott
| Updated to change function FindInsfCost () to accept an additional argument of
| the hhbrHash. This allows a serial item to be found using the hhwhHash OR the
| hhbrHash. Used in stock updates in case a serial item has been transfered.
|
| Revision 5.5  2001/08/09 09:14:39  scott
| Updated to add FinishProgram () function
|
| Revision 5.4  2001/08/06 23:35:00  scott
| RELEASE 5.0
|
| Revision 5.3  2001/07/25 02:18:22  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: pc_issue.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/PC/pc_issue/pc_issue.c,v 5.10 2002/10/07 03:35:45 robert Exp $";

#ifdef TABLINES
#undef TABLINES
#endif
#define	TABLINES	6

#ifdef MAXWIDTH  
#undef MAXWIDTH  
#endif
#define	MAXWIDTH	256
#define	EXPRY_DAYS	0L

#ifdef MAXLINES
#undef MAXLINES
#endif

#define MAXLINES	500

#include	<pslscr.h>
#include	<ml_std_mess.h>
#include	<ml_pc_mess.h>
#include	<proc_sobg.h>
#include	<GlUtils.h>
#include	<twodec.h>
#include	<number.h>
#include	<Costing.h>

#define	DEISSUE_OK	 (pcwo_rec.order_status [0] == 'I' || \
					  pcwo_rec.order_status [0] == 'F')

#define	CHK_ISSUE	 (pcwo_rec.order_status [0] == 'I')
#define	SERIAL		 (inmr_rec.serial_item [0] == 'Y')
#define	HD_SR(x)	 (store [x].head_ser)

#define	SR			store [line_cnt]
#define	NONSTOCK(x)	 (store [x].itemClass [0] == 'N' || \
					  store [x].itemClass [0] == 'Z')

#define	UPD_GL	 	 (generalLedgerFlag [0] == 'G')

#define		HEADER_SCN		1
#define		DETAIL_SCN		2
#define		SERIAL_SCN		3

#ifdef GVISION
#define	SERIAL_TABCOL 105
#else
#define	SERIAL_TABCOL 92
#endif

#define SLEEP_TIME	2

FILE	*pout,
		*pp;

#include	"schema"

struct comrRecord	comr_rec;
struct ccmrRecord	ccmr_rec;
struct commRecord	comm_rec;
struct ffdmRecord	ffdm_rec;
struct inccRecord	incc_rec;
struct inwuRecord	inwu_rec;
struct inmrRecord	inmr_rec;
struct inumRecord	inum_rec;
struct pcglRecord	pcgl_rec;
struct pclnRecord	pcln_rec;
struct pcmsRecord	pcms_rec;
struct pcsfRecord	pcsf_rec;
struct pcwcRecord	pcwc_rec;
struct pcwoRecord	pcwo_rec;
struct pcltRecord	pclt_rec;
struct esmrRecord	esmr_rec;

#include	<MoveRec.h>

	char	*data	= "data",
			*pcwo2	= "pcwo2";



	char	*serialSpace 	= 	"                         ",
			generalLedgerFlag [2],
			debitAccount [MAXLEVEL + 1],
			mfgAccount [MAXLEVEL + 1],
			localCurrency [4];

	int		pcReturn		= 0,
			envQcApply 		= FALSE,
			envPcGenNum		= FALSE,
			envSkQcAvl		= FALSE,
			numberLines		= 0,
			startEdit		= 0,
			lastWasSerial	= 0,
			lastLine		= 0,
			screenTwoLine	= 0,
			screenThreeLine = 0,
			oldLines		= 0;

	long	debitHash,
			mfgHash,
			currentHhcc;

	double	batchTotal		= 0.00;

#include	<SrchPcwo2.h>
#include	<MoveAdd.h>

/*
 * The SER_NO structure holds the serial numbers of all issued   
 * items if  the product is a serial controlled product.     
 */
struct	SER_NO
{
	char	serialNumber [26];
	char	location [11];
	struct	SER_NO	*next;
};
#define	SER_NULL	 ((struct SER_NO *) NULL)
struct	SER_NO		*freeSerial = SER_NULL;


struct	storeRec
{
	long	hhwhHash;
	long	mabrHash;
	long	hhumHash;
	long	hhccHash;
	int		lineNumber;
	char	itemClass [2];
	char	itemNumber [17];
	char	desc [41];
	char	oldQtyDone [2];
	char	qtyDone [2];
	char	cons [2];
	char	creditAccount [MAXLEVEL + 1];
	float	quantity;
	long	creditHash;
	double	amountIssued;
	double	amountReceipted;
	char	lotControl [2];
	char	UOM [5];
	float	waistPercent;
	float	cnvFct;
	int		altNumber;
	int		instrNumber;
	int		issueSequence;
	int		uniqueId;
	char	validSerial [26];
	struct	SER_NO	*head_ser;
} store [MAXLINES], tmp_store;


/*============================
| Local & Screen Structures. |
============================*/
struct
{
/* SCREEN 1 */
	char	reqBrNo [3];
	char	reqBrName [16];
	char	reqWhNo [3];
	char	reqWhName [10];
	char	recBrNo [3];
	char	recBrName [16];
	char	recWhNo [3];
	char	recWhName [10];
	char	order_no [8];
	char	batch_no [10];
	int		priority;
	long	cre_dte;
	char	head_item [17];
	char	strength [6];
	char	head_desc [36];
	char	head_desc2 [41];
	long	lsys_dte;
	char	sys_dte [11];
	long	rqd_dte;
	char	head_std_uom [5];
	char	head_alt_uom [5];
	double	qty_rqd;
	char	status [21];

/* SCREEN 2 */
	long	hhbr_hash;
	char	itemNumber [17];
	char	std_group [21];
	float	std_cnvFct;
	char	alt_group [21];
	float	alt_cnvFct;
	float	outer_size;
	char	iss_uom [5];
	char	std_uom [5];
	double	act_qty_tot;
	double	qty_tot;
	double	qty_prv;
	double	qtyIssue;
	char	qtyDone [4];
	char	changeQtyDone [2];
	int		dec_pt;
	char	serial_item [2];
	int		edit_ok;
	char	serialNumber [26];
	char	ser_loc [11];
	double	workCost;
	double	std_cost;
	int		lpno;
	char	qcReqd [2];
	char	dummy [11];
	char	systemDate [11];
	char	LL [2];
} local_rec, tmp_rec;

static	struct	var	vars [] =
{
	{HEADER_SCN, LIN, "order_no",	 2, 18, CHARTYPE,
		"UUUUUUU", "          ",
		" ", "", "Order Number:", " ",
		 NO, NO,  JUSTLEFT, "", "", local_rec.order_no},
	{HEADER_SCN, LIN, "batch_no",	 2, 43, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", "", "Batch Number:", " ",
		 NO, NO,  JUSTLEFT, "", "", local_rec.batch_no},
	{HEADER_SCN, LIN, "priority",	 2, 95, INTTYPE,
		"N", "          ",
		" ", "5", "Priority:", " ",
		 NA, NO,  JUSTLEFT, "", "", (char *) &local_rec.priority},
	{HEADER_SCN, LIN, "cre_dte",	 2, 118, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", "", "Date Raised:", " ",
		 NA, NO,  JUSTLEFT, "", "", (char *) &local_rec.cre_dte},
	{HEADER_SCN, LIN, "head_item",	 4, 18, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "", "Item Number:", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.head_item},
	{HEADER_SCN, LIN, "strength",	 5, 18, CHARTYPE,
		"AAAAA", "          ",
		" ", "", "Strength   :", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.strength},
	{HEADER_SCN, LIN, "head_desc",	 6, 18, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description:", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.head_desc},
	{HEADER_SCN, LIN, "head_desc2",	 7, 18, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "Description:", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.head_desc2},
	{HEADER_SCN, LIN, "rqd_dte",	 8, 18, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", local_rec.sys_dte, "Reqd Date  :", " ",
		 NA, NO, JUSTLEFT, "", "", (char *) &local_rec.rqd_dte},
	{HEADER_SCN, LIN, "head_std_uom",	 5, 88, CHARTYPE,
		"AAAA", "          ",
		"", "", "Standard:", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.head_std_uom},
	{HEADER_SCN, LIN, "head_alt_uom",	 5, 110, CHARTYPE,
		"AAAA", "          ",
		"", "", "  Alternate:", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.head_alt_uom},
	{HEADER_SCN, LIN, "qty_rqd",	 7, 88, DOUBLETYPE,
		"NNNNNNN.NNNNNN", "          ",
		"", "", "Batch Size:", " ",
		 NA, NO, JUSTRIGHT, "", "", (char *) &local_rec.qty_rqd},
	{HEADER_SCN, LIN, "reqBrNo",	 10, 18, CHARTYPE,
		"AA", "          ",
		" ", " ", "Requesting Br:", "Requesting Branch.",
		NA, NO,  JUSTRIGHT, "", "", local_rec.reqBrNo},
	{HEADER_SCN, LIN, "reqBrName",	 10, 23, CHARTYPE,
		"UUUUUUUUUUUUUUU", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.reqBrName},
	{HEADER_SCN, LIN, "reqWhNo",	 10, 65, CHARTYPE,
		"AA", "          ",
		" ", " ", "Wh:", "Requesting Warehouse.",
		NA, NO,  JUSTRIGHT, "", "", local_rec.reqWhNo},
	{HEADER_SCN, LIN, "reqWhName",	 10, 70, CHARTYPE,
		"UUUUUUUUU", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.reqWhName},
	{HEADER_SCN, LIN, "recBrNo",	 11, 18, CHARTYPE,
		"AA", "          ",
		" ", " ", "Receiving Br :", "Receiving Branch.",
		NA, NO,  JUSTRIGHT, "", "", local_rec.recBrNo},
	{HEADER_SCN, LIN, "recBrName",	 11, 23, CHARTYPE,
		"UUUUUUUUUUUUUUU", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.recBrName},
	{HEADER_SCN, LIN, "recWhNo",	 11, 65, CHARTYPE,
		"AA", "          ",
		" ", " ", "Wh:", "Receiving Warehouse.",
		NA, NO,  JUSTRIGHT, "", "", local_rec.recWhNo},
	{HEADER_SCN, LIN, "recWhName",	 11, 70, CHARTYPE,
		"UUUUUUUUU", "          ",
		" ", "", "", "",
		NA, NO,  JUSTLEFT, "", "", local_rec.recWhName},
	{DETAIL_SCN, TAB, "itemNumber",	MAXLINES, 0, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "", "   Item Number  ", " ",
		 NO, NO,  JUSTLEFT, "", "", local_rec.itemNumber},
	{DETAIL_SCN, TAB, "dec_pt",	 0, 0, INTTYPE,
		"N", "          ",
		"", "", "", " ",
		 ND, NO,  JUSTLEFT, "", "", (char *) &local_rec.dec_pt},
	{DETAIL_SCN, TAB, "uom",		 0, 0, CHARTYPE,
		"AAAA", "          ",
		"", local_rec.iss_uom, "UOM", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.iss_uom},
	{DETAIL_SCN, TAB, "outer",	 0, 0, FLOATTYPE,
		"FFFFFFF.FFFFFFF", "          ",
		"", "", "", " ",
		 ND, NO,  JUSTLEFT, "", "", (char *) &local_rec.outer_size},
	{DETAIL_SCN, TAB, "std_uom",	 0, 0, CHARTYPE,
		"AAAA", "          ",
		"", "", "", " ",
		 ND, NO,  JUSTLEFT, "", "", local_rec.std_uom},
	{DETAIL_SCN, TAB, "act_qty_tot",	 0, 0, DOUBLETYPE,
		"NNNNNNN.NNNNNN", "          ",
		"", "", "", " ",
		 ND, NO, JUSTRIGHT, "", "", (char *) &local_rec.act_qty_tot},
	{DETAIL_SCN, TAB, "qty_tot",	 0, 0, DOUBLETYPE,
		"NNNNNNN.NNNNNN", "          ",
		"", "", " Total Required", " ",
		 NA, NO, JUSTRIGHT, "", "", (char *) &local_rec.qty_tot},
	{DETAIL_SCN, TAB, "qty_prv",	 0, 0, DOUBLETYPE,
		"NNNNNNN.NNNNNN", "          ",
		"", "", "Previous Issues", " ",
		 NA, NO, JUSTRIGHT, "", "", (char *) &local_rec.qty_prv},
	{DETAIL_SCN, TAB, "qtyIssue",	 0, 0, DOUBLETYPE,
		"NNNNNNN.NNNNNN", "          ",
		"", "", "  This  Issue  ", " ",
		 NO, NO, JUSTRIGHT, "", "", (char *) &local_rec.qtyIssue},
	{DETAIL_SCN, TAB, "LL", 	 0, 0, CHARTYPE, 
		"U", "          ", 
		" ", "N", "L", "Lot / Location Selection. <return> ", 
		 YES, NO,  JUSTLEFT, "", "", local_rec.LL}, 
	{DETAIL_SCN, TAB, "serial_item",	 0, 0, CHARTYPE,
		"U", "          ",
		"", "", "", "",
		 ND, NO, JUSTLEFT, "", "", local_rec.serial_item},
	{DETAIL_SCN, TAB, "qtyDone",	 0, 3, CHARTYPE,
		"UAA", "          ",
		" ","", "Completed", "Is this line FULLY ISSUED? Y(es) or N(o)",
		YES, NO,  JUSTLEFT, "YNyn", "", local_rec.qtyDone},
	{DETAIL_SCN, TAB, "changeQtyDone",	 0, 0, CHARTYPE,
		"U", "          ",
		" ", "", "", "",
		 ND, NO,  JUSTLEFT, "", "", local_rec.changeQtyDone},
	{DETAIL_SCN, TAB, "std_group",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		"", "", "", " ",
		 ND, NO,  JUSTLEFT, "", "", local_rec.std_group},
	{DETAIL_SCN, TAB, "std_cnvFct",	 0, 0, FLOATTYPE,
		"NNNNNNN.NNNNNN", "          ",
		"", "", "", " ",
		 ND, NO, JUSTRIGHT, "", "", (char *) &local_rec.std_cnvFct},
	{DETAIL_SCN, TAB, "alt_group",	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		"", "", "", " ",
		 ND, NO,  JUSTLEFT, "", "", local_rec.alt_group},
	{DETAIL_SCN, TAB, "alt_cnvFct",	 0, 0, FLOATTYPE,
		"NNNNNNN.NNNNNN", "          ",
		"", "", "", " ",
		 ND, NO, JUSTRIGHT, "", "", (char *) &local_rec.alt_cnvFct},
	{DETAIL_SCN, TAB, "edit_ok",	 0, 0, INTTYPE,
		"NNNNNNNNNN", "          ",
		"", "", "", " ",
		 ND, NO,  JUSTRIGHT, "", "", (char *)&local_rec.edit_ok},
	{DETAIL_SCN, TAB, "qcReqd",	 0, 0, CHARTYPE,
		"U", "          ",
		"", "", "", " ",
		 ND, NO,  JUSTLEFT, "", "", local_rec.qcReqd},
	{SERIAL_SCN, TAB, "serialNumber",	MAXLINES, 0, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUUUUUUU", "          ",
		" ", " ", "Serial Number      ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.serialNumber},
	{SERIAL_SCN, TAB, "ser_loc",	 0, 0, CHARTYPE,
		"UUUUUUUUUU", "          ",
		" ", "", " Location ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.ser_loc},
	{0, LIN, "",		 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};


/*=====================
| function prototypes |
=====================*/
float 	GetSerialNo 		 (void);
int		LotSelectFlag;
int 	CalcConv 			 (int);
int 	CheckLotLocations 	 (void);
int 	CheckSerialNo 		 (void);
int 	DeleteLine 			 (void);
int 	DeleteSerialLine 	 (void);
int 	DeltaNo 			 (void);
int 	GetAccounts 		 (char *, int);
int 	LoadPcms 			 (void);
int 	ShowInsf 			 (int, int);
int 	Update 				 (void);
int 	ValidQuantity 		 (double, int);
int 	ValidUOM 			 (void);
int 	heading 			 (int);
int 	spec_valid 			 (int);
void	ReadDefault 		 (char *);
void 	AddPcgl 			 (int);
void 	CloseAudit 			 (void);
void 	CloseDB 			 (void);
void 	DisplayOrderDetails  (int);
void 	DoTransaction 		 (int);
void 	FreeSerial 			 (void);
void 	GetInum 			 (int);
void 	InitStore 			 (void);
void 	OpenAudit 			 (void);
void 	OpenDB 				 (void);
void 	PrintCoStuff 		 (void);
void 	PrintDetails 		 (int, float, double, char *, char *, char *,float);
void 	SerialDealloc 		 (struct SER_NO *);
void 	SrchInum 			 (char *);
void 	shutdown_prog 		 (int);
void 	tab_other 			 (int);

struct SER_NO *SerialAllocate (void);
#include	<LocHeader.h>

/*===========================
| Main Processing Routine . |
===========================*/
int
main (
 int  argc, 
 char *argv [])
{
	char	*sptr;
	
	/*
	 * Works order number is M (anually or S (ystem generated. 
	 */
	sptr = chk_env ("PC_GEN_NUM");
	if (sptr)
		envPcGenNum = (*sptr == 'M' || *sptr == 'm') ? FALSE : TRUE;
	else
		envPcGenNum = TRUE;
	/*
	 * Include QC in available stock.                        
	 */
	sptr = chk_env ("SK_QC_AVL");
	envSkQcAvl = (sptr == (char *)0) ? 0 : atoi (sptr);

	sptr = strrchr (argv [0], '/');
	if (sptr == (char *) 0)
		sptr = argv [0];
	else
		sptr ++;

	if (!strncmp (sptr, "pc_issue", 8))
		pcReturn = FALSE;
	else
		pcReturn = TRUE;


	SETUP_SCR (vars);


	if (argc  !=  3)
	{
		print_at (0,0,mlPcMess700,argv [0]);
		return (EXIT_FAILURE);
	}

	envQcApply = (sptr = chk_env ("QC_APPLY")) ? atoi (sptr) : 0;

	local_rec.lpno = atoi (argv [1]);
	switch (argv [2] [0])
	{
	case	'G':
	case	'g':
		strcpy (generalLedgerFlag, "G");
		break;

	case	'J':
	case	'j':
		strcpy (generalLedgerFlag, "J");
		break;

	default:
		print_at (0,0,mlPcMess700,argv [0]);
		return (EXIT_FAILURE);
	}

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));

	/*
	 * Setup required parameters	
	 */
	init_scr ();
	set_tty ();
	set_masks ();
/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (DETAIL_SCN, store, sizeof (struct storeRec));
#endif

	OpenDB ();

	FLD ("LL") = ND;
	if (SK_BATCH_CONT || MULT_LOC)
		FLD ("LL") = YES;

	OpenAudit ();

	strcpy (local_rec.sys_dte, DateToString (TodaysDate ()));
	local_rec.lsys_dte = TodaysDate ();

	if (pcReturn)
	{
		FLD ("itemNumber") 	= NA;
		FLD ("qtyDone") 	= NA;
		vars [label ("qtyIssue")].prmpt = "Return Qty";
	}


	swide ();

	tab_row = 13;
	tab_col = 2;

	/*===================================
	| Beginning of input control loop	|
	===================================*/
	while (prog_exit == 0)
	{
		/*
		 * Reset control flags	
		 */
		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		search_ok	= TRUE;

		init_vars (HEADER_SCN);
		/*
		 * Enter screen 1 linear input	
		 */
		strcpy (local_rec.status, "                   ");
		heading (HEADER_SCN);
		entry (HEADER_SCN);
		if (restart || prog_exit)
		{
			abc_unlock (pcwo);
			continue;
		}

		init_vars (SERIAL_SCN);
		lcount [SERIAL_SCN] = 0;

		/*
		 * Edit screens as required.	
		 */
		lastWasSerial	= FALSE;
		lastLine 		= -1;
		init_vars (DETAIL_SCN);
		lcount [DETAIL_SCN] = 0;
		heading (DETAIL_SCN);
		if (LoadPcms ())
		{
			abc_unlock (pcwo);
			continue;
		}

		scn_display (DETAIL_SCN);

		_edit (DETAIL_SCN, startEdit, label ("qtyIssue"));

		if (FLD ("LL") != ND)
		{
			while (CheckLotLocations () && !restart) 
			{
				strcpy (local_rec.LL, "N");
				scn_display (DETAIL_SCN);
				_edit (DETAIL_SCN, startEdit, label ("LL"));
			}
		}
		if (restart)
		{
			abc_unlock (pcwo);
			continue;
		}

		abc_selfield (inmr, "inmr_hhbr_hash");
		Update ();
		abc_selfield (inmr, "inmr_id_no");
	}
	shutdown_prog (0);
	return (EXIT_SUCCESS);
}

/*=======================
| Program exit sequence	|
=======================*/
void
shutdown_prog (
 int error)
{
	clear ();
	snorm ();

	FreeSerial ();

	recalc_sobg ();

	if (!error)
		CloseAudit ();
	CloseDB (); 
	FinishProgram ();
}

void
ReadDefault (
	char	*category)
{
	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"WIP D MATL",
		" ",
		category
	);
	debitHash = glmrRec.hhmr_hash;
	strcpy (debitAccount, glmrRec.acc_no);

	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"MAN D MATL",
		" ",
		category
	);
	mfgHash = glmrRec.hhmr_hash;
	strcpy (mfgAccount, glmrRec.acc_no);

	return;
}

/*=======================
| Open data base files	|
=======================*/
void
OpenDB (
 void)
{
	MoveOpen	=	TRUE;

	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

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

	abc_alias (pcwo2, pcwo);

	open_rec (ffdm, ffdm_list, FFDM_NO_FIELDS, "ffdm_id_no2");
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	open_rec (incc,  incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (inmr,  inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (inum,  inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	open_rec (pcgl,  pcgl_list, PCGL_NO_FIELDS, "pcgl_id_no");
	open_rec (pcln,  pcln_list, PCLN_NO_FIELDS, "pcln_id_no");
	open_rec (pcms,  pcms_list, PCMS_NO_FIELDS, "pcms_id_no");
	open_rec (pcsf,  pcsf_list, PCSF_NO_FIELDS, "pcsf_id_no");
	open_rec (pcwc,  pcwc_list, PCWC_NO_FIELDS, "pcwc_hhwc_hash");
	open_rec (pcwo,  pcwo_list, PCWO_NO_FIELDS, "pcwo_id_no");
	open_rec (pcwo2,  pcwo_list, PCWO_NO_FIELDS, "pcwo_id_no3");
	open_rec (pclt,  pclt_list, PCLT_NO_FIELDS, "pclt_hhwo_hash");
	open_rec (esmr,  esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (glmr,  glmr_list, GLMR_NO_FIELDS, "glmr_id_no");
	open_rec (inwu, inwu_list, INWU_NO_FIELDS, "inwu_id_no");
	open_rec ("move", move_list, MOVE_NO_FIELDS, "move_move_hash");

	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, comm_rec.est_no);
	strcpy (ccmr_rec.cc_no, comm_rec.cc_no);
	cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "ccmr", "DBFIND");

	currentHhcc = ccmr_rec.hhcc_hash;

	OpenLocation (ccmr_rec.hhcc_hash);

	if (llctPcIssue [0] == 'V')
		LotSelectFlag	=	INP_VIEW;
	if (llctPcIssue [0] == 'A')
		LotSelectFlag	=	INP_AUTO;
	if (llctPcIssue [0] == 'M')
	{
		strcpy (StockTake, "Y");
		LotSelectFlag	=	INP_VIEW;
	}
	if (llctPcIssue [0] == 'N')
	{
		SK_BATCH_CONT 	=	FALSE;
		MULT_LOC		=	FALSE;
		FLD ("LL") 		= 	ND;
	}
	OpenGlmr ();
}

/*=======================
| Close data base files	|
=======================*/
void
CloseDB (
 void)
{
	abc_fclose (ccmr);
    abc_fclose (ffdm);
	abc_fclose (incc);
	abc_fclose (inmr);
	abc_fclose (inum);
	abc_fclose (pcgl);
	abc_fclose (pcln);
	abc_fclose (pcms);
	abc_fclose (pcsf);
	abc_fclose (pcwc);
	abc_fclose (pcwo);
	abc_fclose (pcwo2);
	abc_fclose (pclt);
	abc_fclose (esmr);
	abc_fclose (glmr);
	abc_fclose (inwu);
	abc_fclose ("move");
	CloseLocation ();
	GL_Close ();

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
	if (restart)
		return (EXIT_SUCCESS);

	if (scn == HEADER_SCN)
	{
		swide ();
		clear ();
		if (!pcReturn)
			rv_pr (ML (mlPcMess012), 53, 0, 1);
		else
			rv_pr (ML (mlPcMess013), 52, 0, 1);

		box (0, 1, 132, 10);
		box (67, 3, 65, 5);
		move (0,3); PGCHAR (10); line (67); PGCHAR (8); line (64); PGCHAR (11);
		move (67, 6); PGCHAR (10); line (64); PGCHAR (11);
		move (0, 9); PGCHAR (10); line (131); PGCHAR (11);
		move (67, 9); PGCHAR (9);

		rv_pr (local_rec.status, 59, 2, FALSE);
		rv_pr (ML (mlPcMess014), 88, 4, FALSE);
	}

	if (scn == DETAIL_SCN)
	{
		scn_write (HEADER_SCN);
		scn_display (HEADER_SCN);

		scn_set (SERIAL_SCN);
		tab_col = SERIAL_TABCOL;
		/*
		box (tab_col, tab_row - 1, 38, 8);
		*/
		scn_write (SERIAL_SCN);
		scn_display (SERIAL_SCN);

		tab_col = 2;
		/*
		box (tab_col, tab_row - 1, 83, 8);
		*/
	}

	if (scn == SERIAL_SCN)
	{
		tab_col = SERIAL_TABCOL;
		/*
		box (tab_col, tab_row - 1, 38, 8);
		*/
	}

	PrintCoStuff ();

	scn_set (scn);

	/* reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write (scn);
	return (EXIT_SUCCESS);
}

void
PrintCoStuff (
 void)
{
	move (0, 21);
	line (132);
	print_at (22, 0,ML (mlStdMess038),comm_rec.co_no, comm_rec.co_short);
	print_at (22,40,ML (mlStdMess039),comm_rec.est_no, comm_rec.est_short);
	print_at (22,60,ML (mlStdMess099), comm_rec.cc_no, comm_rec.cc_short);

}
/*===============================
| Validate entered field (s)	|
===============================*/
int
spec_valid (
 int field)
{
	int		TempLine;
	float	tmp_qty;

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
			SearchOrder (temp_str, "FIAR", comm_rec.est_no, comm_rec.cc_no);
			return (EXIT_SUCCESS);
		}
		strcpy (pcwo_rec.co_no, comm_rec.co_no);
		strcpy (pcwo_rec.br_no, comm_rec.est_no);
		strcpy (pcwo_rec.wh_no, comm_rec.cc_no);
		if (envPcGenNum)
			strcpy (pcwo_rec.order_no, zero_pad (local_rec.order_no, 7));
		else
			strcpy (pcwo_rec.order_no, local_rec.order_no);
		cc = find_rec (pcwo, &pcwo_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlPcMess067));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		clear_mess ();
		DisplayOrderDetails (FALSE);
		if (restart)
		{
			switch (pcwo_rec.order_status [0])
			{
			case	'P':
				sprintf (err_str, ML (mlPcMess003));
				break;

			case	'D':
				sprintf (err_str, ML (mlPcMess004));
				break;

			case	'C':
				sprintf (err_str, ML (mlPcMess136));
				break;

			default:
				sprintf (err_str, ML (mlPcMess005));
				break;
			}
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_SUCCESS);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("batch_no"))
	{
		if (SRCH_KEY)
		{
			SearchBatch (temp_str, "FIAR", comm_rec.est_no, comm_rec.cc_no);
			abc_selfield (pcwo, "pcwo_id_no");
			return (EXIT_SUCCESS);
		}
		strcpy (pcwo_rec.co_no, comm_rec.co_no);
		strcpy (pcwo_rec.br_no, comm_rec.est_no);
		strcpy (pcwo_rec.wh_no, comm_rec.cc_no);
		strcpy (pcwo_rec.batch_no, local_rec.batch_no);
		cc = find_rec (pcwo2, &pcwo_rec, EQUAL, "r");
		if (cc)
		{
			sprintf (err_str,ML (mlPcMess138), pcwo_rec.batch_no);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			sprintf (local_rec.batch_no, "%-10.10s", " ");
			return (EXIT_FAILURE);
		}
		clear_mess ();
		DisplayOrderDetails (TRUE);
		if (restart)
		{
			switch (pcwo_rec.order_status [0])
			{
			case	'P':
				sprintf (err_str, ML (mlPcMess003));
				break;

			case	'D':
				sprintf (err_str, ML (mlPcMess004));
				break;

			case	'C':
				sprintf (err_str, ML (mlPcMess136));
				break;

			default:
				sprintf (err_str, ML (mlPcMess005));
				break;
			}
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_SUCCESS);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("head_item"))
	{
		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}
		abc_selfield (inmr, "inmr_id_no");
		
		cc = FindInmr (comm_rec.co_no, local_rec.head_item, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.head_item);
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			print_mess (ML (mlStdMess001));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();
		if
		(
			strcmp (inmr_rec.source, "BP") &&
			strcmp (inmr_rec.source, "BM") &&
			strcmp (inmr_rec.source, "MC") &&
			strcmp (inmr_rec.source, "MP")
		)
		{
			print_mess (ML (mlPcMess006));
			return (EXIT_FAILURE);
		}

		inum_rec.hhum_hash	=	inmr_rec.std_uom;
		cc = find_rec (inum, &inum_rec, EQUAL, "r");
		strcpy (local_rec.head_std_uom, (cc) ?"   " : inum_rec.uom);

		inum_rec.hhum_hash	=	inmr_rec.alt_uom;
		cc = find_rec (inum, &inum_rec, EQUAL, "r");
		strcpy (local_rec.head_alt_uom, (cc) ?"   " : inum_rec.uom);

		strcpy (SR.UOM, local_rec.head_std_uom);

		sprintf (local_rec.head_item, "%-16.16s", inmr_rec.item_no);
		DSP_FLD ("head_item");
		sprintf (local_rec.head_desc, "%-35.35s", inmr_rec.description);
		sprintf (local_rec.strength, "%-5.5s", inmr_rec.description + 35);
		sprintf (local_rec.head_desc2, "%-40.40s", inmr_rec.description2);
		DSP_FLD ("strength");
		DSP_FLD ("head_desc");
		DSP_FLD ("head_desc2");
		DSP_FLD ("head_std_uom");
		DSP_FLD ("head_alt_uom");

		local_rec.hhbr_hash = inmr_rec.hhbr_hash;

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("itemNumber"))
	{
		if (last_char == DELLINE || dflt_used)
			return (DeleteLine ());

		if (SRCH_KEY)
		{
#ifdef GVISION
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			strcpy (local_rec.itemNumber, temp_str);
#else
			screenTwoLine = line_cnt;
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			strcpy (local_rec.itemNumber, temp_str);
			heading (1);
			scn_display (1);
			heading (DETAIL_SCN);
			line_cnt = screenTwoLine;
#endif	/* GVISION */

			return (EXIT_SUCCESS);
		}
		abc_selfield (inmr, "inmr_id_no");

		cc = FindInmr (comm_rec.co_no, local_rec.itemNumber, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.itemNumber);
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			print_mess (ML (mlStdMess001));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();

		InitStore ();

		/*
		 * Get the UOM conversion factor	
		 */
		SR.hhumHash = inmr_rec.std_uom;
		GetInum (line_cnt);

		local_rec.dec_pt = inmr_rec.dec_pt;
		sprintf (local_rec.itemNumber, "%-16.16s", inmr_rec.item_no);
		sprintf (SR.itemNumber, "%-16.16s", inmr_rec.item_no);
		sprintf (SR.itemClass, "%-1.1s", inmr_rec.inmr_class);
		sprintf (SR.desc, "%-40.40s", inmr_rec.description);
		strcpy (local_rec.serial_item, inmr_rec.serial_item);
		strcpy (local_rec.qcReqd, inmr_rec.qc_reqd);
		local_rec.act_qty_tot	= 0.00;
		local_rec.qty_tot		= 0.00;
		local_rec.qty_prv		= 0.00;
		DSP_FLD ("uom");
		DSP_FLD ("qty_tot");
		DSP_FLD ("qty_prv");

		ReadDefault (inmr_rec.category);

		if (GetAccounts (inmr_rec.category, line_cnt))
			return (EXIT_FAILURE);

		SR.mabrHash = inmr_rec.hhbr_hash;
		strcpy (SR.lotControl, inmr_rec.lot_ctrl);

		SR.hhwhHash = 0L;

		incc_rec.hhcc_hash = currentHhcc;
		incc_rec.hhbr_hash = (inmr_rec.hhsi_hash == 0) ? inmr_rec.hhbr_hash : inmr_rec.hhsi_hash;
		cc = find_rec (incc, &incc_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess192));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		SR.hhwhHash = incc_rec.hhwh_hash;
		SR.hhccHash = incc_rec.hhcc_hash;

		local_rec.edit_ok	= TRUE;
		lastWasSerial		= TRUE;
		lastLine			= line_cnt;
		tab_other (line_cnt);

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("uom"))
	{
		if (SRCH_KEY)
		{
			screenTwoLine = line_cnt;

			SrchInum (temp_str);
			abc_selfield (inum, "inum_hhum_hash");

			heading (1);
			scn_display (1);

			heading (DETAIL_SCN);
			line_cnt = screenTwoLine;
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
			return (EXIT_SUCCESS);

		sprintf (inum_rec.uom, "%-4.4s", temp_str);
		abc_selfield (inum, "inum_uom");
		cc = find_rec (inum, &inum_rec, COMPARISON, "r");
		abc_selfield (inum, "inum_hhum_hash");
		if (cc)
		{
			print_mess (ML (mlStdMess028));
			return (EXIT_FAILURE);
		}

		if
		 (
		    strcmp (inum_rec.uom_group, local_rec.std_group) &&
		    strcmp (inum_rec.uom_group, local_rec.alt_group)
		)
		{
			print_mess (ML (mlStdMess028));
			return (EXIT_FAILURE);
		}

		if (ValidItemUom (SR.mabrHash, inum_rec.hhum_hash))
		{
			print_mess (ML (mlStdMess028));
			return (EXIT_FAILURE);
		}

		CalcConv (line_cnt);

		if (!ValidUOM ())
			return (EXIT_FAILURE);

		strcpy (local_rec.iss_uom, inum_rec.uom);
		SR.hhumHash = inum_rec.hhum_hash;

		DSP_FLD ("uom");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("qtyIssue"))
	{
		if (dflt_used && !pcReturn)
			local_rec.qtyIssue = local_rec.qty_tot - local_rec.qty_prv;

		local_rec.qtyIssue = n_dec (local_rec.qtyIssue, local_rec.dec_pt);
		DSP_FLD ("qtyIssue");
		if (!ValidQuantity (local_rec.qtyIssue, local_rec.dec_pt))
			return (EXIT_FAILURE);

		if (!DEISSUE_OK && local_rec.qtyIssue < 0.00)
		{
			print_mess ((pcReturn) ? ML (mlPcMess126) : ML (mlPcMess125));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		abc_selfield (incc, "incc_hhwh_hash");
		incc_rec.hhwh_hash = SR.hhwhHash;
		cc = find_rec (incc, &incc_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, incc, "DBFIND");

		abc_selfield (incc, "incc_id_no");

		if (!NONSTOCK (line_cnt))
		{
			if (!pcReturn )
			{
				if (envQcApply && envSkQcAvl)
					tmp_qty = (	incc_rec.closing_stock -
								incc_rec.qc_qty) * SR.cnvFct;
				else
					tmp_qty = incc_rec.closing_stock * SR.cnvFct;
				if ((tmp_qty < local_rec.qtyIssue) && local_rec.qtyIssue != 0.00)
				{
					if (local_rec.qcReqd [0] == 'Y' && envSkQcAvl && envQcApply )
					{
						sprintf (err_str, ML (mlPcMess007), BELL, 
							n_dec (incc_rec.closing_stock -
									incc_rec.qc_qty,
									local_rec.dec_pt),
							n_dec (incc_rec.qc_qty, local_rec.dec_pt),
							tmp_qty,
							local_rec.qtyIssue);
					}
					else 
					{
						sprintf 
						 (
							err_str, 
							ML (mlPcMess008), 
							BELL, 
							n_dec (incc_rec.closing_stock, local_rec.dec_pt),
							tmp_qty,
							clip (local_rec.itemNumber), 
							local_rec.qtyIssue
						);
					}
					print_mess (err_str);
					sleep (sleepTime);
					clear_mess ();
					return (EXIT_FAILURE);
					}
				}
				else
				{
				if (local_rec.qtyIssue > local_rec.qty_prv)
				{
					sprintf (err_str, ML (mlPcMess009), BELL, local_rec.qty_prv,
						local_rec.qtyIssue);
					print_mess (err_str);
					sleep (sleepTime);
					clear_mess ();
					return (EXIT_FAILURE);
				}
			}
		}

		if (local_rec.serial_item [0] == 'Y')
		{
			if (local_rec.qtyIssue == 0.00)
			{
			    if (HD_SR (line_cnt) != SER_NULL)
			    {
					SerialDealloc (HD_SR (line_cnt));
					HD_SR (line_cnt) = SER_NULL;
			    }
				screenTwoLine = line_cnt;
			    tab_col = SERIAL_TABCOL;
			    scn_set (SERIAL_SCN);
			    init_vars (SERIAL_SCN);
			    lcount [SERIAL_SCN] = 0;
			    scn_write (SERIAL_SCN);
			    scn_display (SERIAL_SCN);
			    tab_col = 2;
			    scn_set (DETAIL_SCN);
				line_cnt = screenTwoLine;
			}
			else
			{
			    local_rec.qtyIssue = GetSerialNo ();

			    scn_set (DETAIL_SCN);
				line_cnt = screenTwoLine;

			    if (local_rec.qtyIssue <= 0.00)
			    {
					if (local_rec.qtyIssue == -1.00)
						return (EXIT_FAILURE);

					scn_set (SERIAL_SCN);
					line_cnt = screenThreeLine;
					strcpy (local_rec.serialNumber, serialSpace);
					strncpy (local_rec.ser_loc, serialSpace, 10);
					DSP_FLD ("serialNumber");
					DSP_FLD ("ser_loc");

					print_mess (ML (mlPcMess127));
					sleep (sleepTime);
					clear_mess ();
					scn_set (DETAIL_SCN);
					line_cnt = screenTwoLine;
					return (EXIT_FAILURE);
			    }

			    DSP_FLD ("qtyIssue");
			}
		}

		SR.quantity	=	 (float) local_rec.qtyIssue;

		if (prog_status != ENTRY && (SK_BATCH_CONT || MULT_LOC))
		{
			    /*
				 * Reenter Location. 
				 */
				do
				{
					strcpy (local_rec.LL, "N");
					get_entry (label ("LL"));
					cc = spec_valid (label ("LL"));
				} while (cc && (FLD ("LL") == ND));

		}
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate lots and locations. 
	 */
	if (LCHECK ("LL"))
	{
		int		LLReturnValue	=	0;

		if (FLD ("LL") == ND || NONSTOCK (line_cnt))
			return (EXIT_SUCCESS);

		TempLine	=	lcount [DETAIL_SCN];

		LLReturnValue = DisplayLL
			 (									/*----------------------*/
				line_cnt,						/*	Line number.		*/
				19,								/*  Row for window		*/
				tab_col + 22,					/*  Col for window		*/
				4,								/*  length for window	*/
				SR.hhwhHash, 					/*	Warehouse hash.		*/
				SR.hhumHash,					/*	UOM hash			*/
				SR.hhccHash,					/*	CC hash.			*/
				SR.UOM,							/* UOM					*/
				SR.quantity,					/* Quantity.			*/
				SR.cnvFct,						/* Conversion factor.	*/
				TodaysDate (), 					/* Expiry Date.			*/
				LotSelectFlag,					/* Silent mode			*/
				 (local_rec.LL [0] == 'Y'),		/* Input Mode.			*/
				SR.lotControl					/* Lot controled item. 	*/
												/*----------------------*/
			);
		/*-----------------
		| Redraw screens. |
		-----------------*/
		strcpy (local_rec.LL, "Y");
		putval (line_cnt);

		lcount [DETAIL_SCN] = (line_cnt + 1 > lcount [DETAIL_SCN]) ? line_cnt + 1 : lcount [DETAIL_SCN];
		scn_write (DETAIL_SCN);
		scn_display (DETAIL_SCN);
		lcount [DETAIL_SCN] = TempLine;
		PrintCoStuff ();

		scn_set (DETAIL_SCN);
		if (LLReturnValue)
			return (EXIT_FAILURE);
		
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("qtyDone"))
	{
		strcpy (local_rec.qtyDone,(local_rec.qtyDone[0] == 'Y') ? "Yes" : "No");
		strcpy (local_rec.changeQtyDone, "Y");

		sprintf (store [line_cnt].qtyDone, "%-1.1s", local_rec.qtyDone);
		if (line_cnt >= numberLines)
			sprintf (store [line_cnt].oldQtyDone, "%-1.1s", local_rec.qtyDone);

		DSP_FLD ("qtyDone");

		return (EXIT_SUCCESS);
	}

	/*-----------------------------------
	| Check If Serial No already exists |
	-----------------------------------*/
	if (LCHECK ("serialNumber"))
	{
		if (SRCH_KEY)
		{
			SearchInsf (SR.hhwhHash, (pcReturn) ? "S" : "F", temp_str);

			screenThreeLine = line_cnt;

			heading (1);
			scn_display (1);

			scn_set (DETAIL_SCN);
			heading (DETAIL_SCN);
			tmp_rec = local_rec;
			scn_display (DETAIL_SCN);
			local_rec = tmp_rec;
			line_cnt = screenTwoLine;
			line_display ();

			line_cnt = screenThreeLine;
			scn_set (SERIAL_SCN);
			return (EXIT_SUCCESS);
		}

		if ((dflt_used || last_char == DELLINE) && prog_status != ENTRY)
			return (DeleteSerialLine ());

		if (!strcmp (local_rec.serialNumber, serialSpace))
		{
			strncpy (local_rec.ser_loc, serialSpace, 10);
			DSP_FLD ("ser_loc");

			print_mess (ML (mlStdMess201));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		if (!CheckSerialNo ())
		{
			strcpy (local_rec.serialNumber, serialSpace);
			strncpy (local_rec.ser_loc, serialSpace, 10);
			DSP_FLD ("serialNumber");
			DSP_FLD ("ser_loc");

			print_mess (ML(mlStdMess097));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		cc =	FindInsf 
				(
					SR.hhwhHash, 
					0L,
					local_rec.serialNumber, 
					(pcReturn) ? "S" : "F", 
					"r"
				);
		if (cc)
		{
			cc = FindInsf (SR.hhwhHash, 0L, local_rec.serialNumber, "C", "r");
			if (cc)
			{
				strcpy (local_rec.serialNumber, serialSpace);
				strncpy (local_rec.ser_loc, serialSpace, 10);
				DSP_FLD ("serialNumber");
				DSP_FLD ("ser_loc");

				print_mess (ML (mlStdMess201));
				return (EXIT_FAILURE);
			}
		}
		sprintf (store [line_cnt].validSerial, "%-25.25s", local_rec.serialNumber);
		sprintf (local_rec.ser_loc, "%-10.10s", insfRec.location);
		DSP_FLD ("ser_loc");
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

void
DisplayOrderDetails (
 int flag)
{
	switch (pcwo_rec.order_status [0])
	{
	case	'P':
		strcpy (local_rec.status, "STATUS: Planned    ");
		restart = TRUE;
		break;

	case	'F':
		strcpy (local_rec.status, "STATUS: Firm Planned");
		break;

	case	'I':
		strcpy (local_rec.status, "STATUS: Issuing    ");
		break;

	case	'A':
		strcpy (local_rec.status, "STATUS: Allocated  ");
		break;

	case	'R':
		strcpy (local_rec.status, "STATUS: Released   ");
		break;

	case	'D':
		strcpy (local_rec.status, "STATUS: Deleted    ");
		restart = TRUE;
		break;

	case	'C':
		strcpy (local_rec.status, "STATUS: Closing    ");
		restart = TRUE;
		break;

	default:
		strcpy (local_rec.status, "STATUS: Closed     ");
		restart = TRUE;
		break;
	};

	rv_pr (local_rec.status, 59, 2, FALSE);

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
		file_err (cc, "esmr", "DBFIND");
	strcpy (local_rec.reqBrName, esmr_rec.short_name);
	DSP_FLD ("reqBrNo");
	DSP_FLD ("reqBrName");

	/* find receiving branch name */
	strcpy (esmr_rec.co_no, comm_rec.co_no);
	strcpy (esmr_rec.est_no, local_rec.recBrNo);
	cc = find_rec (esmr, &esmr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, "esmr", "DBFIND");
	strcpy (local_rec.recBrName, esmr_rec.short_name);
	DSP_FLD ("recBrNo");
	DSP_FLD ("recBrName");

	/* find requesting warehouse name */
	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, local_rec.reqBrNo);
	strcpy (ccmr_rec.cc_no, local_rec.reqWhNo);
	cc = find_rec (ccmr, &ccmr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, "ccmr", "DBFIND");
	strcpy (local_rec.reqWhName, ccmr_rec.acronym);
	DSP_FLD ("reqWhNo");
	DSP_FLD ("reqWhName");

	/* find receiving warehouse name */
	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, local_rec.recBrNo);
	strcpy (ccmr_rec.cc_no, local_rec.recWhNo);
	cc = find_rec (ccmr, &ccmr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, "ccmr", "DBFIND");
	strcpy (local_rec.recWhName, ccmr_rec.acronym);
	DSP_FLD ("recWhNo");
	DSP_FLD ("recWhName");

	local_rec.cre_dte = pcwo_rec.create_date;
	DSP_FLD ("cre_dte");

	local_rec.priority = pcwo_rec.priority;
	DSP_FLD ("priority");

	abc_selfield (inmr, "inmr_hhbr_hash");
	inmr_rec.hhbr_hash	=	pcwo_rec.hhbr_hash;
	cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, inmr, "DBFIND");

	strcpy (local_rec.head_item, inmr_rec.item_no);
	sprintf (local_rec.strength, "%-5.5s", inmr_rec.description + 35);
	sprintf (local_rec.head_desc, "%-35.35s", inmr_rec.description);
	sprintf (local_rec.head_desc2, "%-40.40s", inmr_rec.description2);
	local_rec.hhbr_hash = inmr_rec.hhbr_hash;
	DSP_FLD ("head_item");
	DSP_FLD ("strength");
	DSP_FLD ("head_desc");
	DSP_FLD ("head_desc2");
	abc_selfield (inmr, "inmr_id_no");

	inum_rec.hhum_hash	=	inmr_rec.std_uom;
	cc = find_rec (inum, &inum_rec, EQUAL, "r");
	strcpy (local_rec.head_std_uom, (cc) ?"   " :inum_rec.uom);

	inum_rec.hhum_hash	=	inmr_rec.alt_uom;
	cc = find_rec (inum, &inum_rec, EQUAL, "r");
	strcpy (local_rec.head_alt_uom, (cc) ?"   " :inum_rec.uom);

	strcpy (SR.UOM, local_rec.head_std_uom);
	DSP_FLD ("head_std_uom");
	DSP_FLD ("head_alt_uom");

	pcms_rec.hhwo_hash = pcwo_rec.hhwo_hash;
	pcms_rec.uniq_id = 0;
	cc = find_rec (pcms, &pcms_rec, GTEQ, "r");
	if (cc || pcms_rec.hhwo_hash != pcwo_rec.hhwo_hash)
	{
		memset (&pcms_rec, 0, sizeof (pcms_rec));
		return;
	}
	local_rec.rqd_dte = pcwo_rec.reqd_date;
	DSP_FLD ("rqd_dte");

	local_rec.qty_rqd = n_dec (pcwo_rec.prod_qty, local_rec.dec_pt);
	DSP_FLD ("qty_rqd");

	entry_exit = TRUE;

	pcln_rec.hhwo_hash	= pcwo_rec.hhwo_hash;
	pcln_rec.seq_no	= pcwo_rec.rtg_seq;
	pcln_rec.line_no	= 0;
	cc = find_rec (pcln, &pcln_rec, GTEQ, "r");
	if (!cc)
	{
		pcwc_rec.hhwc_hash	=	pcln_rec.hhwc_hash;
		cc = find_rec (pcwc, &pcwc_rec, EQUAL, "r");
		if (cc)
			memset (&pcwc_rec, 0, sizeof (pcwc_rec));
	}
 }

/*-----------------------------------
| Check if serial number has been   |
| allocated to this product already |
-----------------------------------*/
int
CheckSerialNo (
 void)
{
	int	i;
	struct	SER_NO	*lcl_ptr;

	for (i = 0; i < lcount [DETAIL_SCN]; i++)
	{
	    if (!strcmp (store [i].itemNumber, local_rec.itemNumber))
	    {
			lcl_ptr = HD_SR (i);
			while (lcl_ptr != SER_NULL)
			{
				if (!strcmp (lcl_ptr->serialNumber, local_rec.serialNumber))
					return (FALSE);

				lcl_ptr = lcl_ptr->next;
			}
	    }
	}

	for (i = 0; i < lcount [SERIAL_SCN]; i++)
	{
	    if (!strcmp (store [i].validSerial, local_rec.serialNumber) && i != line_cnt)
		return (FALSE);
	}

	return (TRUE);
}

int 	
CheckLotLocations (
 void)
{
	char	ErrMessStr [256];

	for (line_cnt = 0; line_cnt < lcount [DETAIL_SCN]; line_cnt++)
	{
		getval (line_cnt);
		if (SERIAL || FLD ("LL") == ND || NONSTOCK (line_cnt))
			continue;

		if (local_rec.LL [0] == 'N')
		{
			strcpy (ErrMessStr, ML ("Loc / Location quantity allocation required."));
			print_mess (err_str);
			startEdit	=	line_cnt;
			sleep (sleepTime);
			return (TRUE);
		}
	}
	return (FALSE);
}

/*------------------------------
| Get serial numbers for items |
| being issued or returned.    |
------------------------------*/
float	
GetSerialNo (
 void)
{
	int	i;

	struct	SER_NO
			*lcl_ptr,
			*currentSerial = (struct SER_NO *) 0,
			*SerialAllocate (void);

	tab_col = SERIAL_TABCOL;
	screenTwoLine = line_cnt;

	scn_set (SERIAL_SCN);
	init_vars (SERIAL_SCN);
	lcount [SERIAL_SCN] = 0;

	/*---------------------------------
	| Load serial numbers for product |
	---------------------------------*/
	if (HD_SR (line_cnt) != SER_NULL)
	{
		lcl_ptr = HD_SR (line_cnt);
		while (lcl_ptr != SER_NULL)
		{
			sprintf (local_rec.serialNumber, "%-25.25s", lcl_ptr->serialNumber);
			sprintf (local_rec.ser_loc, "%-10.10s", lcl_ptr->location);

			sprintf (store [lcount [SERIAL_SCN]].validSerial, "%-25.25s", lcl_ptr->serialNumber);

			putval (lcount [SERIAL_SCN]++);

			lcl_ptr = lcl_ptr->next;
		}
	}

	heading (SERIAL_SCN);
	scn_display (SERIAL_SCN);
	edit (SERIAL_SCN);

	if (restart)
	{
		strcpy (local_rec.serialNumber, serialSpace);
		strncpy (local_rec.ser_loc, serialSpace, 10);
	}

	/*-----------------------
	| Clear prompt on scn 3 |
	-----------------------*/
	if (line_cnt > 0)
		line_cnt--;
	screenThreeLine = line_cnt;
	DSP_FLD ("serialNumber");
	DSP_FLD ("ser_loc");

	line_cnt = screenTwoLine;
	tab_col = 2;
	if (restart)
	{
		restart = FALSE;
		return (-1.00);
	}

	/*----------------------
	| Store in linked list |
	----------------------*/
	SerialDealloc (HD_SR (line_cnt));
	HD_SR (line_cnt) = SER_NULL;
	if (lcount [SERIAL_SCN] != 0)
	{
	    for (i = 0; i < lcount [SERIAL_SCN]; i++)
	    {
			getval (i);
			lcl_ptr = SerialAllocate ();

			sprintf (lcl_ptr->serialNumber, "%-25.25s", local_rec.serialNumber);
			sprintf (lcl_ptr->location, "%-10.10s", local_rec.ser_loc);

			if (HD_SR (line_cnt) == SER_NULL)
		    	HD_SR (line_cnt) = lcl_ptr;
			else
		    	currentSerial->next = lcl_ptr;

			currentSerial = lcl_ptr;
	    }
	}

	return ((float)lcount [SERIAL_SCN]);
}

void
InitStore (
 void)
{
	SR.mabrHash 	= 0L;
	strcpy (SR.oldQtyDone, "N");
	strcpy (SR.qtyDone, "N");
	strcpy (SR.cons, "N");
	sprintf (SR.creditAccount, "%-*.*s", MAXLEVEL,MAXLEVEL,"                ");
	SR .creditHash 	= 0L;
	SR .amountIssued 	= 0.00;
	SR .amountReceipted 	= 0.00;
	SR .waistPercent 		= 0.00;
	SR .altNumber 		= 1;
	SR .instrNumber 	= 0;
	SR .issueSequence 	= 0;
	SR .uniqueId 	= 0;
	SR .head_ser 	= SER_NULL;
}

int
DeleteLine (
 void)
{
	int	i = 0;
	int	this_page = line_cnt / TABLINES;
	/*-------
	| entry	|
	-------*/
	if (prog_status == ENTRY)
	{
		print_mess (ML (mlStdMess005));
		return (EXIT_FAILURE);
	}

	getval (line_cnt);

	/*-------------------------------
	| blank last line - if required	|
	-------------------------------*/
	if (line_cnt / TABLINES == this_page)
		blank_display ();

	/*---------------------------
	| zap buffer if deleted all	|
	---------------------------*/
	if (lcount [DETAIL_SCN] <= 0)
	{
		init_vars (DETAIL_SCN);
		putval (i);
	}
	line_cnt = i;
	getval (line_cnt);
	return (EXIT_SUCCESS);
}

int
DeleteSerialLine (
 void)
{
	int	i;
	int	this_page = line_cnt / TABLINES;
	/*-------
	| entry	|
	-------*/
	if (prog_status == ENTRY)
	{
		print_mess (ML (mlStdMess005));
		return (EXIT_FAILURE);
	}

	/*--------------
	| delete lines |
	--------------*/
	lcount [SERIAL_SCN]--;
	for (i = line_cnt;line_cnt < lcount [SERIAL_SCN];line_cnt++)
	{
		getval (line_cnt + 1);
		putval (line_cnt);

		strcpy (store [line_cnt].validSerial, store [line_cnt + 1].validSerial);

		if (line_cnt / TABLINES == this_page)
			line_display ();
	}
	/*-------------------------------
	| blank last line - if required	|
	-------------------------------*/
	if (line_cnt / TABLINES == this_page)
		blank_display ();

	/*---------------------------
	| zap buffer if deleted all	|
	---------------------------*/
	if (lcount [SERIAL_SCN] <= 0)
	{
		init_vars (SERIAL_SCN);
		putval (i);
	}
	line_cnt = i;
	getval (line_cnt);
	return (EXIT_SUCCESS);
}

void
tab_other (
 int line_no)
{
	int	i;

	if (cur_screen != 2)
		return;

	/*----------------------
	| Display page details |
	----------------------*/
	i = (line_no / TABLINES) + 1;
	print_at (8, 88, ML (mlStdMess110), i);
	i = ((lcount [DETAIL_SCN] - 1) / TABLINES) + 1;
	print_at (8, 98, "of %2d", i);

	/*-------------------------------
	| Enable/Disable qtyIssue field. |
	-------------------------------*/
	if (local_rec.edit_ok)
		FLD ("qtyIssue") = NO;
	else
		FLD ("qtyIssue") = NA;

	/*-----------------------------------------
	| if return program, no new items allowed |
	-----------------------------------------*/
	if (pcReturn && (numberLines -1) < line_no)
	{
		FLD ("qtyIssue") = NA;
		FLD ("qtyDone") = NA;
	}
	
	/*--------------------------------
	| Enable/Disable qtyDone field. |
	--------------------------------*/
	if (!pcReturn)
	{
		FLD ("qtyDone") = YES;
		if (line_no <= (numberLines - 1))
		{
			FLD ("itemNumber") = NA;
			FLD ("uom") = NA;
			if (local_rec.changeQtyDone [0] == 'N')
				FLD ("qtyDone") = NA;
		}
		else
		{
			FLD ("itemNumber") = NO;
			FLD ("uom") = NO;
		}
	}

	/*-------------------------------------
	| Display serial numbers if relevant. |
	-------------------------------------*/
	if (line_no > lcount [DETAIL_SCN] ||
	   (line_no == lcount [DETAIL_SCN] && prog_status != ENTRY))
	{
		print_at (12, 18, ML (mlPcMess011), "");

		if (lastWasSerial)
			ShowInsf (TRUE, line_no);

		lastWasSerial = FALSE;
		lastLine = line_no;
		return;
	}

	if (line_no < lcount [DETAIL_SCN] ||
	   (line_no == lcount [DETAIL_SCN] && prog_status == ENTRY))
	{
		print_at (12, 18, ML (mlPcMess011), store [line_no].desc);

		if (local_rec.serial_item [0] == 'Y')
		{
			ShowInsf (FALSE, line_no);
			lastWasSerial = TRUE;
		}
		else if (lastWasSerial)
		{
			ShowInsf (TRUE, line_no);
			lastWasSerial = FALSE;
		}
	}
	lastLine = line_no;

	return;
}

/*----------------------------
| Display all serial numbers |
----------------------------*/
int
ShowInsf (
 int blnk_scn, 
 int line_no)
{
	struct	SER_NO	*lcl_ptr;

	if ((prog_status == ENTRY && !blnk_scn) || lastLine == line_no)
		return (EXIT_SUCCESS);

	tab_col = SERIAL_TABCOL;
	scn_set (SERIAL_SCN);

	init_vars (SERIAL_SCN);
	lcount [SERIAL_SCN] = 0;

	/*---------------------------------
	| Load serial numbers and display |
	---------------------------------*/
	if (!blnk_scn && HD_SR (line_no) != SER_NULL)
	{
		lcl_ptr = HD_SR (line_no);
		while (lcl_ptr != SER_NULL)
		{
			sprintf (local_rec.serialNumber, "%-25.25s", lcl_ptr->serialNumber);
			sprintf (local_rec.ser_loc, "%-10.10s", lcl_ptr->location);

			putval (lcount [SERIAL_SCN]++);

			lcl_ptr = lcl_ptr->next;
		}
	}

	scn_write (SERIAL_SCN);
	scn_display (SERIAL_SCN);

	tab_col = 2;
	scn_set (DETAIL_SCN);
	return (EXIT_FAILURE);
}

int
LoadPcms (void)
{
	int		i;
	int		first_no, 
			last_yes, 
			all_ok = FALSE;
	struct	SER_NO *SerialAllocate (void);

	abc_selfield (inmr, "inmr_hhbr_hash");

	pcms_rec.hhwo_hash = pcwo_rec.hhwo_hash;
	pcms_rec.uniq_id = 0;
	cc = find_rec (pcms, &pcms_rec, GTEQ, "r");
	while (!cc && pcms_rec.hhwo_hash == pcwo_rec.hhwo_hash)
	{
		inmr_rec.hhbr_hash	=	pcms_rec.mabr_hash;
	    cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
	    if (cc)
			file_err (cc, inmr, "DBFIND");

	    local_rec.dec_pt = inmr_rec.dec_pt;
	    strcpy (store [lcount [DETAIL_SCN]].itemNumber, 	inmr_rec.item_no);
	    strcpy (store [lcount [DETAIL_SCN]].itemClass, 	inmr_rec.inmr_class);
	    strcpy (local_rec.itemNumber, inmr_rec.item_no);
	    strcpy (store [lcount [DETAIL_SCN]].desc, inmr_rec.description);
	    strcpy (store [lcount [DETAIL_SCN]].lotControl, inmr_rec.lot_ctrl);
	    strcpy (local_rec.serial_item, inmr_rec.serial_item);
		strcpy (local_rec.qcReqd, inmr_rec.qc_reqd);

		ReadDefault (inmr_rec.category);

	    if (GetAccounts (inmr_rec.category,	lcount [DETAIL_SCN]))
			return (EXIT_FAILURE);

	    /*-----------------------------------
	    | Get the UOM conversion factor	|
	    -----------------------------------*/
	    store [lcount [DETAIL_SCN]].hhumHash = pcms_rec.uom;
	    GetInum (lcount [DETAIL_SCN]);

	    store [lcount [DETAIL_SCN]].hhwhHash = 0L;

	    incc_rec.hhcc_hash = currentHhcc;
	    incc_rec.hhbr_hash = (inmr_rec.hhsi_hash == 0L) ? inmr_rec.hhbr_hash : inmr_rec.hhsi_hash;
	    cc = find_rec (incc, &incc_rec, COMPARISON, "r");
	    if (!cc)
	    {
			store [lcount [DETAIL_SCN]].hhwhHash = incc_rec.hhwh_hash;
			store [lcount [DETAIL_SCN]].hhccHash = incc_rec.hhcc_hash;
			local_rec.edit_ok = TRUE;
	    }
	    else
	    {
			store [lcount [DETAIL_SCN]].hhwhHash = 0L;
			local_rec.edit_ok = FALSE;
	    }

	    /*--------------------------------
	    | Remaining Details For Tab Line |
	    --------------------------------*/
	    local_rec.act_qty_tot = n_dec (pcms_rec.matl_qty, local_rec.dec_pt);
	    local_rec.qty_tot = pcms_rec.matl_qty;
	    local_rec.qty_prv = n_dec (pcms_rec.qty_issued, local_rec.dec_pt);

	    store [lcount [DETAIL_SCN]].mabrHash		= pcms_rec.mabr_hash;
	    store [lcount [DETAIL_SCN]].lineNumber 		= lcount [DETAIL_SCN];
	    sprintf (store [lcount [DETAIL_SCN]].oldQtyDone, "%-1.1s", pcms_rec.act_qty_in);
	    strcpy (store [lcount [DETAIL_SCN]].cons, pcms_rec.cons);
	    store [lcount [DETAIL_SCN]].amountIssued	= pcms_rec.amt_issued;
	    store [lcount [DETAIL_SCN]].amountReceipted	= pcms_rec.amt_recptd;
	    store [lcount [DETAIL_SCN]].waistPercent	= pcms_rec.matl_wst_pc;

	    /*----------------------------------------
	    | Don't forget to include the wastage %. |
	    ----------------------------------------*/
	    pcms_rec.matl_wst_pc	+= 100.00;
	    pcms_rec.matl_wst_pc	/= 100.00;
	    local_rec.qty_tot		*= (double) pcms_rec.matl_wst_pc;
		local_rec.qty_tot = n_dec (local_rec.qty_tot, local_rec.dec_pt);
	    pcms_rec.matl_wst_pc	= store [lcount [DETAIL_SCN]].waistPercent;
	    local_rec.qtyIssue = 0.00;
	    if (pcms_rec.act_qty_in [0] == 'Y')
	    {
			strcpy (local_rec.qtyDone, "Yes");
			strcpy (local_rec.changeQtyDone, "N");
	    }
	    else
	    {
			strcpy (local_rec.changeQtyDone, "Y");
			if (inmr_rec.serial_item [0] == 'Y' || inmr_rec.lot_ctrl [0] == 'Y')
				strcpy (local_rec.qtyDone, "No");
			else
			{
				if (pcReturn)
					strcpy (local_rec.qtyDone, "No");
				else
				{
					strcpy (local_rec.qtyDone, "Yes");
					local_rec.qtyIssue = n_dec (local_rec.qty_tot, 
						local_rec.dec_pt) - n_dec (local_rec.qty_prv, 
						local_rec.dec_pt);
				}
			}
	    }
	    sprintf (store [lcount [DETAIL_SCN]].qtyDone, "%-1.1s", local_rec.qtyDone);

	    store [lcount [DETAIL_SCN]].quantity		= (float)local_rec.qtyIssue;
	    store [lcount [DETAIL_SCN]].altNumber		= pcms_rec.alt_no;
	    store [lcount [DETAIL_SCN]].instrNumber		= pcms_rec.instr_no;
	    store [lcount [DETAIL_SCN]].issueSequence	= pcms_rec.iss_seq;
	    store [lcount [DETAIL_SCN]].uniqueId		= pcms_rec.uniq_id;
	    if (HD_SR (lcount [DETAIL_SCN]) != SER_NULL)
	    {
			SerialDealloc (HD_SR (lcount [DETAIL_SCN]));
			HD_SR (lcount [DETAIL_SCN]) = SER_NULL;
	    }

		if (SK_BATCH_CONT || MULT_LOC)
		{
			cc =	DisplayLL
					(										
						lcount [DETAIL_SCN],							
						tab_row + 3 + (line_cnt % TABLINES),
						tab_col + 22,						
						4,									
						store [lcount [DETAIL_SCN]].hhwhHash,
						store [lcount [DETAIL_SCN]].hhumHash,
						store [lcount [DETAIL_SCN]].hhccHash,
						store [lcount [DETAIL_SCN]].UOM,
						store [lcount [DETAIL_SCN]].quantity,
						store [lcount [DETAIL_SCN]].cnvFct,
						TodaysDate (),
						TRUE,
						FALSE,
						store [lcount [DETAIL_SCN]].lotControl
					);
			strcpy (local_rec.LL, (cc) ? "N" : "Y");
		}
	    putval (lcount [DETAIL_SCN]++);

	    oldLines = lcount [DETAIL_SCN];

	    cc = find_rec (pcms, &pcms_rec, NEXT, "r");
	}
	numberLines = lcount [DETAIL_SCN];

	/*--------------------------------------------
	| Put lines in order based on qtyDone field |
	| All 'Y's come before all 'N's              |
	--------------------------------------------*/
	while (!all_ok)
	{
	    all_ok = TRUE;
	    first_no = lcount [DETAIL_SCN];
	    last_yes = -1;

	    /*----------------
	    | Find first 'N' |
	    ----------------*/
	    for (i = 0; i < lcount [DETAIL_SCN]; i++)
	    {
			if (store [i].oldQtyDone [0] == 'N')
			{
				first_no = i;
				break;
			}
	    }

	    /*---------------
	    | Find last 'Y' |
	    ---------------*/
	    for (i = lcount [DETAIL_SCN] - 1; i >= 0; i--)
	    {
			if (store [i].oldQtyDone [0] == 'Y')
			{
				last_yes = i;
				break;
			}
	    }

	    /*-----------------------------------------------------------
	    | OK... We have a mismatch... Let's reorder the lines.		|
	    -----------------------------------------------------------*/
	    if (last_yes > first_no && first_no >= 0 && last_yes < lcount [DETAIL_SCN])
	    {
			/*------------------------------
			| Swap lines in tabular screen |
			------------------------------*/
			getval (last_yes);
			putval (-1);
			getval (first_no);
			putval (last_yes);
			getval (-1);
			putval (first_no);

			/*---------------------------
			| Swap lines in store array |
			---------------------------*/
			memcpy 
			(
				(char *) &tmp_store, 
				(char *) &store [last_yes], 
			   	sizeof (struct storeRec)
			);
			memcpy 
			(
				(char *) &store [last_yes], 
				(char *) &store [first_no], 
				sizeof (struct storeRec)
			);
			memcpy 
			(
				(char *) &store [first_no], 
				(char *) &tmp_store, 
				sizeof (struct storeRec)
			);
			all_ok = FALSE;
	    }
	}

	/*--------------------------
	| Find start line for edit |
	--------------------------*/
	startEdit = 0;
	for (i = 0; i < lcount [DETAIL_SCN]; i++)
	{
	    if (store [i].oldQtyDone [0] == 'N')
	    {
			startEdit = i;
			break;
	    }
	}
	return (EXIT_SUCCESS);
}

void
GetInum (
 int line_cnt)

{
	/*-------------------------------
	| Get the UOM conversion factor	|
	-------------------------------*/
	inum_rec.hhum_hash	=	inmr_rec.alt_uom;
	cc = find_rec (inum, &inum_rec, EQUAL, "r");
	if (cc)
		file_err (cc, "inum", "DBFIND");

	sprintf (local_rec.alt_group, "%-20.20s", inum_rec.uom_group);
	local_rec.alt_cnvFct = inum_rec.cnv_fct;

	inum_rec.hhum_hash	=	inmr_rec.std_uom;
	cc = find_rec (inum, &inum_rec, EQUAL, "r");
	if (cc)
		file_err (cc, "inum", "DBFIND");

	strcpy (SR.UOM, inum_rec.uom);
	local_rec.outer_size = inmr_rec.outer_size;
	strcpy (local_rec.std_uom, inum_rec.uom);
	sprintf (local_rec.std_group, "%-20.20s", inum_rec.uom_group);
	local_rec.std_cnvFct = inum_rec.cnv_fct;

	inum_rec.hhum_hash	=	SR.hhumHash;
	cc = find_rec (inum, &inum_rec, EQUAL, "r");
	if (cc)
		file_err (cc, "inum", "DBFIND");

	CalcConv (line_cnt);
}

int
CalcConv (
 int line_cnt)
{
	number	std_cnvFct;
	number	alt_cnvFct;
	number	cnvFct;
	number	result;
	number	uom_cfactor;


	/*------------------------------------------------
	| converts a float to arbitrary precision number |
	| defined as number.                             |
	------------------------------------------------*/
	NumFlt (&std_cnvFct, local_rec.std_cnvFct);
	NumFlt (&alt_cnvFct, local_rec.alt_cnvFct);
	NumFlt (&cnvFct, inum_rec.cnv_fct);

	/*----------------------------------------------------------
	| a function that divides one number by another and places |
	| the result in another number defined variable            |
	| Conversion factor = std uom cnvFct / iss uom cnvFct    |
	|      OR                                                  |
	| Conversion factor = (std uom cnvFct / iss uom cnvFct)  |
	|                     * item's conversion factor           |
	| Same calculations as in pc_recprt.                       |
	----------------------------------------------------------*/
	if (strcmp (local_rec.alt_group, inum_rec.uom_group))
		NumDiv (&std_cnvFct, &cnvFct, &result);
	else
	{
		NumFlt (&uom_cfactor, inmr_rec.uom_cfactor);
		NumDiv (&alt_cnvFct, &cnvFct, &result);
		NumMul (&result, &uom_cfactor, &result);
	}

	strcpy (local_rec.iss_uom, inum_rec.uom);
	SR.hhumHash = inum_rec.hhum_hash;

	/*---------------------------------------
	| converts a arbitrary precision number |
	| to a float                            |
	---------------------------------------*/
	SR.cnvFct = NumToFlt (&result);

	return (EXIT_SUCCESS);
}

/*---------------------
| Update pcms records |
---------------------*/
int
Update (
 void)
{
	char	pipe_name [31];

	fprintf (pp, "| WORKS ORDER (%-7.7s) /  BATCH (%-10.10s)  /  ITEM : %-16.16s - %-40.40s             ",
			local_rec.order_no,
			local_rec.batch_no,
			local_rec.head_item,
			local_rec.head_desc);

	if (UPD_GL)
		fprintf (pp, "                 ");
	fprintf (pp, "           |\n");

	/*-----------------------------
	| Delete ALL old pcms records |
	| for the current hhwo hash   |
	-----------------------------*/
	pcms_rec.hhwo_hash = pcwo_rec.hhwo_hash;
	pcms_rec.uniq_id = 0;
	cc = find_rec (pcms, &pcms_rec, GTEQ, "u");
	while (!cc && pcms_rec.hhwo_hash == pcwo_rec.hhwo_hash)
	{
		cc = abc_delete (pcms);
		if (cc)
			file_err (cc, "pcms", "DBDELETE");

		pcms_rec.hhwo_hash = pcwo_rec.hhwo_hash;
		pcms_rec.uniq_id = 0;
		cc = find_rec (pcms, &pcms_rec, GTEQ, "u");
	}
	abc_unlock (pcms);

	/*------------------
	| Add pcms records |
	------------------*/
	for (line_cnt = 0; line_cnt < lcount [DETAIL_SCN]; line_cnt++)
	{
		getval (line_cnt);

		if (pcReturn)
			local_rec.qtyIssue *= -1;

		if (line_cnt < numberLines)
			pcms_rec.uniq_id = SR.uniqueId;
		else
			pcms_rec.uniq_id = numberLines++;

		if (local_rec.qtyIssue != 0.00)
			DoTransaction (line_cnt);

		strcpy (pcms_rec.co_no, comm_rec.co_no);
		pcms_rec.hhbr_hash		= local_rec.hhbr_hash;
		pcms_rec.alt_no			= SR.altNumber;
		if (line_cnt < oldLines)
			pcms_rec.line_no	= (line_cnt < oldLines) ? SR.lineNumber : line_cnt;
		strcpy (pcms_rec.cons, SR.cons);
		pcms_rec.mabr_hash		= SR.mabrHash;
		pcms_rec.uom			= SR.hhumHash;
		pcms_rec.matl_qty	= (float)
								n_dec (local_rec.act_qty_tot, local_rec.dec_pt);
		pcms_rec.matl_wst_pc	= SR.waistPercent;
		pcms_rec.instr_no		= SR.instrNumber;
		pcms_rec.iss_seq		= SR.issueSequence;
		pcms_rec.qty_issued		= (float) (local_rec.qtyIssue + 
										   local_rec.qty_prv);
		strcpy (pcms_rec.act_qty_in, SR.qtyDone);
		pcms_rec.hhwo_hash		= pcwo_rec.hhwo_hash;
		pcms_rec.amt_recptd		= SR.amountReceipted;
		pcms_rec.amt_issued		= 	CENTS 
									(
										local_rec.qtyIssue * 
										local_rec.workCost / SR.cnvFct
									);
		pcms_rec.amt_issued		= 	out_cost 
									(
										pcms_rec.amt_issued,
								   		local_rec.outer_size
									);
		pcms_rec.amt_issued		+= SR.amountIssued;

		cc = abc_add (pcms, &pcms_rec);
		if (cc)
			file_err (cc, "pcms", "DBADD");

		/*-----------------------------------
		| add sobg record for recalculation |
		| of BOM committed qty              |
		-----------------------------------*/
		add_hash 
		(
			pcwo_rec.co_no,
			pcwo_rec.br_no, 
			"RC",
			0,
			pcms_rec.mabr_hash, 
			ccmr_rec.hhcc_hash, 
			0L, 
			(double) 0
		); /* decrease the committed qty of the mat. items */

		if (UPD_GL && local_rec.qtyIssue != 0.00)
			AddPcgl (line_cnt);
	}
	fprintf (pp, "|----------------!");
	fprintf (pp, "---------------------!");
	fprintf (pp, "-------------------------!");
	fprintf (pp, "-------!");
	fprintf (pp, "----------!");
	fprintf (pp, "-------------------!");
	fprintf (pp, "---------------!");
	fprintf (pp, "-------!");

	if (UPD_GL)
		fprintf (pp, "----------------!");
	fprintf (pp, "-----------|\n");

	/*-----------------
	| Call pc_chk_iss |
	-----------------*/
	if (CHK_ISSUE && DeltaNo ())
	{
		sprintf (pipe_name, "pc_chk_iss %2d 0 Y Y Y Y", local_rec.lpno);
		/*-------------------------
		| Open pipe to pc_chk_iss |
		-------------------------*/
		if ((pout = popen (pipe_name, "w")) == (FILE *) 0)
			sys_err ("Error in pc_chk_iss During (POPEN)", errno, PNAME);

		fprintf (pout, "%010ld\n", pcwo_rec.hhwo_hash);
		fflush (pout);
		pclose (pout);
	}

	return (EXIT_SUCCESS);
}

void
DoTransaction (
	int		line_cnt)
{
	int		i;
	int		NoLots		=	TRUE;
	int		pcsf_line_no;
	float	workQty		=	0.00,
			tranQty		=	0.00;
	struct	SER_NO	*ser_ptr;
	double	cur_cost	=	0.00, 
			old_qty 	= 	0.00, 
			xx_qty 		= 	0.00;

	inmr_rec.hhbr_hash	=	SR.mabrHash;
	cc = find_rec (inmr, &inmr_rec, EQUAL, "u");
	if (cc)
		file_err (cc, inmr, "DBFIND");

	incc_rec.hhcc_hash = SR.hhccHash;
	incc_rec.hhbr_hash = (inmr_rec.hhsi_hash == 0L) 
							? inmr_rec.hhbr_hash : inmr_rec.hhsi_hash;
	cc = find_rec (incc, &incc_rec, EQUAL, "u");
	if (cc)
		file_err (cc, incc, "DBFIND");

	if (SERIAL)
	{
		if (pcReturn)
			pcsf_line_no = 0;
		else
	    	pcsf_line_no = (int) local_rec.qty_prv;

	    local_rec.workCost = 0.00;
	    ser_ptr = SR.head_ser;
	    while (ser_ptr != SER_NULL)
	    {
			cur_cost	=	FindInsfCost
							(
								SR.hhwhHash, 
								0L,
								ser_ptr->serialNumber, 
								(pcReturn) ? "S" : "F"
							);
			if (cur_cost != -1.00)
			{
				local_rec.workCost += cur_cost;
				MoveAdd 
				(
					comm_rec.co_no, 
					comm_rec.est_no, 
					comm_rec.cc_no, 
					incc_rec.hhbr_hash, 
					SR.hhccHash,
					inmr_rec.std_uom,
					comm_rec.inv_date, 
					8, 
					"      ", 
					inmr_rec.inmr_class, 
					inmr_rec.category, 
					pcwo_rec.order_no, 
					pcwo_rec.batch_no, 
					(float) (pcReturn) ? -1.00 : 1.00, 
					0.00, 
					CENTS (cur_cost)
				);
				cc	= 	UpdateInsf 
						(
							SR.hhwhHash,
							0L,
							ser_ptr->serialNumber,
							(pcReturn) ? "S" : "F",
							(pcReturn) ? "F" : "S"
						);
				if (cc)
					file_err (cc, insf, "DBUPDATE");

				if (pcReturn)
				{
					/*---------------------
					| Delete pcsf record. |
					---------------------*/
					pcsf_rec.hhwo_hash = pcwo_rec.hhwo_hash;
					pcsf_rec.uniq_id = pcms_rec.uniq_id;
					pcsf_rec.line_no = pcsf_line_no;
					cc = find_rec (pcsf, &pcsf_rec, GTEQ, "r");
					while (!cc &&
						pcsf_rec.hhwo_hash == pcwo_rec.hhwo_hash &&
						pcsf_rec.uniq_id == pcms_rec.uniq_id)
					{
						if (!strcmp (pcsf_rec.serial_no, ser_ptr->serialNumber))
						{
							cc = abc_delete (pcsf);
							if (cc)
								file_err (cc, "pcsf", "DBDELETE");
							break;
						}
						cc = find_rec (pcsf, &pcsf_rec, NEXT, "r");
					}
				}
				else
				{
					/*---------------
					| Add pcsf recs |
					---------------*/
					pcsf_rec.hhwo_hash = pcwo_rec.hhwo_hash;
					pcsf_rec.uniq_id = pcms_rec.uniq_id;
					pcsf_rec.line_no = pcsf_line_no++;
					sprintf (pcsf_rec.serial_no,"%-25.25s", ser_ptr->serialNumber);
					sprintf (pcsf_rec.location,"%-25.25s",ser_ptr->location);

					cc = abc_add (pcsf, &pcsf_rec);
					if (cc)
						file_err (cc, "pcsf", "DBADD");
				}
			}
			else
				local_rec.qtyIssue -= 1.00;

			/*---------------------
			| Print an audit line |
			---------------------*/
			PrintDetails
			(
				line_cnt, 
			 	(float)	pcReturn ? (float) (0 - 1.00) : (float) 1.00,
				cur_cost, 
				" ", 
				ser_ptr->serialNumber, 
				ser_ptr->location, 
			    SR.cnvFct
			);

			ser_ptr = ser_ptr->next;
	    }
	    local_rec.workCost /= pcReturn ? 0 - local_rec.qtyIssue : local_rec.qtyIssue;
	}
	else
	{
		workQty	=	(float) (pcReturn) ? local_rec.qtyIssue * -1
									   : local_rec.qtyIssue;
		workQty /= SR.cnvFct;

	    switch (inmr_rec.costing_flag [0])
	    {
	    case 'A':
	    case 'P':
	    case 'T':
	    case 'L':
			local_rec.workCost	=	FindIneiCosts
									(
										inmr_rec.costing_flag,
										comm_rec.est_no,
										incc_rec.hhbr_hash
									);
		break;

	    case 'F':

			local_rec.workCost	=	
					FindIncfValue 
					(
						incc_rec.hhwh_hash, 
						(pcReturn) ? incc_rec.closing_stock + workQty  
								   : incc_rec.closing_stock, 
						workQty,
						TRUE,
						inmr_rec.dec_pt
					);
		break;

	    case 'I':
			local_rec.workCost = 
					FindIncfValue 
					(
						incc_rec.hhwh_hash, 
						(pcReturn) ? incc_rec.closing_stock + workQty  
								   : incc_rec.closing_stock, 
						workQty,
						FALSE,
						inmr_rec.dec_pt
					);
		break;
	    }
	}
	if (local_rec.workCost < 0.00)
	{
		local_rec.workCost	=	FindIneiCosts
								(
									"L",
									comm_rec.est_no,
									incc_rec.hhbr_hash
								);
	}
	local_rec.std_cost	=	FindIneiCosts
							(
								"T",
								comm_rec.est_no,
								incc_rec.hhbr_hash
							);

	NoLots = TRUE;
	if (!SERIAL && FLD ("LL") != ND && !NONSTOCK (line_cnt))
	{
		NoLots	=	TRUE;
		for (i = 0; i < MAX_LOTS; i++)
		{
			if (!LL_Valid (line_cnt, i))
				break;

			if (GetBaseQty (line_cnt, i) == 0.00)
				continue;

			NoLots		=	FALSE;
			tranQty		=	GetBaseQty (line_cnt, i);
			/*--------------------------
			| Log inventory movements. |
			--------------------------*/
			MoveAdd
			 (
				comm_rec.co_no, 
				comm_rec.est_no, 
				comm_rec.cc_no, 
				incc_rec.hhbr_hash, 
				SR.hhccHash,
				inmr_rec.std_uom,
				comm_rec.inv_date, 
				8, 
				GetLotNo (line_cnt, i),
				inmr_rec.inmr_class, 
				inmr_rec.category, 
				pcwo_rec.order_no, 
				pcwo_rec.batch_no, 
				(pcReturn) ? tranQty * -1 : tranQty,
				0.00, 
				CENTS (local_rec.workCost)
			);
			/*---------------------
			| Print an audit line |
			---------------------*/
			PrintDetails
		 	(
				line_cnt, 
				GetQty (line_cnt, i),
				local_rec.workCost, 
				GetLotNo (line_cnt, i),
				" ", 
				GetLoc (line_cnt, i),
				SR.cnvFct
			);

			strcpy (pclt_rec.co_no, pcwo_rec.co_no);
			strcpy (pclt_rec.br_no, pcwo_rec.br_no);
			pclt_rec.hhwo_hash 	= pcwo_rec.hhwo_hash;
			pclt_rec.hhbr_hash 	= SR.mabrHash;
			pclt_rec.issue_date = StringToDate (local_rec.systemDate);
			pclt_rec.qty_used 	= GetBaseQty (line_cnt, i);
			pclt_rec.iss_uom 	= SR.hhumHash;

			strcpy (pclt_rec.lot_number, 	GetLotNo	 (line_cnt, i));
			strcpy (pclt_rec.slot_no, 	 	GetSLotNo	 (line_cnt, i));
			strcpy (pclt_rec.lot_location, 	GetLoc 		 (line_cnt,i));
			/*----------------------------
			| Add W/O lots trace record. |
			----------------------------*/
			cc = abc_add (pclt, &pclt_rec);
			if (cc)
				file_err (cc, "pclt", "DBADD");
		}
		if (NoLots)
		{
			tranQty	=	(float) (local_rec.qtyIssue / SR.cnvFct);
			/*--------------------------
			| Log inventory movements. |
			--------------------------*/
			MoveAdd
			(
				comm_rec.co_no, 
				comm_rec.est_no, 
				comm_rec.cc_no, 
				incc_rec.hhbr_hash, 
				SR.hhccHash,
				inmr_rec.std_uom,
				comm_rec.inv_date, 
				8, 
			    "      ", 
				inmr_rec.inmr_class, 
				inmr_rec.category, 
				pcwo_rec.order_no, 
			    pcwo_rec.batch_no, 
				(pcReturn) ? tranQty * -1 : tranQty,
				0.00, 
			    CENTS (local_rec.workCost)
			);
			/*---------------------
			| Print an audit line |
			---------------------*/
			PrintDetails
			 (
				line_cnt, 
				 (float)local_rec.qtyIssue, 
				local_rec.workCost, 
				" ", 
				" ", 
				" ",
				SR.cnvFct
			);
		}
	}

	if (!SERIAL && (FLD ("LL") == ND || NONSTOCK (line_cnt)))
	{
		if (NoLots)
		{
			/*--------------------------
			| Log inventory movements. |
			--------------------------*/
		    tranQty	=	(float) (local_rec.qtyIssue /  SR.cnvFct);
			MoveAdd
			 (
				comm_rec.co_no, 
				comm_rec.est_no, 
				comm_rec.cc_no, 
				incc_rec.hhbr_hash, 
				SR.hhccHash,
				inmr_rec.std_uom,
				comm_rec.inv_date, 
				8, 
			    "      ", 
				inmr_rec.inmr_class, 
				inmr_rec.category, 
				pcwo_rec.order_no, 
			    pcwo_rec.batch_no, 
				(pcReturn) ? tranQty * -1 : tranQty,
				0.00, 
				CENTS (local_rec.workCost)
			);
			/*---------------------
			| Print an audit line |
			---------------------*/
			PrintDetails
			 (
				line_cnt, 
				 (float)local_rec.qtyIssue, 
				local_rec.workCost, 
				" ", 
				" ", 
				" ",
			    SR.cnvFct
			);
		}
	}

	if (!NONSTOCK (line_cnt))
	{
		incc_rec.issues		+= (float) (local_rec.qtyIssue / SR.cnvFct);
		incc_rec.ytd_issues	+= (float) (local_rec.qtyIssue / SR.cnvFct);
		old_qty				= incc_rec.closing_stock;
		xx_qty				= incc_rec.closing_stock;
		incc_rec.closing_stock = incc_rec.opening_stock
								+ incc_rec.pur
								+ incc_rec.receipts
								+ incc_rec.adj
								- incc_rec.issues
								- incc_rec.sales;
		cc = abc_update (incc, &incc_rec);
		if (cc)
			file_err (cc, incc, "DBUPDATE");

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
				file_err (cc, "inwu", "DBADD");

			inwu_rec.hhwh_hash	=	incc_rec.hhwh_hash;
			inwu_rec.hhum_hash	=	inmr_rec.std_uom;
			cc = find_rec (inwu, &inwu_rec, COMPARISON,"u");
			if (cc)
				file_err (cc, "inwu", "DBFIND");
		}
		inwu_rec.issues	+= (float) (local_rec.qtyIssue / SR.cnvFct);
		inwu_rec.closing_stock = inwu_rec.opening_stock +
								 inwu_rec.pur +
								 inwu_rec.receipts +
								 inwu_rec.adj -
								 inwu_rec.issues -
								 inwu_rec.sales;

		cc = abc_update (inwu,&inwu_rec);
		if (cc)
			file_err (cc, "inwu", "DBUPDATE");
	}
	else
		abc_unlock (incc);

	if ((MULT_LOC || SK_BATCH_CONT) && !NONSTOCK (line_cnt))
	{
		UpdateLotLocation 
		(
			line_cnt,
			(pcReturn) ? FALSE : TRUE
		);
	}
	if (!NONSTOCK (line_cnt))
	{
		ffdm_rec.hhbr_hash	=	incc_rec.hhbr_hash;
		ffdm_rec.hhcc_hash	=	incc_rec.hhcc_hash;
		strcpy (ffdm_rec.type, "6");
		ffdm_rec.date		=	local_rec.lsys_dte;
		cc = find_rec (ffdm, &ffdm_rec, COMPARISON, "r");
		if (cc)
		{
			ffdm_rec.qty	=	(float) (local_rec.qtyIssue / SR.cnvFct);
			cc = abc_add (ffdm, &ffdm_rec);
			if (cc)
				file_err (cc, ffdm, "DBADD");
		}
		else
		{
			ffdm_rec.qty	+=	(float) (local_rec.qtyIssue / SR.cnvFct);
			cc = abc_update (ffdm, &ffdm_rec);
			if (cc)
				file_err (cc, ffdm, "DBUPDATE");
		}
		inmr_rec.on_hand -= (float) (local_rec.qtyIssue / SR.cnvFct);
		cc = abc_update (inmr, &inmr_rec);
		if (cc)
			file_err (cc, inmr, "DBUPDATE");

	}
	else
		abc_unlock (inmr);
		
	SerialDealloc (SR.head_ser);
	SR.head_ser = SER_NULL;
}
/*======================================================================
| Check if any lines have changed status from Not complete to complete |
======================================================================*/
int
DeltaNo (
 void)
{
	int	i;

	for (i = 0; i < lcount [DETAIL_SCN]; i++)
	{
		if (store [i].oldQtyDone [0] == 'N' && store [i].qtyDone [0] == 'Y')
			return (TRUE);
	}
	return (FALSE);
}

/*===============
| Search on UOM |
===============*/
void
SrchInum (
 char *searchValue)
{
	work_open ();
	save_rec ("#UOM", "#Unit Description");
	abc_selfield (inum, "inum_id_no");

	strcpy (inum_rec.uom_group, local_rec.std_group);
	inum_rec.hhum_hash = 0L;
	cc = find_rec (inum, &inum_rec, GTEQ, "r");
	while
	(
	    !cc &&
	    !strcmp (inum_rec.uom_group, local_rec.std_group)
	)
	{
		if (strncmp (inum_rec.uom, searchValue, strlen (searchValue)))
		{
			cc = find_rec (inum, &inum_rec, NEXT, "r");
			continue;
		}

		if (!ValidItemUom (SR.mabrHash, inum_rec.hhum_hash))
		{
			cc = save_rec (inum_rec.uom, inum_rec.desc);
			if (cc)
			break;
		}
	    cc = find_rec (inum, &inum_rec, NEXT, "r");
	}

	if (strcmp (local_rec.std_group, local_rec.alt_group))
	{
	    strcpy (inum_rec.uom_group, local_rec.alt_group);
	    inum_rec.hhum_hash = 0L;
	    cc = find_rec (inum, &inum_rec, GTEQ, "r");
	    while (!cc && !strcmp (inum_rec.uom_group, local_rec.alt_group))
	    {
			if (strncmp (inum_rec.uom, searchValue, strlen (searchValue)))
			{
				cc = find_rec (inum, &inum_rec, NEXT, "r");
				continue;
			}

			if (!ValidItemUom (SR.mabrHash, inum_rec.hhum_hash))
			{
				cc = save_rec (inum_rec.uom, inum_rec.desc);
				if (cc)
					break;
			}

			cc = find_rec (inum, &inum_rec, NEXT, "r");
	    }
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	abc_selfield (inum, "inum_uom");
	sprintf (inum_rec.uom, "%-4.4s", temp_str);
	cc = find_rec (inum, &inum_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "inum", "DBFIND");
}

/*===================================
| Allocate a new list entry from	|
| either malloc OR from the head	|
| of the current freeSerial list.	|
===================================*/
struct	SER_NO	*
SerialAllocate (
 void)
{
	struct	SER_NO	*tempSerial;

	tempSerial = freeSerial;
	if (tempSerial != SER_NULL)
		freeSerial = freeSerial->next;
	else
		tempSerial = (struct SER_NO *) malloc (sizeof (struct SER_NO));

	while (tempSerial == SER_NULL)
	{
		sleep (sleepTime);
		tempSerial = (struct SER_NO *) malloc (sizeof (struct SER_NO));
	}

	tempSerial->next = SER_NULL;

	return (tempSerial);
}

/*===================================================
| Move ALL of head_ser list to the freeSerial list. |
===================================================*/
void
SerialDealloc (
 struct SER_NO *head_ser)
{
	struct	SER_NO	*tempSerial;

	while (head_ser != SER_NULL)
	{
		tempSerial = head_ser->next;
		head_ser->next = freeSerial;
		freeSerial = head_ser;
		head_ser = tempSerial;
	}
}

/*===================================================
| Release ALL RAM held in the freeSerial list.		|
===================================================*/
void
FreeSerial (
 void)
{
	struct	SER_NO	*tempSerial;

	while (freeSerial != SER_NULL)
	{
		tempSerial = freeSerial->next;
		free (freeSerial);
		freeSerial = tempSerial;
	}
}

/*===========================
| Process control Accounts. |
===========================*/
int
GetAccounts (
	char	*category, 
	int		linePosition)
{
	GL_GLI 
	(
		ccmr_rec.co_no,
		ccmr_rec.est_no,
		ccmr_rec.cc_no,
		"COSTSALE M",
		" ",
		category
	);
	strcpy (store [linePosition].creditAccount, glmrRec.acc_no);
	store [linePosition].creditHash = glmrRec.hhmr_hash;
	return (EXIT_SUCCESS);
}

/*================================
| Add transactions to pcgl file. |
================================*/
void
AddPcgl (
 int i)
{
	int		periodMonth;
	double	costDiff;
	char	type [2];

	strcpy (pcgl_rec.co_no, comm_rec.co_no);
	strcpy (pcgl_rec.tran_type, "19");
	sprintf (pcgl_rec.sys_ref, "%5.1d", comm_rec.term);
	pcgl_rec.tran_date = comm_rec.inv_date;
	DateToDMY (comm_rec.inv_date, NULL, &periodMonth, NULL);
	sprintf (pcgl_rec.period_no, "%02d", periodMonth);
	pcgl_rec.post_date = comm_rec.inv_date;
	sprintf (pcgl_rec.narrative, "%-20.20s", pcwo_rec.order_no);
	sprintf (pcgl_rec.user_ref, "%8.8s", pcwc_rec.work_cntr);
	strcpy (pcgl_rec.stat_flag, "2");

	if (pcReturn)
		local_rec.qtyIssue *= -1;

	/*-------------------------------------------------------
	| Post the std cost to the WIP Direct Material Account. |
	-------------------------------------------------------*/
	strcpy (pcgl_rec.acc_no, debitAccount);
	pcgl_rec.hhgl_hash = debitHash;
	pcgl_rec.amount = CENTS (local_rec.qtyIssue * local_rec.std_cost / SR.cnvFct);
	pcgl_rec.amount = out_cost (pcgl_rec.amount, local_rec.outer_size);
	strcpy (pcgl_rec.jnl_type, (pcReturn) ? "2" : "1");
	pcgl_rec.loc_amount	=	pcgl_rec.amount;
	pcgl_rec.exch_rate	=	1.00;
	strcpy (pcgl_rec.currency, localCurrency);

	if (pcgl_rec.amount != 0.00)
	{
		cc = abc_add (pcgl, &pcgl_rec);
		if (cc)
			file_err (cc, "pcgl", "DBADD");
	}

	/*-------------------------------------------------------
	| Post the act cost to the components stock account.    |
	-------------------------------------------------------*/
	strcpy (pcgl_rec.acc_no, store [i].creditAccount);
	pcgl_rec.hhgl_hash = store [i].creditHash;
	pcgl_rec.amount = CENTS (local_rec.qtyIssue * local_rec.workCost / SR.cnvFct);
	pcgl_rec.amount = out_cost (pcgl_rec.amount, local_rec.outer_size);
	strcpy (pcgl_rec.jnl_type, (pcReturn) ? "1" : "2");
	pcgl_rec.loc_amount	=	pcgl_rec.amount;
	pcgl_rec.exch_rate	=	1.00;
	strcpy (pcgl_rec.currency, localCurrency);

	if (pcgl_rec.amount != 0.00)
	{
		cc = abc_add (pcgl, &pcgl_rec);
		if (cc)
			file_err (cc, "pcgl", "DBADD");
	}

	/*-------------------------------------------------------
	| Post the act cost to the components stock account.    |
	| post the difference between the act cost and the std  |
	| cost to the MFG Variance Direct Material account.     |
	-------------------------------------------------------*/
	strcpy (pcgl_rec.acc_no, mfgAccount);
	pcgl_rec.hhgl_hash = mfgHash;
	costDiff = local_rec.workCost - local_rec.std_cost;
	if (costDiff == 0.00) /* No record needed if variance is zero. */
		return;
	if (costDiff > 0.00)
		strcpy (type, (pcReturn) ? "2" : "1");
	else
	{
		costDiff = 0 - costDiff;
		strcpy (type, (pcReturn) ? "1" : "2");
	}
	pcgl_rec.amount = CENTS (local_rec.qtyIssue * costDiff / SR.cnvFct);
	pcgl_rec.amount = out_cost (pcgl_rec.amount, local_rec.outer_size);
	strcpy (pcgl_rec.jnl_type, type);

	pcgl_rec.loc_amount	=	pcgl_rec.amount;
	pcgl_rec.exch_rate	=	1.00;
	strcpy (pcgl_rec.currency, localCurrency);

	cc = abc_add (pcgl, &pcgl_rec);
	if (cc)
		file_err (cc, "pcgl", "DBADD");
}

/*=========================================================================
| Routine to open output pipe to standard print to provide an audit trail |
| of events. This also sends the output straight to the spooler.          |
=========================================================================*/
void
OpenAudit (
 void)
{
	if ((pp = popen ("pformat", "w")) == 0) 
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);

	fprintf (pp, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (pp, ".SO\n");
	fprintf (pp, ".LP%d\n", local_rec.lpno);
	fprintf (pp, ".12\n");
	fprintf (pp, ".L158\n");
	if (!pcReturn)
		fprintf (pp, ".ESTOCK ISSUES FOR PRODUCTION\n");
	else
		fprintf (pp, ".ESTOCK RETURNS FROM PRODUCTION\n");

	fprintf (pp, ".B1\n");
	fprintf (pp, ".E%s as at %24.24s\n", clip (comm_rec.co_short), SystemTime ());

	fprintf (pp, ".B2\n");
	fprintf (pp, ".EBRANCH: %s         WAREHOUSE: %s\n", 
										clip (comm_rec.est_name), 
										clip (comm_rec.cc_name));

	fprintf (pp, ".R==================");
	fprintf (pp, "======================");
	fprintf (pp, "==========================");
	fprintf (pp, "========");
	fprintf (pp, "=================");
	fprintf (pp, "===================");
	fprintf (pp, "===========");
	fprintf (pp, "========");
	if (UPD_GL)
		fprintf (pp, "=================");
	fprintf (pp, "============\n");

	fprintf (pp, "==================");
	fprintf (pp, "======================");
	fprintf (pp, "==========================");
	fprintf (pp, "========");
	fprintf (pp, "=================");
	fprintf (pp, "===================");
	fprintf (pp, "===========");
	fprintf (pp, "========");
	if (UPD_GL)
		fprintf (pp, "=================");
	fprintf (pp, "============\n");

	fprintf (pp, "|  ITEM NUMBER   |");
	fprintf (pp, "  ITEM DESCRIPTION   |");
	fprintf (pp, "       SERIAL NUMBER     |");
	fprintf (pp, "  LOT  |");
	fprintf (pp, " LOCATION |");
	fprintf (pp, "      QUANTITY     |");
	fprintf (pp, "     @COST     |");
	fprintf (pp, " OUTER |");
	if (UPD_GL)
		fprintf (pp, " GENERAL LEDGER |");
	fprintf (pp, "  EXTENDED |\n");

	fprintf (pp, "|                |");
	fprintf (pp, "                     |");
	fprintf (pp, "                         |");
	fprintf (pp, "  NO.  |");
	fprintf (pp, "          |");
	if (!pcReturn)
		fprintf (pp, "       ISSUED      |");
	else
		fprintf (pp, "      RETURNED     |");
	fprintf (pp, "               |");
	fprintf (pp, "  SIZE |");
	if (UPD_GL)
		fprintf (pp, "     ACCOUNT    |");
	fprintf (pp, "  VALUE.   |\n");

	fprintf (pp, "|----------------+");
	fprintf (pp, "---------------------+");
	fprintf (pp, "-------------------------+");
	fprintf (pp, "-------+");
	fprintf (pp, "----------+");
	fprintf (pp, "-------------------+");
	fprintf (pp, "---------------+");
	fprintf (pp, "-------+");
	if (UPD_GL)
		fprintf (pp, "----------------+");
	fprintf (pp, "-----------|\n");

	fprintf (pp, ".PI12\n");

	batchTotal = 0.00;
}

/*==============================
| Print details of data input. |
==============================*/
void
PrintDetails (
	int		line_no,
	float	qtyIssue,
	double	cost,
	char	*lotNumber,
	char	*serialNumber,
	char	*location,
	float	cfactor)
{
	double	value = out_cost (cost, inmr_rec.outer_size);

	/*----------------
	| Printe Details |
	----------------*/
	fprintf (pp, "!%16.16s!",	inmr_rec.item_no);
	fprintf (pp, "%-21.21s!",	inmr_rec.description);
	fprintf (pp, "%25.25s!",	local_rec.serialNumber);
	fprintf (pp, "%7.7s!",  	lotNumber);
	fprintf (pp, "%-10.10s!",	location);
	fprintf (pp, "%14.6f/", 	pcReturn ? 0 - qtyIssue : qtyIssue);
	fprintf (pp, "%-4.4s!", 	local_rec.iss_uom);
	fprintf (pp, "%10.2f/", 	cost);
	fprintf (pp, "%-4.4s!", 	local_rec.std_uom);
	fprintf (pp, "%7.1f!", 		inmr_rec.outer_size);

	if (UPD_GL)
		fprintf (pp, "%-16.16s!", store [line_no].creditAccount);
	fprintf (pp, "%11.2f!\n",
			 ((pcReturn ? 0 - qtyIssue : qtyIssue) * value) / cfactor);
	fflush (pp);

	batchTotal += (double)
			 (((pcReturn ? 0 - qtyIssue : qtyIssue) * value) / cfactor);
}

/*===============================================
| Routine to close the audit trail output file. |
===============================================*/
void
CloseAudit (
 void)
{

	fprintf (pp, "| BATCH TOTAL    |");
	fprintf (pp, "                     |");
	fprintf (pp, "                         |");
	fprintf (pp, "       |");
	fprintf (pp, "          |");
	fprintf (pp, "                   |");
	fprintf (pp, "               |");
	fprintf (pp, "       |");

	if (UPD_GL)
		fprintf (pp, "                |");
	fprintf (pp, "%11.2f|\n", batchTotal);

	fprintf (pp, ".EOF\n");
	pclose (pp);
}


/*============================================
| Checks if the quantity entered by the user |
| valid quantity that can be saved to a      |
| float variable without any problems of     |
| losing figures after the decimal point.    |
| eg. if _dec_pt is 2 then the greatest      |
| quantity the user can enter is 99999.99    |
============================================*/
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
		print_at (23, 0, ML (mlPcMess083), _qty, compare [_dec_pt]);
		sleep (sleepTime);
		clear_mess ();
		return (FALSE);
	}

	return (TRUE);
}

/*==========================================
| Check whether uom is valid compared with |
| the dec_pt and the conversion factor.    |
| eg. std uom = kg     iss uom = gm        |
|     conv.fact = 1000 dec_pt = 2          |
|     issue 5 gm, converts to 0.005 kg     |
|     round to 2 dec_pt, new qty = 0.01 kg |
|     or 10gm                              |
|This is incorrect and not allowed.        |
==========================================*/
int
ValidUOM (
 void)
{
	long	numbers [7];
	
	numbers [0] = 1;
	numbers [1] = 10;
	numbers [2] = 100;
	numbers [3] = 1000;
	numbers [4] = 10000;
	numbers [5] = 100000;
	numbers [6] = 1000000;

	if (SR.cnvFct > numbers [local_rec.dec_pt])
	{
		print_at (23, 0, ML (mlPcMess010), local_rec.dec_pt);
		sleep (sleepTime);
		clear_mess ();
		return (FALSE);
	}
	return (TRUE);
}

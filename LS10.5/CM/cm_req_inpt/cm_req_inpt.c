/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: cm_req_inpt.c,v 5.4 2002/11/28 04:09:45 scott Exp $
|  Program Name  : (cm_req_inpt.c) 
|  Program Desc  : (Contract Management Requisition Entry)
|---------------------------------------------------------------------|
|  Author        : Campbell Mander | Date Written : 02/03/93          |
|---------------------------------------------------------------------|
| $Log: cm_req_inpt.c,v $
| Revision 5.4  2002/11/28 04:09:45  scott
| SC0053 - Platinum Logistics - LS10.5.2 2002-11-28
| Updated for changes in pricing - See S/C for Details
|
| Revision 5.3  2002/07/24 08:38:42  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.2  2002/07/03 04:21:41  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.1  2002/01/17 01:27:16  scott
| Updated as first phase of contract management cleanup on it's way to becoming
| useable for 3PL.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cm_req_inpt.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CM/cm_req_inpt/cm_req_inpt.c,v 5.4 2002/11/28 04:09:45 scott Exp $";

#define	FGN_CURR    (envDbMcurr && strcmp (cumr_rec.curr_code, envCurrCode))

#define	MAXSCNS		2
#define MAXWIDTH	170
#define	MAXLINES	300
#define	TABLINES	9

#define	SCN_HEADER		1
#define	SCN_ITEMS		2

#include <pslscr.h>
#include <twodec.h>
#include <proc_sobg.h>
#include <ml_std_mess.h>
#include <ml_cm_mess.h>
#include <Costing.h>

#define	PRMPT_LINE	2

#define	MANUAL		0
#define	BRANCH		1
#define	COMPANY		2

#define	NORMAL		0
#define	DISP_ONLY	1

#define	MAX_FLDS	100

#define	SR		store [line_cnt]

#define	NO_COST		(SR.itemClass [0] == 'N')
#define	NON_STOCK	(SR.itemClass [0] == 'Z')
#define SERIAL		(SR.serialFlag [0] == 'Y')
#define	KIT_ITEM	(SR.itemClass [0] == 'K' && prog_status == ENTRY)
#define	PHANTOM		(SR.itemClass [0] == 'P' && prog_status == ENTRY)
#define	BO_OK		((SR.bOrderFlag [0] == 'Y' || SR.bOrderFlag [0] == 'F') && \
						cumr_rec.bo_flag [0] == 'Y')

#define	FULL_BO		(SR.bOrderFlag [0] == 'F' && cumr_rec.bo_flag [0] == 'Y')

#define	PRINT_REQ	1

extern	int	SR_Y_POS;
extern	int	SR_X_POS;

FILE *	pout;

char	*twentyFiveSpaces 	= "                         ",
		*sixteenSpaces 		= "                ";


#include	"schema"

struct commRecord	comm_rec;
struct ccmrRecord	ccmr_rec;
struct cmcbRecord	cmcb_rec;
struct cmcmRecord	cmcm_rec;
struct cmrdRecord	cmrd_rec;
struct cmrhRecord	cmrh_rec;
struct cmrhRecord	cmrh2_rec;
struct cmhrRecord	cmhr_rec;
struct cmcdRecord	cmcd_rec;
struct comrRecord	comr_rec;
struct cumrRecord	cumr_rec;
struct cumrRecord	cumr2_rec;
struct esmrRecord	esmr_rec;
struct inccRecord	incc_rec;
struct inmrRecord	inmr_rec;
struct inumRecord	inum_rec;
struct inumRecord	inum2_rec;
struct soktRecord	sokt_rec;

	char	*data  = "data",
			*ccmr2 = "ccmr2",
			*cmcm2 = "cmcm2",
			*cmhr2 = "cmhr2",
			*cmrh2 = "cmrh2",
			*inmr2 = "inmr2",
			*inum2 = "inum2";

	struct storeRec {
		long	hhbrHash;
		long	hhsiHash;
		long	hhccHash;
		long	hhumHash;
		long	hhwhHash;

		char	category 		[12];
		char	sellGroup 		[7];
		char	costHeadDesc 	[41];
		char	itemDesc 		[41];
		char	itemClass 		[2];
		char	bOrderFlag 		[2];
		char	costingFlag 	[2];
		char	priceOverride 	[2];
		char	discOverride 	[2];
		char	UOM 			[5];
		char	lotControl 		[2];
		char	serialFlag 		[2];
		char	serialNo 		[26];
		char	oldStatFlag 	[2];
		char	statFlag 		[2];

		float	closingStock;
		float	qtyAvailable;
		float	weight;
		float	outerSize;
		float	qtyOrder;
		float	qtyBackorder;
		float	qtyIssue;
		float	discPc;
		float	regPc;			/* Regulatory percent.      		*/
		float	discA;			/* Discount percent A.      		*/
		float	discB;			/* Discount percent A.      		*/
		float	discC;			/* Discount percent A.      		*/
		float	cnvFct;			/* Conversion Factor.	      		*/
		float	calcDisc;
		float	dfltDisc;

		double	salePrice;		/* 	                 				*/
		double	calcSalePrice;	/*                 					*/
		double	actSalePrice;	/*                 					*/
		double	gSalePrice;		/* cmrd_gsale_price					*/
		double	dfltPrice;		/*                					*/
		double	costPrice;

		int		lotSelectFlag;
		int		pricingChk;		/* Set if pricing has been  		*/
								/* called for line.         		*/
		int		cumulative;		/* Cumulative 1 or 0				*/
		int		conPrice;		/* Contract price.					*/
		int		contStatus;		/* 0 = not contract line			*/
		int		decPt;
	} store [MAXLINES];

	int		fieldRequired [MAX_FLDS];

	int		envDbMcurr 			= FALSE,
			envCmAutoReq		= 2,
			envCmAutoCon		= 2,
			envMultLoc			= 0,
			envSoFwdAvl			= 0,
			envWinOk			= 0,
			envInpDisc			= 0,
			envSerialItemsOk	= FALSE,
			IN_special 			= FALSE,
			num_flds			= 0,
			wpipe_open 			= FALSE,
			np_fn				= 0,
			newReq				= 0,
			overrideManual		= 0,
			printerNo			= 1;

	char	envCurrCode 		[4],
			branchNo 			[3],
			reqBranchNo 		[3],
			previousCostHead 	[5],
			envLogname 		[15];

	char	*scn_desc [] = {
		" REQUISITION HEADER SCREEN.",
		" REQUISITION DETAIL SCREEN.",
		""
	};


	extern	int		TruePosition;
/*
 * Local & Screen Structures 
 */
struct {
	long	lsystemDate;
	long	requisitionNo;
	long	hhcmHash;
	long	hhccHash;

	char	dummy 			[11];
	char	systemDate 		[11];
	char	requisitionDesc [26];
	char	contractNo 		[7];
	char	contDesc 		[7][71];
	char	previousRef 	[7];
	char	lotControl 		[2];
	char	costHead 		[5];
	char	costHeadDesc 	[41];
	char	item_desc 		[41];
	char	wh_no 			[3];
	char	UOM 			[5];
	char	serialNo 		[26];
	char	LL 				[2];

	float	qtyOrder;
	float	qtyBackorder;

	double	workCost;
} local_rec;

static struct	var vars [] =
{
	{1, LIN, "requisitionNo",	 3,  2, LONGTYPE,
		"NNNNNN", "          ",
		"0", "0", "Requisition Number   ", " ",
		NE, NO,  JUSTRIGHT, "", "", (char *)&local_rec.requisitionNo},
	{1, LIN, "requisitionDesc",	 3,  35, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		"", "", "", " ",
		NA, NO,  JUSTLEFT, "", "", local_rec.requisitionDesc},
	{1, LIN, "contractNo",	 4,  2, CHARTYPE,
		"UUUUUU", "          ",
		" ", "0", "Contract Number      ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.contractNo},
	{1, LIN, "descr1",	 5,  2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "",  "Description          ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.contDesc [0]},
	{1, LIN, "descr2",	 6,  2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "",  "                     ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.contDesc [1]},
	{1, LIN, "descr3",	 7,  2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "",  "                     ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.contDesc [2]},
	{1, LIN, "descr4",	 8,  2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "",  "                     ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.contDesc [3]},
	{1, LIN, "descr5",	 9, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "",  "                     ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.contDesc [4]},
	{1, LIN, "descr6",	 10, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "",  "                     ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.contDesc [5]},
	{1, LIN, "descr7",	 11, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "",  "                     ", " ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.contDesc [6]},

	{1, LIN, "req_date",	 13, 2, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", local_rec.systemDate, "Requisition Date     ", "",
		YES, NO, JUSTRIGHT, "", "", (char *)&cmrh_rec.req_date},
	{1, LIN, "rqrd_date",	 13, 65, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", " ", "Date Required        ", "",
		YES, NO, JUSTRIGHT, "", "", (char *)&cmrh_rec.rqrd_date},
	{1, LIN, "req_by",	 14, 2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		" ", envLogname, "Requested By         ", "",
		YES, NO,  JUSTLEFT, "", "", cmrh_rec.req_by},
	{1, LIN, "rq_full_supply",14, 65, CHARTYPE,
		"U", "          ",
		" ", "N","Full Supply          ", "Enter Y(es) or N(o)",
		 NE, NO,  JUSTLEFT, "YN", "", cmrh_rec.full_supply},

	{1, LIN, "del_name",	 16,  2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", cmhr_rec.contact, "Delivery Name        ", " ",
		 YES, NO,  JUSTLEFT, "", "", cmrh_rec.del_name},
	{1, LIN, "del_addr1",	 17,  2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", cmhr_rec.adr1,    "Delivery Address     ", " ",
		 YES, NO,  JUSTLEFT, "", "", cmrh_rec.del_adr1},
	{1, LIN, "del_addr2",	 18,  2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", cmhr_rec.adr2, "                     ", " ",
		 YES, NO,  JUSTLEFT, "", "", cmrh_rec.del_adr2},
	{1, LIN, "del_addr3",	 19,  2, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", cmhr_rec.adr3, "                     ", " ",
		 YES, NO,  JUSTLEFT, "", "", cmrh_rec.del_adr3},
	{1, LIN, "add_int1",	 16,  65, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Extra Instruction 1  ", " ",
		 YES, NO,  JUSTLEFT, "", "", cmrh_rec.add_int1},
	{1, LIN, "add_int2",	 17,  65, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Extra Instruction 2  ", " ",
		 YES, NO,  JUSTLEFT, "", "", cmrh_rec.add_int2},
	{1, LIN, "add_int3",	 18,  65, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "Extra Instruction 3  ", " ",
		 YES, NO,  JUSTLEFT, "", "", cmrh_rec.add_int3},

	{2, TAB, "costhd",	MAXLINES, 2, CHARTYPE,
		"UUUU", "          ",
		" ", " ", "Costhead", "Enter Costhead.",
		YES, NO,  JUSTLEFT, "", "", local_rec.costHead},
	{2, TAB, "wh_no",	0, 0, CHARTYPE,
		"AA", "          ",
		" ", comm_rec.cc_no, "WH", "Enter warehouse number.",
		 YES, NO, JUSTRIGHT, "1", "99", local_rec.wh_no},
	{2, TAB, "item_no",		0, 0, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", "", "    Item no.    ", "Enter item number. Full search available.",
		YES, NO,  JUSTLEFT, "", "", inmr_rec.item_no},
	{2, TAB, "UOM",	 0, 0, CHARTYPE,
		"AAAA", "          ",
		" ", "", "UOM.", " Unit of Measure ",
		 YES, NO, JUSTLEFT, "", "", local_rec.UOM},
	{2, TAB, "qtyOrder",	 0,  0, FLOATTYPE,
		"NNNNN.NN", "          ",
		" ", "1.0", "Qty Ord.", "Enter order quantity.",
		YES, NO, JUSTRIGHT, "0.00", "99999.99", (char *) &local_rec.qtyOrder},
	{2, TAB, "qtyBackorder",	 0,  0, FLOATTYPE,
		"NNNNN.NN", "          ",
		" ", " ", "Qty B/O.", "Enter quantity backordered. ",
		YES, NO, JUSTRIGHT, "0.00", "99999.99", (char *) &local_rec.qtyBackorder},
	{2, TAB, "sale_price",		 0,  0, MONEYTYPE,
		"NNNNNNNNN.NN", "          ",
		" ", "0", " Sell Price ", "",
		NO, NO, JUSTRIGHT, "", "", (char *) &cmrd_rec.sale_price},
	{2, TAB, "disc_pc",		 0,  0, FLOATTYPE,
		"NNN.NN", "          ",
		" ", "", " Disc ", "",
		NO, NO, JUSTRIGHT, "", "", (char *) &cmrd_rec.disc_pc},
	{2, TAB, "lotControl",	 0,  0, CHARTYPE,
		"U", "          ",
		"", "", "", "",
		 ND, NO,  JUSTLEFT, "", "", local_rec.lotControl},
	{2, TAB, "LL", 	 0, 0, CHARTYPE, 
		"U", "          ", 
		" ", "N", "L", "Lot / Location Selection. <return> ", 
		 YES, NO,  JUSTLEFT, "", "", local_rec.LL}, 
	{2, TAB, "ln_status",	 0,  2, CHARTYPE,
		"U", "          ",
		" ", " ", "Status", "",
		 NA, NO,  JUSTLEFT, "", "", cmrd_rec.stat_flag},
	{2, TAB, "serialNo",	 0,  0, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUUUUUUU", "          ",
		" ", " ", "        Serial No        ", "Enter Serial Number, Search key available. ",
		 NA, NO,  JUSTLEFT, "", "", local_rec.serialNo},
	{2, TAB, "hhcmHash",	 0,  0, LONGTYPE,
		"NNNNNNNNNN", "          ",
		" ", " ", "", "",
		 ND, NO,  JUSTLEFT, "", "", (char *)&local_rec.hhcmHash},
	{2, TAB, "hhccHash",	 0,  0, LONGTYPE,
		"NNNNNNNNNN", "          ",
		" ", " ", "", "",
		 ND, NO,  JUSTLEFT, "", "", (char *)&local_rec.hhccHash},

	{0, LIN, "",		 0,  0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}
};

#include <cus_price.h>
#include <cus_disc.h>
#include <LocHeader.h>

char	envSoOverrideQty [2];

#ifdef GVISION
#	include <StockWindow.h>
#endif	/* GVISION */

/*
 * Local function prototypes 
 */
float	PhantomSOH		(long);
float	ProcessPhantom	(long, float);
float	ToLclUom		(float);
float	ToStdUom		(float);
int		Addincc			(long);
int		CheckReqNumber	(long);
int		CheckSupInsf	(char *, long, int);
int		DeleteLine		(void);
int		FindCost		(void);
int		heading			(int);
int		InputRes		(void);
int		LoadItems		(void);
int		LoadReq		 	(void);
int		NoneOnBackorder	(void);
int		NotInContract	(void);
int		OpenSkWin		(void);
int		SaveRequired	(void);
int		SetRequired		(int);
int		SetStatus		(int);
int		spec_valid		(int);
int		Update			(void);
int		ValidItemNo		(void);
long	GenReqNumber	(void);
void	AddNewCmcbs		(void);
void	CloseDB		 	(void);
void	CommitInsf		(int, char *);
void	DiscProcess		(void);
void	FreeInsf		(int, char *);
void	IntUpdateInsf	(int, char *);
void	OpenDB			(void);
void	PriceProcess	(void);
void	PrintCoStuff	(void);
void	ProcessKitItem	(long, float);
void	SetRequistion	(void);
void	shutdown_prog	(void);
void	SrchCcmr		(char *);
void	SrchCmcb		(char *);
void	SrchCmcm		(char *);
void	SrchCmhr		(char *);
void	SrchCmrh		(char *);
void	SrchInsf		(char *, int);
void	SrchInum		(char *);
void	tab_other		(int);

/*
 * Main Processing Routine. 
 */
int
main (
 int	argc,
 char *	argv [])
{
	char *	sptr;
		
	int	i;

	TruePosition	=	TRUE;

	SETUP_SCR (vars);


	if (argc != 2)
	{
		print_at (0, 0, ML (mlStdMess036), argv [0]);
		return (EXIT_FAILURE);
	}

	/*
	 * Get current user.
	 */
	sptr = getenv ("LOGNAME");
	sprintf (envLogname, "%-14.14s", sptr);

	sptr = chk_env ("DB_MCURR");
	envDbMcurr = (sptr == (char *)0) ? TRUE : atoi (sptr);

	sprintf (envCurrCode, "%-3.3s", get_env ("CURR_CODE"));
	sprintf (envSoOverrideQty, "%-1.1s", get_env ("SO_OVERRIDE_QTY"));

	/*
	 * Validate if serial items allowed.
	 */
	sptr = chk_env ("SK_SERIAL_OK");
	envSerialItemsOk = (sptr == (char *)0) ? FALSE : atoi (sptr);

	tab_col = (envSerialItemsOk) ? 16 : 30;
	tab_row = 8;
	/*
	 * Get printerNo.
	 */
	printerNo = atoi (argv [1]);

	/*
	 * Multi-bin ?.
	 */
	envMultLoc = atoi (get_env ("MULT_LOC"));

	/*
	 * Check auto generation of requisition numbers.
	 */
	sptr = chk_env ("CM_AUTO_REQ");
	envCmAutoReq = (sptr == (char *)0) ? 2 : atoi (sptr);

	/*
	 * Check contract number level.
	 */
	sptr = chk_env ("CM_AUTO_CON");
	envCmAutoCon = (sptr == (char *)0) ? 2 : atoi (sptr);

	/*
	 * Input Discount.
	 */
	sptr = chk_env ("INP_DISC");
	if (sptr == (char *)0)
		envInpDisc = 0;
	else
		envInpDisc = (*sptr == 'M' || *sptr == 'm');

	/*
	 * Forward orders included in available ?
	 */
	sptr = chk_env ("SO_FWD_AVL");
	envSoFwdAvl = (sptr == (char *)0) ? TRUE : atoi (sptr);

	/*
	 * Check for Stock infomation window.
	 */
	sptr = chk_env ("WIN_OK");
	envWinOk = (sptr == (char *)0) ? TRUE : atoi (sptr);

	init_scr ();			/*  sets terminal from termcap	*/
	set_tty ();             /*  get into raw mode			*/
	_set_masks (argv [0]);	/*  setup print using masks		*/

/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (2, store, sizeof (struct storeRec));
#endif
	init_vars (SCN_HEADER);			/*  set default values			*/

	/*
	 * Save vars [?].required values.
	 */
	SaveRequired ();

	for (i = 0;i < 3;i++)
		tab_data [i]._desc = scn_desc [i];

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));
	local_rec.lsystemDate = TodaysDate ();

	OpenDB ();

	FLD ("LL") = ND;
	if (SK_BATCH_CONT || MULT_LOC)
		FLD ("LL") = YES;

	FLD ("serialNo") = (envSerialItemsOk) ? YES : ND;

	OpenPrice 	();
	OpenDisc 	();

	strcpy (branchNo, (envCmAutoCon == COMPANY) ? " 0" : comm_rec.est_no);
	strcpy (reqBranchNo, (envCmAutoReq == COMPANY) ? " 0" : comm_rec.est_no);

	while (prog_exit == 0)
	{
		entry_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		init_vars (SCN_HEADER);
		lcount [SCN_ITEMS] 	= 0;

		for (i = 0; i < MAXLINES; i++)
		{
			memset (store + i, 0, sizeof (struct storeRec));
			strcpy (store [i].serialNo, twentyFiveSpaces);
		}
		abc_unlock (cmrh);

		/*
		 * Enter screen SCN_HEADER linear input. Turn screen initialise on.  
		 */
		init_ok = TRUE;
		eoi_ok = FALSE;

		heading (SCN_HEADER);
		entry (SCN_HEADER);
		if (prog_exit || restart)
			continue;

		/*
		 * Disable/Enable status display.
		 */
		if (newReq)
			FLD ("ln_status") = ND;
		else
			FLD ("ln_status") = NA;

		strcpy (previousCostHead, "");
		if (newReq)
		{
			heading (SCN_ITEMS);
			entry (SCN_ITEMS);
			if (restart)
				continue;
		}

		edit_all ();
		if (restart)
			continue;

		Update ();
	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*
 * Program exit sequence. 
 */
void
shutdown_prog (
 void)
{
#ifdef GVISION
	CloseStockWindow ();
#else
	if (wpipe_open)
	{
		pclose (pout);
		IP_CLOSE (np_fn);
		IP_UNLINK (getpid ());
	}
#endif	/* GVISION */

	CloseDB (); 
	FinishProgram ();
}

/*
 * Open Data Dase Files . 
 */
void
OpenDB (void)
{
	abc_dbopen (data);
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	abc_alias (ccmr2, ccmr);
	abc_alias (cmcm2, cmcm);
	abc_alias (cmhr2, cmhr);
	abc_alias (cmrh2, cmrh);
	abc_alias (inmr2, inmr);
	abc_alias (inum2, inum);

	open_rec (ccmr,  ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	open_rec (ccmr2, ccmr_list, CCMR_NO_FIELDS, "ccmr_hhcc_hash");
	open_rec (cmcb,  cmcb_list, CMCB_NO_FIELDS, "cmcb_id_no");
	open_rec (cmcd,  cmcd_list, CMCD_NO_FIELDS, "cmcd_id_no");
	open_rec (cmcm,  cmcm_list, CMCM_NO_FIELDS, "cmcm_id_no");
	open_rec (cmcm2, cmcm_list, CMCM_NO_FIELDS, "cmcm_hhcm_hash");
	open_rec (cmhr,  cmhr_list, CMHR_NO_FIELDS, "cmhr_hhhr_hash");
	open_rec (cmhr2, cmhr_list, CMHR_NO_FIELDS, "cmhr_id_no2");
	open_rec (cmrh,  cmrh_list, CMRH_NO_FIELDS, "cmrh_id_no");
	open_rec (cmrh2, cmrh_list, CMRH_NO_FIELDS, "cmrh_id_no");
	open_rec (cmrd,  cmrd_list, CMRD_NO_FIELDS, "cmrd_id_no");
	open_rec (comr,  comr_list, COMR_NO_FIELDS, "comr_co_no");
	open_rec (cumr,  cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (esmr,  esmr_list, ESMR_NO_FIELDS, "esmr_id_no");
	open_rec (incc,  incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (inmr,  inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (inmr2, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
    open_rec (inum,  inum_list, INUM_NO_FIELDS, "inum_uom");
    open_rec (inum2, inum_list, INUM_NO_FIELDS, "inum_id_no2");

	OpenInsf ();

	/*
	 * Read ccmr record for current warehouse.
	 */
	strcpy (ccmr_rec.co_no,  comm_rec.co_no);
	strcpy (ccmr_rec.est_no, comm_rec.est_no);
	strcpy (ccmr_rec.cc_no,  comm_rec.cc_no);
	cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");

	OpenLocation (ccmr_rec.hhcc_hash);
}

/*
 * Close data base files. 
 */
void
CloseDB (void)
{
	abc_fclose (ccmr);
	abc_fclose (ccmr2);
	abc_fclose (cmcb);
	abc_fclose (cmcd);
	abc_fclose (cmcm);
	abc_fclose (cmcm2);
	abc_fclose (cmhr);
	abc_fclose (cmhr2);
	abc_fclose (cmrh);
	abc_fclose (cmrh2);
	abc_fclose (cmrd);
	abc_fclose (comr);
	abc_fclose (cumr);
	abc_fclose (esmr);
	abc_fclose (incc);
	abc_fclose (inmr);
	abc_fclose (inmr2);
	abc_fclose (inum);
	abc_fclose (inum2);

	CloseLocation ();
	CloseCosting ();

	ClosePrice ();
	CloseDisc ();

	SearchFindClose ();
	abc_dbclose (data);
}

int
spec_valid (
 int field)
{
	int	i;
	int	new_costhd;
	int	this_page;
	int	TempLine;
	int	TempLineCnt;

	/*
	 * Validate Requisition Number.
	 */
	if (LCHECK ("requisitionNo"))
	{
		SetRequired (NORMAL);
		newReq = TRUE;
		strcpy (cmrh_rec.full_supply, "N");
		strcpy (local_rec.requisitionDesc, twentyFiveSpaces);
		DSP_FLD ("requisitionDesc");

		if (SRCH_KEY)
		{
			SrchCmrh (temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used || local_rec.requisitionNo == 0L)
		{
			if (envCmAutoReq == MANUAL)
			{
				print_mess (ML (mlCmMess026));		
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
			strcpy (local_rec.requisitionDesc, ML ("(New Requisition)"));
			DSP_FLD ("requisitionDesc");
		
			/*
			 * Set defaults for requisition. 
			 */
			SetRequistion ();
			return (EXIT_SUCCESS);
		}

		strcpy (cmrh_rec.co_no, comm_rec.co_no);
		strcpy (cmrh_rec.br_no, reqBranchNo);
		cmrh_rec.req_no = local_rec.requisitionNo;

		/*
		 * Lookup requisition.
		 */
		if (!find_rec (cmrh, &cmrh_rec, COMPARISON, "u"))
		{
			cc = LoadItems ();
			if (cc == 1)
			{
			    scn_set (SCN_HEADER);
				print_mess (ML (mlCmMess074));		
			    sleep (sleepTime);
			    clear_mess ();
			    return (EXIT_FAILURE);
			}

			/*
			 * Display screen 1
			 */
			entry_exit = TRUE;
			newReq = FALSE;
			return (LoadReq ());
		}
		strcpy (local_rec.requisitionDesc, ML ("(New Requisition)"));
		DSP_FLD ("requisitionDesc");
		newReq = TRUE;
		
		/*
		 * Set defaults for requisition.
		 */
		SetRequistion ();

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("contractNo"))
	{
		if (SRCH_KEY)
		{
			SrchCmhr (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (cmhr_rec.co_no, comm_rec.co_no);
		strcpy (cmhr_rec.br_no, branchNo);
		sprintf (cmhr_rec.cont_no, "%-6.6s", zero_pad (local_rec.contractNo,6));
		cc = find_rec (cmhr2, &cmhr_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess075));		
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
	
		/*
		 * Contract must be open.
		 */
		if (cmhr_rec.status [0] != 'O')
		{
			print_mess (ML (mlCmMess018));		
			sleep (sleepTime);
			clear_mess ();	
			return (EXIT_FAILURE);
		}

		/*
		 * Find customer for contract
		 */
		cumr_rec.hhcu_hash	=	cmhr_rec.hhcu_hash;
		cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess021));		
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
	
		/*
		 * Load contract decsription.
		 */
		cmcd_rec.hhhr_hash = cmhr_rec.hhhr_hash;
		strcpy (cmcd_rec.stat_flag, "D");
		cmcd_rec.line_no = 0;
		cc = find_rec (cmcd, &cmcd_rec, GTEQ, "r");
		while (!cc && cmcd_rec.hhhr_hash == cmhr_rec.hhhr_hash &&
		       		  cmcd_rec.stat_flag [0] == 'D' && cmcd_rec.line_no < 7)
		{
			sprintf (local_rec.contDesc [cmcd_rec.line_no],
				"%-70.70s",
				cmcd_rec.text);
	
			cc = find_rec (cmcd, &cmcd_rec, NEXT, "r");
		}

		/*
		 * Display contract description.
		 */
		for (i = label ("descr1"); i <= label ("descr7"); i++)
			display_field (i);

		/*
		 * Display Address.
		 */
		sprintf (cmrh_rec.del_name, "%-40.40s", cmhr_rec.contact);
		sprintf (cmrh_rec.del_adr1, "%-40.40s", cmhr_rec.adr1);
		sprintf (cmrh_rec.del_adr2, "%-40.40s", cmhr_rec.adr2);
		sprintf (cmrh_rec.del_adr3, "%-40.40s", cmhr_rec.adr3);
		DSP_FLD ("del_name");
		DSP_FLD ("del_addr1");
		DSP_FLD ("del_addr2");
		DSP_FLD ("del_addr3");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("rqrd_date"))
	{
		if (dflt_used)
			cmrh_rec.rqrd_date = local_rec.lsystemDate;

		if (cmrh_rec.rqrd_date > local_rec.lsystemDate)
		{
			i = prmptmsg (ML (mlCmMess027),	"YyNn",
				     1, PRMPT_LINE);
			if (i == 'N' || i == 'n')
			{
				cmrh_rec.rqrd_date = local_rec.lsystemDate;
				DSP_FLD ("rqrd_date");
			}
			move (0, PRMPT_LINE);
			cl_line ();
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("del_addr1"))
	{
		if (dflt_used)
		{
			skip_entry = goto_field (cur_field, label ("add_int1"));
			DSP_FLD ("del_addr1");
			DSP_FLD ("del_addr2");
			DSP_FLD ("del_addr3");
			return (EXIT_SUCCESS);
		}

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("add_int1"))
	{
		if (dflt_used)
		{
			if (prog_status == ENTRY)
				entry_exit = TRUE;
			
			return (EXIT_SUCCESS);
		}

		return (EXIT_SUCCESS);
	}

	/*
	 * Screen 2 validation.
	 */
	if (LCHECK ("costhd"))
	{
		new_costhd = FALSE;
		if (SRCH_KEY)
		{
			if (search_key == FN9)
				SrchCmcm (temp_str);
			else
				SrchCmcb (temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used || last_char == DELLINE)
		{
			if (prog_status == ENTRY && strlen (previousCostHead) != 0)
			{
				sprintf (local_rec.costHead, "%-4.4s", previousCostHead);
			}
			else
				return (DeleteLine ());
		}

		strcpy (cmcm_rec.co_no, comm_rec.co_no);
		sprintf (cmcm_rec.ch_code, "%-4.4s", local_rec.costHead);
		cc = find_rec (cmcm, &cmcm_rec, EQUAL, "r");
		while (cc)
		{
			i= prmptmsg (ML (mlCmMess016), "YNyn", 1, PRMPT_LINE);

			move (0, PRMPT_LINE);
			cl_line ();
			if (i == 'n' || i == 'N')
				return (EXIT_FAILURE);

			i = line_cnt;
			putval (line_cnt);

			new_costhd = TRUE;
			snorm ();
			* (arg) = "cm_costhd";
			* (arg+ (1)) = (char *)0;
			shell_prog (2);
			swide ();
			heading (SCN_ITEMS);

			line_cnt = i;
			if (prog_status == ENTRY)
				lcount [SCN_ITEMS] = line_cnt;

			scn_display (SCN_ITEMS);
			getval (line_cnt);
			line_display ();

			cc = find_rec (cmcm, &cmcm_rec, EQUAL, "r");
			if (!cc)
				AddNewCmcbs ();
		}

		if (!new_costhd && NotInContract ())
		{
			sprintf (err_str, ML (mlCmMess016),cmcm_rec.ch_code,cmhr_rec.cont_no);
			i = prmptmsg (err_str, "YNyn", 1, PRMPT_LINE);

			print_at (PRMPT_LINE, 1, "%-100.100s", " ");

			if (i == 'N' || i == 'n')
				return (EXIT_FAILURE);
			
			AddNewCmcbs ();
		}

		sprintf (local_rec.costHeadDesc, "%-40.40s", cmcm_rec.desc);
		print_at (5, 20, "%-40.40s", local_rec.costHeadDesc);

		sprintf (SR.costHeadDesc, "%-40.40s", cmcm_rec.desc);
		local_rec.hhcmHash = cmcm_rec.hhcm_hash;

		sprintf (previousCostHead, "%-4.4s", cmcm_rec.ch_code);

		/*
		 * The following lines allow edit of new lines added to an existing
		 * requisition EVEN if the line will be fully backordered.            
		 */
		if (!newReq && line_cnt >= lcount [SCN_ITEMS])
			strcpy (SR.oldStatFlag, "R");

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("wh_no"))
	{
		if (SRCH_KEY)
		{
			SrchCcmr (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (ccmr_rec.co_no,  comm_rec.co_no);
		strcpy (ccmr_rec.est_no, comm_rec.est_no);
		strcpy (ccmr_rec.cc_no,  local_rec.wh_no);
		cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess027));		
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		local_rec.hhccHash = ccmr_rec.hhcc_hash;

		 ReadLLCT (local_rec.hhccHash);

		if (llctInput [0] == 'V')
			SR.lotSelectFlag	=	INP_VIEW;
		if (llctInput [0] == 'A')
			SR.lotSelectFlag	=	INP_AUTO;
		if (llctInput [0] == 'M')
		{
			strcpy (StockTake, "Y");
			SR.lotSelectFlag	=	INP_VIEW;
		}
		
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("item_no"))
	{
		/*
		 * Search for part number.
		 */
		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}

		return (ValidItemNo ());
	}

	/*
	 * Validate discount.
	 */
	if (LCHECK ("disc_pc"))
	{
		if (FLD ("disc_pc") == NI && prog_status == ENTRY)
			return (EXIT_SUCCESS);

		if (dflt_used)
		{
			strcpy (SR.discOverride, "N");
			DiscProcess ();
		}

		if (SR.conPrice || SR.contStatus == 2)
		{
			cmrd_rec.disc_pc 	=	0.00;
			SR.discA			=	0.00;
			SR.discB			=	0.00;
			SR.discC			=	0.00;
			DSP_FLD ("disc_pc");
		}
		SR.discPc = cmrd_rec.disc_pc;

		if (SR.calcDisc != cmrd_rec.disc_pc)
			strcpy (SR.discOverride, "Y");

		/*
		 * Discount has been entered so set disc B & C to zero.     
		 */
		if (!dflt_used)
		{
			SR.discA = SR.discPc;
			SR.discB = 0.00;
			SR.discC = 0.00;
		}
		SR.discPc = cmrd_rec.disc_pc;

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Unit of Measure. 
	 */
	if (LCHECK ("UOM"))
	{
		if (dflt_used)
		{
			strcpy (local_rec.UOM, inum_rec.uom);
			strcpy (SR.UOM, inum_rec.uom);
			SR.hhumHash = inum_rec.hhum_hash;
		}

		if (SRCH_KEY)
		{
			SrchInum (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (inum2_rec.uom_group, inum_rec.uom_group);
		strcpy (inum2_rec.uom, local_rec.UOM);
		cc = find_rec (inum2, &inum2_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess028));		
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
	
		if (ValidItemUom (SR.hhbrHash, inum2_rec.hhum_hash))
		{
			sprintf (err_str, ML ("Invalid Unit of Measure for Item."));
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.UOM, inum2_rec.uom);
		strcpy (SR.UOM, inum2_rec.uom);
		SR.hhumHash 	= inum2_rec.hhum_hash;

		if (inum_rec.cnv_fct == 0.00)
			inum_rec.cnv_fct = 1.00;

		SR.cnvFct 	= inum2_rec.cnv_fct/inum_rec.cnv_fct;
		PriceProcess ();
		DiscProcess ();
		DSP_FLD ("UOM");
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate quantity.
	 */
	if (LCHECK ("qtyOrder"))
	{
		/*
		 * A KIT has been ordered.
		 */
		if (KIT_ITEM)
		{
			this_page = line_cnt / TABLINES;
			ProcessKitItem (inmr_rec.hhbr_hash, local_rec.qtyOrder);
			skip_entry = goto_field (label ("qtyOrder"),
		                 	         label ("costhd"));
			local_rec.qtyOrder = 0.00;
			SR.qtyOrder = 0.00;
			if (this_page == (line_cnt / TABLINES))
				blank_display ();
			return (EXIT_SUCCESS);
		}

		/*
		 * A PHANTOM has been ordered.
		 */
		if (PHANTOM)
			SR.qtyAvailable = ProcessPhantom (inmr_rec.hhbr_hash, local_rec.qtyOrder);

		if (prog_status == ENTRY)
		{
			local_rec.qtyBackorder = 0.00;
			SR.qtyBackorder = 0.00;
		}
			
		/*
		 * Serial Items Can only have Qty of 0.00 or 1.00
		 */
		if (SR.costingFlag [0] == 'S' &&
		    local_rec.qtyOrder != 0.00 && 
                    local_rec.qtyOrder != 1.00)
		{
			print_mess (ML (mlStdMess029));		
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		PriceProcess ();
		DiscProcess ();

		if (envWinOk && 
                    ((SR.qtyAvailable - local_rec.qtyOrder) < 0.00) && 
		     !NO_COST && !NON_STOCK)
		{
			sprintf (err_str,
				ML (mlStdMess090),
				SR.qtyAvailable,
				clip (inmr_rec.item_no));
			clear_mess ();
			print_mess (err_str);
	
			if (InputRes () && cmrh_rec.full_supply [0] == 'N')
			{
				sprintf (err_str, 
					"%s %s ? ", 
					ML (mlCmMess199), 
					cumr_rec.dbt_no);
			
				i = prmptmsg (err_str, "YyNn", 1, PRMPT_LINE);
				print_at (PRMPT_LINE, 1, "%-100.100s", " ");
				if (i == 'Y' || i == 'y')
					strcpy (cmrh_rec.full_supply, "Y");
				else
					strcpy (cmrh_rec.full_supply, "A");
			}
			SR.qtyOrder = local_rec.qtyOrder;
		
			DSP_FLD ("qtyOrder");
		}

		if (prog_status == ENTRY)
		{
			DSP_FLD ("qtyBackorder");
			if (local_rec.qtyOrder != 0.00)
				skip_entry = (cmrd_rec.sale_price == 0.00) ? 1 : 3;
		}

		if (NO_COST)
			SR.costPrice = 0.00;
		else
		{
			cc = FindCost ();
			if (cc)
			{
				SR.costPrice = 0.00;
				return (EXIT_FAILURE);
			}
		}
		SR.qtyOrder = local_rec.qtyOrder;

		/*
		 * Set status on a line for an existing requisition.
		 */
		if (!newReq)
			SetStatus (field);

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Quantity Backordered.
	 */
	if (LCHECK ("qtyBackorder"))
	{
		if (prog_status == ENTRY)
		{
			if (local_rec.qtyOrder != 0.00)
				return (EXIT_SUCCESS);
		}

		if (!BO_OK)
		{
			if (local_rec.qtyBackorder != 0.00)
			{
				print_mess (ML (mlStdMess030));		
				sleep (sleepTime);
				clear_mess ();
				local_rec.qtyBackorder = 0.00;
				SR.qtyBackorder = 0.00;
				DSP_FLD ("qtyBackorder");
				skip_entry = goto_field (cur_field, label ("qtyOrder"));
			}
			/*
			 * Set status on a line for an existing requisition.
			 */
			if (!newReq)
				SetStatus (field);

			return (EXIT_SUCCESS);
		}
		PriceProcess ();
		DiscProcess ();

		if (prog_status == ENTRY && cmrd_rec.sale_price == 0.00)
			skip_entry = 0;
		else
			skip_entry = 2;

		/*
		 * Set status on a line for an existing requisition.
		 */
		if (!newReq)
			SetStatus (field);

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Price Input.
	 */
	if (LCHECK ("sale_price")) 
	{
		if (dflt_used)
		{
			strcpy (SR.priceOverride, "N");
			PriceProcess ();
			DiscProcess ();
			DSP_FLD ("sale_price");
		}

		if (cmrd_rec.sale_price == 0.00)
		{
			i = prmptmsg (ML (mlStdMess031),"YyNn",1,PRMPT_LINE);
			print_at (PRMPT_LINE,1, "                                    ");
			if (i != 'Y' && i != 'y')
				return (EXIT_FAILURE);
		}

		if (SR.calcSalePrice != cmrd_rec.sale_price)
			strcpy (SR.priceOverride, "Y");

		/*
		 * Calculate new GROSS sale price.
		 */
		SR.gSalePrice = no_dec (cmrd_rec.sale_price / (1.00 - (SR.regPc / 100.00)));
		SR.salePrice = GetCusGprice (SR.gSalePrice, SR.regPc);
		cmrd_rec.sale_price = SR.salePrice;

		DiscProcess ();

		SR.actSalePrice = cmrd_rec.sale_price;

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Serial Number
	 */
	if (LCHECK ("serialNo"))
	{
		if (F_HIDE (field) || FIELD.required == NA || SR.serialFlag [0] != 'Y')
			return (EXIT_SUCCESS);

		if (end_input)
			return (EXIT_SUCCESS);

		abc_selfield (insf, "insf_id_no");

		if (SRCH_KEY)
		{
			SrchInsf (temp_str, line_cnt);
			return (EXIT_SUCCESS);
		}

		if (dflt_used || !strcmp (local_rec.serialNo, twentyFiveSpaces))
		{
			/*
			 * Free previous serial number if any.
			 */
			if (strcmp (SR.serialNo, twentyFiveSpaces))
			{
				cc = 	UpdateInsf 
						(
							SR.hhwhHash, 
							SR.hhsiHash, 
							SR.serialNo, 
							"C", 
							"F"
						);
	
				if (cc && cc < 1000)
					file_err (cc, insf, "DBUPDATE");
			}

			strcpy (local_rec.serialNo, twentyFiveSpaces);
			strcpy (SR.serialNo, twentyFiveSpaces);
			return (EXIT_SUCCESS);
		}

		insfRec.hhwh_hash = SR.hhwhHash;
		insfRec.hhbr_hash = SR.hhsiHash;
		strcpy (insfRec.status, "F");
		strcpy (insfRec.serial_no, local_rec.serialNo);
		cc	=	FindInsf 
				(
					SR.hhwhHash,
					0L,
					local_rec.serialNo,
					"F",
					"r"
				);
		if (cc)
		{
			cc	=	FindInsf 
					(
						0L,
						SR.hhsiHash,
						local_rec.serialNo,
						"F",
						"r"
					);
		}
		if (cc)
		{
			print_mess (ML (mlCmMess028));		
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (CheckSupInsf (local_rec.serialNo, SR.hhsiHash, line_cnt))
		{
			print_mess (ML (mlCmMess029));		
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (SR.serialNo, local_rec.serialNo);

		DSP_FLD ("serialNo");
		abc_selfield (insf, "insf_id_no");

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate lots and locations.
	 */
	if (LCHECK ("LL"))
	{
		int		LLReturnValue	=	0;

		if (FLD ("LL") == ND)
			return (EXIT_SUCCESS);

		TempLine	=	lcount [SCN_ITEMS];
		TempLineCnt	=	line_cnt;
		LLReturnValue = DisplayLL
			(										/*----------------------*/
				line_cnt,							/*	Line number.		*/
				tab_row + 3 + (line_cnt % TABLINES),/*  Row for window		*/
				tab_col,							/*  Col for window		*/
				4,									/*  length for window	*/
				SR.hhwhHash, 						/*	Warehouse hash.		*/
				SR.hhumHash,						/*	UOM hash			*/
				SR.hhccHash,						/*	CC hash.			*/
				SR.UOM,								/* UOM					*/
				SR.qtyOrder,						/* Quantity.			*/
				SR.cnvFct,							/* Conversion factor.	*/
				TodaysDate (), 						/* Expiry Date.			*/
				SR.lotSelectFlag,					/* Silent mode			*/
				(local_rec.LL [0] == 'Y'),			/* Input Mode.			*/
				SR.lotControl						/* Lot controled item. 	*/
													/*----------------------*/
			);
		/*
		 * Redraw screens.
		 */
		strcpy (local_rec.LL,"Y");
		putval (line_cnt);

		lcount [SCN_ITEMS] = (line_cnt + 1 > lcount [SCN_ITEMS]) ? line_cnt + 1 : lcount [SCN_ITEMS];
		heading (SCN_ITEMS);
		line_cnt = TempLineCnt;
		scn_display (SCN_ITEMS);
		lcount [SCN_ITEMS] = TempLine;
		PrintCoStuff ();
		if (LLReturnValue)
			return (EXIT_FAILURE);
		
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

/*
 *  Delete tabular screen line.
 */
int
DeleteLine (void)
{
	int	i;
	int	this_page = line_cnt / TABLINES;

	if (prog_status == ENTRY)
	{
		print_mess (ML (mlStdMess005));		
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}

	if (lcount [SCN_ITEMS] == 0)
	{
		print_mess (ML (mlStdMess032));		
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}


	print_at (1, 0, ML (mlStdMess035));
	fflush (stdout);

	FreeInsf (line_cnt, SR.serialNo);

	lcount [SCN_ITEMS]--;

	for (i = line_cnt,line_cnt = 0;line_cnt < lcount [SCN_ITEMS];line_cnt++)
	{
		if (line_cnt >= i)
		{
			SR.hhbrHash		= store [line_cnt + 1].hhbrHash;
			SR.hhsiHash		= store [line_cnt + 1].hhsiHash;
			SR.hhccHash		= store [line_cnt + 1].hhccHash;
			SR.hhumHash		= store [line_cnt + 1].hhumHash;
			SR.hhwhHash		= store [line_cnt + 1].hhwhHash;
			SR.closingStock			= store [line_cnt + 1].closingStock;
			SR.qtyAvailable		= store [line_cnt + 1].qtyAvailable;
			SR.weight			= store [line_cnt + 1].weight;
			SR.outerSize			= store [line_cnt + 1].outerSize;
			SR.qtyOrder			= store [line_cnt + 1].qtyOrder;
			SR.qtyBackorder		= store [line_cnt + 1].qtyBackorder;
			SR.qtyIssue			= store [line_cnt + 1].qtyIssue;
			SR.discPc			= store [line_cnt + 1].discPc;
			SR.regPc			= store [line_cnt + 1].regPc;
			SR.discA			= store [line_cnt + 1].discA;
			SR.discB			= store [line_cnt + 1].discB;
			SR.discC			= store [line_cnt + 1].discC;
			SR.calcDisc		= store [line_cnt + 1].calcDisc;
			SR.dfltDisc		= store [line_cnt + 1].dfltDisc;
			SR.salePrice		= store [line_cnt + 1].salePrice;
			SR.calcSalePrice		= store [line_cnt + 1].calcSalePrice;
			SR.actSalePrice		= store [line_cnt + 1].actSalePrice;
			SR.gSalePrice		= store [line_cnt + 1].gSalePrice;
			SR.dfltPrice		= store [line_cnt + 1].dfltPrice;
			SR.costPrice		= store [line_cnt + 1].costPrice;
			SR.pricingChk		= store [line_cnt + 1].pricingChk;
			SR.cumulative		= store [line_cnt + 1].cumulative;
			SR.conPrice		= store [line_cnt + 1].conPrice ;
			SR.contStatus		= store [line_cnt + 1].contStatus;
			SR.decPt			= store [line_cnt + 1].decPt;
	
			strcpy (SR.category, 	store [line_cnt + 1].category);
			strcpy (SR.sellGroup, 	store [line_cnt + 1].sellGroup);
			strcpy (SR.costHeadDesc,store [line_cnt + 1].costHeadDesc);
			strcpy (SR.itemDesc, 	store [line_cnt + 1].itemDesc);
			strcpy (SR.itemClass, 		store [line_cnt + 1].itemClass);
			strcpy (SR.bOrderFlag, 	store [line_cnt + 1].bOrderFlag);
			strcpy (SR.costingFlag, 	store [line_cnt + 1].costingFlag);
			strcpy (SR.priceOverride, 	store [line_cnt + 1].priceOverride);
			strcpy (SR.discOverride, 	store [line_cnt + 1].discOverride);
			strcpy (SR.lotControl, 	store [line_cnt + 1].lotControl);
			strcpy (SR.serialFlag, store [line_cnt + 1].serialFlag);
			strcpy (SR.serialNo, 	store [line_cnt + 1].serialNo);
			strcpy (SR.oldStatFlag, store [line_cnt + 1].oldStatFlag);
			strcpy (SR.statFlag, 	store [line_cnt + 1].statFlag);
	
			getval (line_cnt + 1);
			if (this_page == line_cnt / TABLINES)
				line_display ();
		}
		else
			getval (line_cnt);

		putval (line_cnt);
	}

	if (this_page == line_cnt / TABLINES)
		blank_display ();
	
	strcpy (previousCostHead, "");
	print_at (PRMPT_LINE, 0, "%-100.100s", " ");

	line_cnt = i;
	getval (line_cnt);
	return (EXIT_SUCCESS);
}

/*
 * Validate Item Number.
 */
int
ValidItemNo (
 void)
{
	int		i;
	int		itemChanged = FALSE;

	/*
	 * Change index.
	 */
	abc_selfield (inmr, "inmr_id_no");

	skip_entry = 0;

	/*
	 * Find item on file.
	 */
	cc = FindInmr (comm_rec.co_no, inmr_rec.item_no, 0L, "N");
	if (!cc)
	{
		strcpy (inmr_rec.co_no, comm_rec.co_no);
		strcpy (inmr_rec.item_no, inmr_rec.item_no);
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
	}
	if (cc)
	{
		print_mess (ML (mlStdMess001));		
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}

	/*
	 * Supercession / Alternate Error.
	 */
	SuperSynonymError ();

	/*
	 * Check item class.
	 */
	if (NON_STOCK)
	{
		print_mess (ML (mlCmMess076));
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}

	/*
	 * Display item Number and description.
	 */
	DSP_FLD ("item_no");
	sprintf (local_rec.item_desc, "%-40.40s", inmr_rec.description);
	sprintf (SR.itemDesc, "%-40.40s", inmr_rec.description);
	tab_other (line_cnt);

	if (prog_status != ENTRY && SR.hhbrHash != inmr_rec.hhbr_hash)
		itemChanged = TRUE;
	
	SR.hhbrHash = inmr_rec.hhbr_hash;
	SR.hhsiHash = alt_hash (inmr_rec.hhbr_hash, inmr_rec.hhsi_hash);
	/*
	 * Look up to see if item is on issuing Cost Centre
	 */
	incc_rec.hhcc_hash = local_rec.hhccHash;
	incc_rec.hhbr_hash = alt_hash (SR.hhbrHash, SR.hhsiHash);
	cc = find_rec (incc, &incc_rec, COMPARISON, "r");
	if (cc)
	{
		sprintf (err_str, ML (mlStdMess033), clip (inmr_rec.item_no));
		i = prmptmsg (err_str, "YyNn", 1, PRMPT_LINE);
		print_at (PRMPT_LINE, 1, "%-100.100s", " ");
		if (i == 'n' || i == 'N') 
			return (EXIT_FAILURE);
		else 
		{
			cc = Addincc (local_rec.hhccHash);
			if (cc)
				file_err (cc, incc, "DBADD");
			SR.qtyAvailable = 0.00;
		}
	}

	/*
	 * Information into store [] array. 
	 */
	SR.hhccHash = incc_rec.hhcc_hash;
	SR.weight = inmr_rec.weight;
	SR.outerSize = inmr_rec.outer_size;
	SR.decPt = inmr_rec.dec_pt;
	SR.dfltDisc = inmr_rec.disc_pc;
	strcpy (SR.sellGroup, inmr_rec.sellgrp);
	strcpy (SR.bOrderFlag, inmr_rec.bo_flag);
	strcpy (SR.itemClass, inmr_rec.inmr_class);
	strcpy (SR.lotControl, inmr_rec.lot_ctrl);
	strcpy (SR.serialFlag, inmr_rec.serial_item);
	strcpy (SR.costingFlag, inmr_rec.costing_flag);
	strcpy (SR.priceOverride, "N");
	strcpy (SR.discOverride, "N");
	strcpy (local_rec.UOM, inmr_rec.sale_unit);

    /*
     * Find for UOM GROUP.
     */
    strcpy (inum_rec.uom, inmr_rec.sale_unit);
    cc = find_rec (inum, &inum_rec, EQUAL, "r");
    if (cc)
        file_err (cc, inum, "DBFIND");

    SR.hhumHash   = inum_rec.hhum_hash;
    SR.cnvFct     = inum_rec.cnv_fct;

	strcpy (local_rec.lotControl, inmr_rec.lot_ctrl);
	
	SR.hhwhHash 	= 	incc_rec.hhwh_hash;
	SR.closingStock = 	incc_rec.closing_stock;
	SR.qtyAvailable = 	incc_rec.closing_stock -
						incc_rec.backorder -
						incc_rec.committed -
						incc_rec.forward;

	if (PHANTOM)
		SR.qtyAvailable = PhantomSOH (inmr_rec.hhbr_hash);


	FLD ("qtyOrder") = YES;

	if (itemChanged)
	{
		SR.qtyIssue = 0.00;
		local_rec.qtyOrder = 0.00;
		local_rec.qtyBackorder = 0.00;
		local_rec.workCost = 0.00;
		sprintf (local_rec.serialNo, "%-25.25s", " ");
		sprintf (SR.serialNo, "%-25.25s", " ");
		PriceProcess ();
		DiscProcess ();
		DSP_FLD ("qtyOrder");
		DSP_FLD ("qtyBackorder");
		DSP_FLD ("serialNo");
	}

	/*
	 * Item is a serial item.
	 */
	strcpy (SR.serialFlag, inmr_rec.serial_item);
	/*
	 * Item is a serial item.
	 */
	if (SERIAL)
	{
		if (!F_HIDE (label ("serialNo")))
		{
			FLD ("serialNo") = YES;
			FLD ("qtyOrder") = NA;
			local_rec.qtyOrder = 1.00;
			DSP_FLD ("qtyOrder");
		}
	}
	else
	{
		if (!F_HIDE (label ("serialNo")))
		{
			FLD ("serialNo") = NA;
		}
		FLD ("qtyOrder") = YES;
	}
	if (envSoFwdAvl)
	{
		SR.qtyAvailable = incc_rec.closing_stock -
						incc_rec.committed -
						incc_rec.backorder - 
						incc_rec.forward;
	}
	else
	{
		SR.qtyAvailable = incc_rec.closing_stock -
						incc_rec.committed -
						incc_rec.backorder;
	}
	return (EXIT_SUCCESS);
}

void
PriceProcess (
 void)
{
	int		pType;
	float	regPc;
	double	grossPrice;
	SR.pricingChk	= FALSE;

	pType = atoi (cumr_rec.price_type);
	grossPrice = GetCusPrice (ccmr_rec.co_no,
					  		  ccmr_rec.est_no,
							  ccmr_rec.cc_no,
							  "  ",	
							  cumr_rec.class_type,
							  SR.sellGroup,
							  cumr_rec.curr_code,
							  pType,
							  cumr_rec.disc_code,
							  " ",
							  cumr_rec.hhcu_hash,
							  ccmr_rec.hhcc_hash,
							  SR.hhbrHash,
							  SR.category,
							  0L,
							  cmrh_rec.req_date,
							  ToStdUom (local_rec.qtyOrder),
							  1.0,			/*	pocrRec.factor MCURR NEED */
							  FGN_CURR,
							  &regPc);

	SR.pricingChk	= TRUE;

	/*
	 * Inclusion of the conversion factor for the multiple unit
	 * of measure in computing the SR.calcSalePrice.          
	 */
	SR.calcSalePrice = GetCusGprice (grossPrice, regPc) * SR.cnvFct;
	if (SR.priceOverride [0] == 'N')
	{
		SR.gSalePrice 	= 	grossPrice * SR.cnvFct;
		SR.salePrice 		=	SR.calcSalePrice;
		SR.regPc 			= 	regPc;
		cmrd_rec.sale_price	= 	SR.calcSalePrice;
		SR.actSalePrice 		= 	SR.calcSalePrice;
	}
	SR.conPrice 	= (_CON_PRICE) ? TRUE : FALSE;
	SR.contStatus = _cont_status;
	DSP_FLD ("sale_price");
}

void
DiscProcess (
 void)
{
	int		pType;
	int		cumDisc;
	float	discArray [3];

	/*
	 * Discount does not apply.
	 */
	if (SR.contStatus == 2 || SR.conPrice)
	{
		cmrd_rec.disc_pc  	= 0.00;
		SR.discPc 		= 0.00;
		SR.calcDisc 		= 0.00;
		SR.discA			= 0.00;
		SR.discB			= 0.00;
		SR.discC			= 0.00;
		DSP_FLD ("disc_pc");
		return;
	}

	if (SR.pricingChk == FALSE)
		PriceProcess ();

	pType = atoi (cumr_rec.price_type);
	cumDisc		=	GetCusDisc (comm_rec.co_no,
								comm_rec.est_no,
								ccmr_rec.hhcc_hash,
								cumr_rec.hhcu_hash,
								cumr_rec.class_type,
								cumr_rec.disc_code,
								SR.hhsiHash,
								SR.category,
								SR.sellGroup,
								pType,
								SR.gSalePrice,
								SR.regPc,
								ToStdUom (local_rec.qtyOrder),
								discArray);
							
	if (SR.discOverride [0] == 'Y')
	{
		DSP_FLD ("disc_pc");
		return;
	}
	SR.calcDisc		=	CalcOneDisc (cumDisc,
								 		 discArray [0],
								 		 discArray [1],
								 		 discArray [2]);

	if (SR.discOverride [0] == 'N')
	{
		cmrd_rec.disc_pc 	=	SR.calcDisc;
		SR.discPc			=	SR.calcDisc;

		SR.discA 			= 	discArray [0];
		SR.discB 			= 	discArray [1];
		SR.discC 			= 	discArray [2];
		SR.cumulative 		= 	cumDisc;

		if (SR.dfltDisc > cmrd_rec.disc_pc && SR.dfltDisc != 0.00)
		{
			cmrd_rec.disc_pc 	= 	SR.dfltDisc;
			SR.calcDisc		=	SR.dfltDisc;
			SR.discPc			=	SR.dfltDisc;
			SR.discA 			= 	SR.dfltDisc;
			SR.discB 			= 	0.00;
			SR.discC 			= 	0.00;
		}
	}
	DSP_FLD ("disc_pc");
}

float
PhantomSOH (
	long	hhbrHash)
{
	float	min_qty = 0.00,
			on_hand = 0.00;

	int	first_time = TRUE;

	open_rec (sokt, sokt_list, SOKT_NO_FIELDS, "sokt_hhbr_hash");

	sokt_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec (sokt, &sokt_rec, GTEQ, "r");
	while (!cc && sokt_rec.hhbr_hash == hhbrHash)
	{
		incc_rec.hhcc_hash = local_rec.hhccHash;
		incc_rec.hhbr_hash = sokt_rec.mabr_hash;
		cc = find_rec (incc, &incc_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (sokt, &sokt_rec, NEXT, "r");
			continue;
		}

		on_hand = incc_rec.closing_stock -
				  incc_rec.committed -
				  incc_rec.backorder -
		          incc_rec.forward;

		on_hand /= sokt_rec.matl_qty;
		if (first_time)
			min_qty = on_hand;

		if (min_qty > on_hand)
			min_qty = on_hand;

		first_time = FALSE;

		cc = find_rec (sokt, &sokt_rec, NEXT, "r");
	}
	abc_fclose (sokt);

	return (min_qty);
}

float
ProcessPhantom (
	long	hhbrHash,
	float	qty)
{
	float	min_qty = 0.00,
			on_hand = 0.00;

	int	first_time = TRUE;

	open_rec (sokt, sokt_list, SOKT_NO_FIELDS, "sokt_hhbr_hash");

	sokt_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec (sokt, &sokt_rec, GTEQ, "r");
	while (!cc && sokt_rec.hhbr_hash == hhbrHash)
	{
		incc_rec.hhcc_hash = local_rec.hhccHash;
		incc_rec.hhbr_hash = sokt_rec.mabr_hash;
		cc = find_rec (incc, &incc_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (sokt, &sokt_rec, NEXT, "r");
			continue;
		}
	
		if (envSoFwdAvl)
		{
			on_hand = incc_rec.closing_stock -
					  incc_rec.committed -
					  incc_rec.backorder - 
		   	          incc_rec.forward;
		}
		else
		{
			on_hand = incc_rec.closing_stock -
					  incc_rec.committed -
					  incc_rec.backorder;
		}
		on_hand /= sokt_rec.matl_qty;
		if (first_time)
			min_qty = on_hand;

		if (min_qty > on_hand)
			min_qty = on_hand;

		first_time = FALSE;

		cc = find_rec (sokt, &sokt_rec, NEXT, "r");
	}
	abc_fclose (sokt);

	return (min_qty);
}

void
ProcessKitItem (
	 long	hhbrHash,
	 float	qty)
{
	int		i;
	int		this_page;
	long	hold_date = 0L;

	this_page = line_cnt / TABLINES;

	cc = open_rec (sokt, sokt_list, SOKT_NO_FIELDS, "sokt_id_no");
	if (cc)
		file_err (cc, sokt, "OPEN_REC");

	strcpy (sokt_rec.co_no, comm_rec.co_no);
	sokt_rec.hhbr_hash = hhbrHash;
	sokt_rec.line_no = 0;
	cc = find_rec (sokt, &sokt_rec, GTEQ, "r");
	while (!cc && !strcmp (sokt_rec.co_no, comm_rec.co_no) &&
			sokt_rec.hhbr_hash == hhbrHash)
	{
		abc_selfield (inmr, "inmr_hhbr_hash");
		inmr_rec.hhbr_hash	=	sokt_rec.mabr_hash;
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (sokt, &sokt_rec, NEXT, "r");
			continue;
		}
		strcpy (inmr_rec.item_no,inmr_rec.item_no);

		dflt_used = FALSE;

		if (ValidItemNo ())
		{
			cc = find_rec (sokt, &sokt_rec, NEXT, "r");
			continue;
		}
		local_rec.qtyOrder = ToLclUom (sokt_rec.matl_qty * qty);
		local_rec.qtyBackorder = 0.00;
		if (local_rec.qtyOrder == 0.00)
			get_entry (label ("qtyOrder"));
		
		if (local_rec.qtyOrder == 0.00)
		{
			cc = find_rec (sokt, &sokt_rec, NEXT, "r");
			continue;
		}
		if (sokt_rec.due_date == 0L)
			cmrh_rec.rqrd_date = hold_date;
		else
			cmrh_rec.rqrd_date = sokt_rec.due_date;

		for (i = label ("qtyOrder") ; i <= label ("serialNo") ; i++)
		{
			spec_valid (i);
			i += skip_entry;
		}
	
		putval (line_cnt);

		if (this_page != (line_cnt / TABLINES))
		{
			scn_write (cur_screen);
			lcount [SCN_ITEMS] = line_cnt;
			this_page = line_cnt / TABLINES;
		}
		lcount [SCN_ITEMS] = line_cnt;
			
		line_display ();
		line_cnt++;
		cc = find_rec (sokt, &sokt_rec, NEXT, "r");
	}
	lcount [SCN_ITEMS] = line_cnt;
	abc_fclose (sokt);
	cmrh_rec.rqrd_date = hold_date;
}

void
AddNewCmcbs (
 void)
{
	strcpy (cmcb_rec.budg_type, "V");
	cmcb_rec.budg_cost = 0.00;
	cmcb_rec.budg_qty = 0.00;
	cmcb_rec.budg_value = 0.00;
	cmcb_rec.hhhr_hash = cmhr_rec.hhhr_hash;
	cmcb_rec.hhcm_hash = cmcm_rec.hhcm_hash;
	strcpy (cmcb_rec.dtl_lvl, "A");

	cc = abc_add (cmcb, &cmcb_rec);
	if (cc)
		file_err (cc, cmcb, "DBADD");
}

int
NotInContract (
 void)
{
	cmcb_rec.hhhr_hash = cmhr_rec.hhhr_hash;
	cmcb_rec.hhcm_hash = cmcm_rec.hhcm_hash;

	cc = find_rec (cmcb, &cmcb_rec, EQUAL, "r");
	return (cc);
}

/*
 * Find item cost.
 */
int
FindCost (void)
{
	double	workCost = (double) 0;

	switch (SR.costingFlag [0])
	{
	case 'A':
	case 'L':
	case 'P':
	case 'T':
		workCost = 	FindIneiCosts
					(
						SR.costingFlag,
						comm_rec.est_no,
						SR.hhbrHash
					);
		break;

	case 'F':
	case 'I':
		workCost = 	FindIncfValue 
					(
						SR.hhwhHash,
						SR.closingStock,
						local_rec.qtyOrder,
						(SR.costingFlag [0] == 'F') ? TRUE : FALSE,
						SR.decPt
					);
		break;

	case 'S':
		workCost = FindInsfValue (SR.hhwhHash, TRUE);
		break;
	}

	if (workCost < 0.00)
	{
		workCost = 	FindIneiCosts
					(
						"L",
						comm_rec.est_no,
						SR.hhbrHash
					);
	}
	SR.costPrice = no_dec (CENTS (workCost));

	return (EXIT_SUCCESS);
}

int
InputRes (
 void)
{
	int		i;
	int		fs_flag = FALSE;
	int		displayed = FALSE;
	float	wk_qty;
	char	val_keys [300];
	char	disp_str [300];

	cc = 0;


	if (envSoOverrideQty [0] == 'Y')
	{	
		strcpy (val_keys, "OoCcNnAaRr");
		sprintf (disp_str,ML (mlCmMess152),
				ta [8], ta [9], ta [8], ta [9],
				ta [8], ta [9], ta [8], ta [9]);
	}
	else
	{
		strcpy (val_keys, "CcNnAaRr");
		sprintf (disp_str,ML ("%s (C)ancel%s %s (R)educe%s %sDisp (N) (A)%s  "),
				ta [8], ta [9],
				ta [8], ta [9], ta [8], ta [9]);
	}
			
	if (strcmp (inmr_rec.alternate, sixteenSpaces))
	{
		/*sprintf (err_str, "%s (S)ubstitute%s  ", ta [8], ta [9]);*/
		sprintf (err_str,ML (mlCmMess186), ta [8], ta [9]);

		strcat (val_keys, "Ss");
		strcat (disp_str, err_str);
	}

	if (BO_OK)
	{
		if (FULL_BO)
		{
			sprintf (err_str, ML (mlCmMess187), ta [8], ta [9]);
			strcat (disp_str, err_str);
			strcat (val_keys, "Ff");
		}
		else
		{
			/*sprintf (err_str,"%s (B)ackorder bal%s  %s (F)orce b/o%s ",*/
			sprintf (err_str,ML (mlCmMess188),
				ta [8], ta [9], ta [8], ta [9]);
			strcat (disp_str, err_str);
			strcat (val_keys, "BbFf");
		}
	}

	while (1)
	{
		i = prmptmsg (disp_str, val_keys, 1, PRMPT_LINE);

		switch (i)
		{
		/*
		 * Accept Quantity input.
		 */
		case	'O':
		case	'o':
			break;

		case	'B':
		case	'b':
			local_rec.qtyBackorder = local_rec.qtyOrder;
			local_rec.qtyOrder = ToLclUom (SR.qtyAvailable);
			if (local_rec.qtyOrder < 0.00)
				local_rec.qtyOrder = 0.00;

			local_rec.qtyBackorder -= local_rec.qtyOrder;
			SR.qtyOrder = local_rec.qtyOrder;
			SR.qtyBackorder = local_rec.qtyBackorder;
			DSP_FLD ("qtyOrder");
			DSP_FLD ("qtyBackorder");
			fs_flag = TRUE;
			if (cmrd_rec.sale_price == 0.00)
				skip_entry = goto_field (label ("qtyOrder"), label ("sale_price"));
			else
				skip_entry = goto_field (label ("qtyOrder"), label ("serialNo"));

			SetStatus (label ("qtyBackorder"));
			break;

		/*
		 * Cancel Quantity input and check if log to lost sale.
		 */
		case	'C':
		case	'c':
			local_rec.qtyOrder = 0.00;
			SR.qtyOrder = local_rec.qtyOrder;
			skip_entry = goto_field (label ("qtyOrder"),
				                label ("costhd"));
			if (prog_status == ENTRY)
				blank_display ();
			break;

		/*
		 * Display Stock Status Window.
		 */
		case	'N':
		case	'n':
		case	'A':
		case	'a':
#ifdef GVISION
			if (i == 'N' || i == 'n')
				DisplayStockWindow (SR.hhsiHash, local_rec.hhccHash);
			else
				DisplayStockWindow (SR.hhsiHash, 0L);
#else
			if (!wpipe_open)
			{
				if (OpenSkWin ())
					break;
			}
			if (i == 'N' || i == 'n')
			{
				fprintf (pout, 
					"%10ld%10ld\n", 
					SR.hhsiHash,
					local_rec.hhccHash);
			}
			else
				fprintf (pout,"%10ld%10ld\n",SR.hhsiHash,0L);

			fflush (pout);
			IP_READ (np_fn);
			displayed = TRUE;
#endif	/* GVISION */
			continue;

		/*
		 * Quantity has been reduced to equal quantity on hand.
		 */
		case	'R':
		case	'r':
			wk_qty = ToStdUom (local_rec.qtyOrder);
			local_rec.qtyOrder = ToLclUom (SR.qtyAvailable);
			if (local_rec.qtyOrder < 0.00)
				local_rec.qtyOrder = 0.00;
			SR.qtyOrder = local_rec.qtyOrder;
			break;

		/*
		 * Substitute Alternate number.
		 */
		case	'S':
		case	's':
			sprintf (err_str,"%s", clip (inmr_rec.alternate));

			sprintf (inmr_rec.item_no,"%-16.16s",err_str);
			if (ValidItemNo ())
				skip_entry = goto_field (label ("qtyOrder"),
				                          label ("item_no"));
			else
				skip_entry = -1;

			DSP_FLD ("item_no");
			DSP_FLD ("descr");
			break;

		case	'F':
		case	'f':
			local_rec.qtyBackorder = local_rec.qtyOrder;
			local_rec.qtyOrder = 0.00;
			SR.qtyOrder = local_rec.qtyOrder;
			SR.qtyBackorder = local_rec.qtyBackorder;
			DSP_FLD ("qtyOrder");
			DSP_FLD ("qtyBackorder");
			fs_flag = TRUE;
			if (cmrd_rec.sale_price == 0.00)
				skip_entry = goto_field (label ("qtyOrder"), label ("sale_price"));
			else
				skip_entry = goto_field (label ("qtyOrder"), label ("serialNo"));

			SetStatus (label ("qtyBackorder"));
			break;
		}
		move (1,PRMPT_LINE);
		printf ("%90.90s"," ");

		if (i != 'D' && i != 'd')
			break;
	}

	if (displayed)
	{
		scn_write (SCN_ITEMS);
		if (prog_status == ENTRY)
			lcount [SCN_ITEMS] = line_cnt + 1;
		putval (line_cnt);
		scn_display (SCN_ITEMS);
	}

	return (fs_flag);
}

#ifndef GVISION
int
OpenSkWin (
 void)
{
	np_fn = IP_CREATE (getpid ());
	if (np_fn < 0)
	{
		envWinOk = FALSE;
		return (EXIT_FAILURE);
	}
	if ((pout = popen ("so_pwindow", "w")) == 0)
	{
		envWinOk = FALSE;
		return (EXIT_FAILURE);
	}
	wpipe_open = TRUE;
	fprintf (pout, "%06d\n", getpid ());
	return (EXIT_SUCCESS);
}
#endif	/* GVISION */

int
SetStatus (
 int field)
{
	if (field == label ("qtyOrder") && 
	    local_rec.qtyOrder == 0.00 &&
	    prog_status == ENTRY)
	{
		return (FALSE);
	}

	strcpy (cmrd_rec.stat_flag, "R");

	if (local_rec.qtyOrder == 0.00 && local_rec.qtyBackorder != 0.00)
		strcpy (cmrd_rec.stat_flag, "B");

	strcpy (SR.statFlag, cmrd_rec.stat_flag);
	DSP_FLD ("ln_status");

	return (TRUE);
}

void
SrchCcmr (
 char *	keyValue)
{
	_work_open (2,0,40);

	cc = save_rec ("#No","#Warehouse Name");

	strcpy (ccmr_rec.co_no,  comm_rec.co_no);
	strcpy (ccmr_rec.est_no, comm_rec.est_no);
	sprintf (ccmr_rec.cc_no, "%2.2s", keyValue);
	cc = find_rec (ccmr, &ccmr_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (ccmr_rec.co_no,  comm_rec.co_no) &&
	       !strcmp (ccmr_rec.est_no, comm_rec.est_no) &&
	       !strncmp (ccmr_rec.cc_no, keyValue, strlen (keyValue)))
	{
		cc = save_rec (ccmr_rec.cc_no, ccmr_rec.name);
		if (cc)
			break;
		cc = find_rec (ccmr, &ccmr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	strcpy (ccmr_rec.co_no,  comm_rec.co_no);
	strcpy (ccmr_rec.est_no, comm_rec.est_no);
	sprintf (ccmr_rec.cc_no, "%2.2s", temp_str);
	cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");
}

void
SrchCmcb (
 char *	keyValue)
{
	int cc1;

	_work_open (4,0,40);
	save_rec ("#Code", "#Costhead Description");

	cmcb_rec.hhhr_hash = cmhr_rec.hhhr_hash;
	cmcb_rec.hhcm_hash = 0L;
	cc1 = find_rec (cmcb, &cmcb_rec, GTEQ, "r");
	if (cmcb_rec.hhhr_hash != cmhr_rec.hhhr_hash)
		cc1 = TRUE;

	if (!cc1)
	{
		cmcm_rec.hhcm_hash	=	cmcb_rec.hhcm_hash;
		cc = find_rec (cmcm2, &cmcm_rec, EQUAL, "r");
	}

	while (!cc && !cc1 && !strcmp (cmcm_rec.co_no, comm_rec.co_no))
	{
		cc = save_rec (cmcm_rec.ch_code, cmcm_rec.desc);
		if (cc)
			break;

		cc1 = find_rec (cmcb, &cmcb_rec, NEXT, "r");
		if (cmcb_rec.hhhr_hash != cmhr_rec.hhhr_hash)
			cc1 = TRUE;

		if (!cc1)
		{
			cmcm_rec.hhcm_hash	=	cmcb_rec.hhcm_hash;
			cc = find_rec (cmcm2, &cmcm_rec, EQUAL, "r");
		}
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (cmcm_rec.co_no, comm_rec.co_no);
	sprintf (cmcm_rec.ch_code, "%-4.4s", temp_str);
	cc = find_rec (cmcm, &cmcm_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, cmcm, "DBFIND");
}

void
SrchCmcm (
 char *	keyValue)
{
	_work_open (4,0,40);
	save_rec ("#Code", "#Costhead Description");

	strcpy (cmcm_rec.co_no, comm_rec.co_no);
	sprintf (cmcm_rec.ch_code, "%-4.4s", keyValue);
	cc = find_rec (cmcm, &cmcm_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (cmcm_rec.co_no, comm_rec.co_no) &&
	       !strncmp (cmcm_rec.ch_code, keyValue, strlen (keyValue)))
	{
		cc = save_rec (cmcm_rec.ch_code, cmcm_rec.desc);
		if (cc)
			break;

		cc = find_rec (cmcm, &cmcm_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (cmcm_rec.co_no, comm_rec.co_no);
	sprintf (cmcm_rec.ch_code, "%-4.4s", temp_str);
	cc = find_rec (cmcm, &cmcm_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, cmcm, "DBFIND");
}

void
SrchCmrh (
 char *	keyValue)
{
	char	requisitionNo [7];
	char	desc [41];

	_work_open (6,0,40);
	save_rec ("#Req No", "#Contract | Requested By ");
	strcpy (cmrh_rec.co_no, comm_rec.co_no);
	strcpy (cmrh_rec.br_no, reqBranchNo);
	cmrh_rec.req_no = atol (keyValue);
	cc = find_rec (cmrh, &cmrh_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (cmrh_rec.br_no, reqBranchNo) &&
	       !strcmp (cmrh_rec.co_no, comm_rec.co_no))
	{
		if (cmrh_rec.stat_flag [0] == 'C')
		{
			cc = find_rec (cmrh, &cmrh_rec, NEXT, "r");
			continue;
		}

		cmhr_rec.hhhr_hash	=	cmrh_rec.hhhr_hash;
		cc = find_rec (cmhr, &cmhr_rec, COMPARISON, "r");
		if (!cc)
		{
			sprintf (requisitionNo, "%06ld", cmrh_rec.req_no);
			sprintf (desc, "%-6.6s | %-20.20s", 
										cmhr_rec.cont_no, cmrh_rec.req_by);
			cc = save_rec (requisitionNo, desc);
			if (cc)
				break;
		}

		cc = find_rec (cmrh, &cmrh_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (cmrh_rec.co_no, comm_rec.co_no);
	strcpy (cmrh_rec.br_no, reqBranchNo);
	cmrh_rec.req_no = atol (temp_str);
	cc = find_rec (cmrh, &cmrh_rec, GTEQ, "r");
	if (cc)
		file_err (cc, cmrh, "DBFIND");
}

void
SrchCmhr (
 char *	keyValue)
{
	char contDesc [51];

	_work_open (6,0,40);
	save_rec ("#No", "#Contract Description");

	strcpy (cmhr_rec.co_no, comm_rec.co_no);
	strcpy (cmhr_rec.br_no, branchNo);
	sprintf (cmhr_rec.cont_no, "%-6.6s", keyValue);
	cc = find_rec (cmhr2, &cmhr_rec, GTEQ, "r");
	while (!cc && 
	       !strcmp (cmhr_rec.co_no, comm_rec.co_no) &&
	       !strcmp (cmhr_rec.br_no, branchNo) &&
	       !strncmp (cmhr_rec.cont_no, keyValue, strlen (keyValue)))
	{
		if (cmhr_rec.status [0] != 'O')
		{
			cc = find_rec (cmhr2, &cmhr_rec, NEXT, "r");
			continue;
		}

		cmcd_rec.hhhr_hash = cmhr_rec.hhhr_hash;
		strcpy (cmcd_rec.stat_flag, "D");
		cmcd_rec.line_no = 0;
		cc = find_rec (cmcd, &cmcd_rec, COMPARISON, "r");
		if (cc)
			sprintf (cmcd_rec.text, "%-70.70s", " ");
		sprintf (contDesc, "%-50.50s", cmcd_rec.text);

		cc = save_rec (cmhr_rec.cont_no, contDesc);
		if (cc)
			break;

		cc = find_rec (cmhr2, &cmhr_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (cmhr_rec.co_no, comm_rec.co_no);
	strcpy (cmhr_rec.br_no, branchNo);
	sprintf (cmhr_rec.cont_no, "%-10.10s", temp_str);
	cc = find_rec (cmhr2, &cmhr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, cmhr, "DBFIND");
}

void
SrchInsf (
 char *	keyValue,
 int	line_no)
{
	int	diff_wh = FALSE;

	_work_open (25,0,40);
	save_rec ("#      Serial Item.         ", (search_key == FN9) 
					? "#   Item Number. | Current Wh "
					: "#   Item Number. ");

	if (search_key == FN9)
		abc_selfield (insf, "insf_hhbr_id");

	insfRec.hhwh_hash = store [line_no].hhwhHash;
	insfRec.hhbr_hash = store [line_no].hhsiHash;
	strcpy (insfRec.status, "F");
	sprintf (insfRec.serial_no, "%-25.25s", keyValue);
	cc = find_rec (insf, &insfRec, GTEQ, "r");
	while (!cc && 
	       insfRec.status [0] == 'F' && 
	       !strncmp (insfRec.serial_no, keyValue, strlen (keyValue)))
	{
		if (search_key == FN9)
		{
			if (insfRec.hhbr_hash != store [line_no].hhsiHash)
			{
				cc = find_rec (insf, &insfRec, NEXT, "r");
				continue;
			}
		}
		else
		{
			if (insfRec.hhwh_hash != store [line_no].hhwhHash)
			{
				cc = find_rec (insf, &insfRec, NEXT, "r");
				continue;
			}
		}
		if (!CheckSupInsf (insfRec.serial_no,
				 store [line_no].hhsiHash,
				 line_no))
		{
			if (insfRec.hhwh_hash != store [line_no].hhwhHash)
				diff_wh = TRUE;	
			else
				diff_wh = FALSE;	

			if (search_key == FN9)
			{
				sprintf (err_str, 
					"%-16.16s | %-12.12s",
					inmr_rec.item_no,
					(diff_wh) ? " *** NO *** " : " ");
			}
			else
				sprintf (err_str,"%-16.16s",inmr_rec.item_no);

			cc = save_rec (insfRec.serial_no,err_str);
			if (cc)
				break;
		}
		cc = find_rec (insf,&insfRec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	insfRec.hhwh_hash = store [line_no].hhwhHash;
	insfRec.hhbr_hash = store [line_no].hhsiHash;
	strcpy (insfRec.status,"F");
	sprintf (insfRec.serial_no,"%-25.25s",temp_str);
	cc = find_rec (insf, &insfRec, COMPARISON, "r");
	if (cc)
		file_err (cc, insf, "DBFIND");
	
	strcpy (local_rec.serialNo, insfRec.serial_no);

	abc_selfield (insf, "insf_id_no");
}

/*
 * Check Whether A Serial Number For This Item Number
 * Has Already Been Used.				
 * Return 1 if duplicate			
 */
int
CheckSupInsf (
 char *	serialNo,
 long	hhbr_hash,
 int	line_no)
{
	int	i;
	int	no_items = (prog_status == ENTRY) ? line_cnt : lcount [SCN_ITEMS];

	for (i = 0;i < no_items;i++)
	{
		/*
		 * Ignore Current Line
		 */
		if (i == line_no)
			continue;

		/*
		 * cannot duplicate item_no/serialNo unless serial no was not input
		 */
		if (!strcmp (store [i].serialNo, twentyFiveSpaces))
			continue;

		/*
		 * Only compare serial numbers for the same item number
		 */
		if (store [i].hhsiHash == hhbr_hash)
		{
			if (!strcmp (store [i].serialNo, serialNo))
				return (EXIT_FAILURE);
		}
	}
	return (EXIT_SUCCESS);
}

/*
 * Load requisition lines. 
 */
int
LoadItems (void)
{
	float	std_cnv_fct;

	abc_selfield (inum, "inum_hhum_hash");

	/*
	 * Set screen 2 - for putval.
	 */
	scn_set (SCN_ITEMS);
	lcount [SCN_ITEMS] = 0;

	/*
	 * Read cmrd records. 
	 */
	cmrd_rec.hhrq_hash = cmrh_rec.hhrq_hash;
	cmrd_rec.line_no = 0;
	cc = find_rec (cmrd, &cmrd_rec, GTEQ, "r");
	while (!cc && cmrd_rec.hhrq_hash == cmrh_rec.hhrq_hash)
	{
		/*
		 * Lookup warehouse record.
		 */
		ccmr_rec.hhcc_hash = cmrd_rec.hhcc_hash;
		cc = find_rec (ccmr2, &ccmr_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, ccmr, "DBFIND");

		sprintf (local_rec.wh_no, "%2.2s", ccmr_rec.cc_no);

		/*
		 * Lookup costHead record.
		 */
		cmcm_rec.hhcm_hash = cmrd_rec.hhcm_hash;
		cc = find_rec (cmcm2, &cmcm_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (cmrd, &cmrd_rec, NEXT, "r");
			continue;
		}

		sprintf (local_rec.costHead, "%-4.4s", cmcm_rec.ch_code);
		sprintf (store [lcount [SCN_ITEMS]].costHeadDesc, "%-40.40s", cmcm_rec.desc);
		/*
		 * Get part number. 
		 */
		inmr_rec.hhbr_hash	=	cmrd_rec.hhbr_hash;
		cc = find_rec (inmr2, &inmr_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (cmrd, &cmrd_rec, NEXT, "r");
			continue;
		}
		inum_rec.hhum_hash	=	inmr_rec.std_uom;
		cc = find_rec (inum, &inum_rec, EQUAL, "r");
		if (cc)
			file_err (cc, "inum", "DBFIND");

		std_cnv_fct = inum_rec.cnv_fct;

		inum_rec.hhum_hash	=	cmrd_rec.hhum_hash;
		cc = find_rec (inum, &inum_rec, EQUAL, "r");
		if (cc)
			strcpy (local_rec.UOM, "    ");

		strcpy (local_rec.UOM, inum_rec.uom);
		strcpy (store [lcount [SCN_ITEMS]].UOM, inum_rec.uom);
	    store [lcount [SCN_ITEMS]].hhumHash  = cmrd_rec.hhum_hash;
		line_cnt = lcount [SCN_ITEMS];

		if (std_cnv_fct == 0.00)
			std_cnv_fct = 1;

		store [lcount [SCN_ITEMS]].cnvFct = inum_rec.cnv_fct / std_cnv_fct;
		cmrd_rec.sale_price	 *= store [lcount [SCN_ITEMS]].cnvFct;
		cmrd_rec.gsale_price *= store [lcount [SCN_ITEMS]].cnvFct;

		sprintf (store [lcount [SCN_ITEMS]].itemDesc, "%-40.40s", inmr_rec.description);
		store [lcount [SCN_ITEMS]].hhbrHash  = inmr_rec.hhbr_hash;
		store [lcount [SCN_ITEMS]].hhsiHash  = alt_hash (inmr_rec.hhbr_hash, inmr_rec.hhsi_hash);
		store [lcount [SCN_ITEMS]].weight     = inmr_rec.weight;
		store [lcount [SCN_ITEMS]].outerSize      = inmr_rec.outer_size;
		store [lcount [SCN_ITEMS]].decPt      = inmr_rec.dec_pt;
		store [lcount [SCN_ITEMS]].dfltDisc  = inmr_rec.disc_pc;
		store [lcount [SCN_ITEMS]].discPc	   = inmr_rec.disc_pc;
		store [lcount [SCN_ITEMS]].qtyOrder    = ToLclUom (cmrd_rec.qty_order);
		store [lcount [SCN_ITEMS]].qtyBackorder   = ToLclUom (cmrd_rec.qty_border);
		store [lcount [SCN_ITEMS]].qtyIssue    = ToLclUom (cmrd_rec.qty_order);
		store [lcount [SCN_ITEMS]].salePrice = cmrd_rec.sale_price;
		store [lcount [SCN_ITEMS]].costPrice = cmrd_rec.cost;
		store [lcount [SCN_ITEMS]].discPc    = cmrd_rec.disc_pc;
		store [lcount [SCN_ITEMS]].conPrice  = FALSE;
		store [lcount [SCN_ITEMS]].actSalePrice 		= cmrd_rec.sale_price;
		store [lcount [SCN_ITEMS]].regPc 		= cmrd_rec.reg_pc;
		store [lcount [SCN_ITEMS]].discA 		= cmrd_rec.disc_a;
		store [lcount [SCN_ITEMS]].discB 		= cmrd_rec.disc_b;
		store [lcount [SCN_ITEMS]].discB 		= cmrd_rec.disc_c;
		store [lcount [SCN_ITEMS]].cumulative 	= cmrd_rec.cumulative;
		store [lcount [SCN_ITEMS]].gSalePrice 	= cmrd_rec.gsale_price;
		store [lcount [SCN_ITEMS]].contStatus 	= cmrd_rec.cont_status;
		store [lcount [SCN_ITEMS]].pricingChk 	= TRUE;
		strcpy (store [lcount [SCN_ITEMS]].priceOverride, "N");
		strcpy (store [lcount [SCN_ITEMS]].discOverride, "N");
		strcpy (store [lcount [SCN_ITEMS]].sellGroup,     inmr_rec.sellgrp);
		strcpy (store [lcount [SCN_ITEMS]].category,    inmr_rec.category);
		strcpy (store [lcount [SCN_ITEMS]].bOrderFlag,      inmr_rec.bo_flag);
		strcpy (store [lcount [SCN_ITEMS]].itemClass,       inmr_rec.inmr_class);
		strcpy (store [lcount [SCN_ITEMS]].costingFlag,   inmr_rec.costing_flag);
		strcpy (store [lcount [SCN_ITEMS]].serialFlag, inmr_rec.serial_item);
		strcpy (store [lcount [SCN_ITEMS]].oldStatFlag,  cmrd_rec.stat_flag);
		strcpy (store [lcount [SCN_ITEMS]].statFlag,   cmrd_rec.stat_flag);
		strcpy (store [lcount [SCN_ITEMS]].lotControl, inmr_rec.lot_ctrl);
		strcpy (local_rec.lotControl, inmr_rec.lot_ctrl);

		/*
		 * Lookup incc record.
		 */
		incc_rec.hhcc_hash = cmrd_rec.hhcc_hash;
		incc_rec.hhbr_hash = cmrd_rec.hhbr_hash;
		cc = find_rec (incc, &incc_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, incc, "DBFIND");

		store [lcount [SCN_ITEMS]].hhwhHash = incc_rec.hhwh_hash;
		store [lcount [SCN_ITEMS]].closingStock   = incc_rec.closing_stock;
		store [lcount [SCN_ITEMS]].qtyAvailable = incc_rec.closing_stock -
					      incc_rec.backorder -
					      incc_rec.committed -
					      incc_rec.forward;

		store [lcount [SCN_ITEMS]].qtyAvailable += ToLclUom (cmrd_rec.qty_border + cmrd_rec.qty_order);

		local_rec.hhcmHash = cmrd_rec.hhcm_hash;
		local_rec.hhccHash = cmrd_rec.hhcc_hash;
		local_rec.qtyOrder   = ToLclUom (cmrd_rec.qty_order);
		local_rec.qtyBackorder  = ToLclUom (cmrd_rec.qty_border);

		if (inmr_rec.serial_item [0] == 'Y')
		{
			strcpy (store [lcount [SCN_ITEMS]].serialNo, cmrd_rec.serial_no);
			strcpy (local_rec.serialNo, cmrd_rec.serial_no);
		}
		else
		{
			sprintf (store [lcount [SCN_ITEMS]].serialNo, "%25.25s", " ");
			sprintf (local_rec.serialNo, "%25.25s", " ");
		}

		if (FLD ("LL") != ND)
		{
			cc = Load_LL_Lines
			(
				lcount [SCN_ITEMS],
				LL_LOAD_CM,
				cmrd_rec.cmrd_hash,
				store [lcount [SCN_ITEMS]].hhccHash,
				store [lcount [SCN_ITEMS]].UOM,
				store [lcount [SCN_ITEMS]].cnvFct,
				TRUE
			);
			if (cc)
			{
				cc = DisplayLL
					(										
						line_cnt,							
						tab_row + 3 + (line_cnt % TABLINES),
						tab_col + 22,						
						4,									
						store [lcount [SCN_ITEMS]].hhwhHash, 						
						store [lcount [SCN_ITEMS]].hhumHash,						
						store [lcount [SCN_ITEMS]].hhccHash,						
						store [lcount [SCN_ITEMS]].UOM,							
						store [lcount [SCN_ITEMS]].qtyOrder,
						store [lcount [SCN_ITEMS]].cnvFct,						
						TodaysDate (),
						TRUE,
						FALSE,
						store [lcount [SCN_ITEMS]].lotControl						
					);
			}
			strcpy (local_rec.LL, "Y");
		}

		putval (lcount [SCN_ITEMS]++);
		if (lcount [SCN_ITEMS] > MAXLINES)
			break;

		/*abc_unlock (cmrd);*/
		cc = find_rec (cmrd, &cmrd_rec, NEXT, "r");
	}
	/*abc_unlock (cmrd);*/

	scn_set (SCN_HEADER);

	abc_selfield (inum, "inum_uom");

	return (EXIT_SUCCESS);
}

/*
 * Load info for screen 1.
 */
int
LoadReq (void)
{
	/*
	 * Load contract number and description.
	 */
	cmhr_rec.hhhr_hash = cmrh_rec.hhhr_hash;
	cc = find_rec (cmhr, &cmhr_rec, EQUAL, "r");
	if (cc)
	{
		print_mess (ML (mlStdMess075));		
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}

	sprintf (local_rec.contractNo, "%-6.6s", cmhr_rec.cont_no);

	/*
	 * Set up description based on status.
	 */
	switch (cmrh_rec.stat_flag [0])
	{
	case 'R':
		strcpy (local_rec.requisitionDesc, twentyFiveSpaces);
		break;

	case 'B':
		strcpy (local_rec.requisitionDesc, ML ("(Backordered)          "));
		SetRequired (DISP_ONLY);
		break;

	case 'F':
		strcpy (local_rec.requisitionDesc, ML ("(Forward Requisition)  "));
		SetRequired (DISP_ONLY);
		break;

	case 'C':
		print_mess (ML (mlCmMess077));		
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}

	/*
	 * Find customer for contract
	 */
	cumr_rec.hhcu_hash	= cmhr_rec.hhcu_hash;
	cc = find_rec (cumr, &cumr_rec, COMPARISON, "r");
	if (cc)
	{
		print_mess (ML (mlStdMess021));		
		sleep (sleepTime);
		clear_mess ();
		return (EXIT_FAILURE);
	}
	
	/*
	 * Load contract decsription.
	 */
	cmcd_rec.hhhr_hash = cmhr_rec.hhhr_hash;
	strcpy (cmcd_rec.stat_flag, "D");
	cmcd_rec.line_no = 0;
	cc = find_rec (cmcd, &cmcd_rec, GTEQ, "r");
	while (!cc &&
	       cmcd_rec.hhhr_hash == cmhr_rec.hhhr_hash &&
	       !strcmp (cmcd_rec.stat_flag, "D") &&
	       cmcd_rec.line_no < 7)
	{
		strcpy (local_rec.contDesc [cmcd_rec.line_no], cmcd_rec.text);
		cc = find_rec (cmcd, &cmcd_rec, NEXT, "r");
	}

	scn_display (SCN_HEADER);

	return (EXIT_SUCCESS);
}

/*
 * Add warehouse record for current W/H. 
 */
int
Addincc (
	long	hhccHash)
{
	incc_rec.hhcc_hash = hhccHash;
	incc_rec.hhbr_hash = alt_hash (inmr_rec.hhbr_hash,
									  inmr_rec.hhsi_hash);
	incc_rec.hhwh_hash = 0L;
	sprintf (incc_rec.sort,"%s%11.11s%-16.16s",inmr_rec.inmr_class,
						     inmr_rec.category,
						     inmr_rec.item_no);

	incc_rec.first_stocked = local_rec.lsystemDate;
	incc_rec.closing_stock = 0.00;
	incc_rec.committed = 0.00;
	incc_rec.backorder = 0.00;

	strcpy (incc_rec.ff_option, "A");
	strcpy (incc_rec.ff_method, "A");
	strcpy (incc_rec.abc_code,  "A");
	strcpy (incc_rec.abc_update,"Y");
	
	strcpy (incc_rec.stat_flag,"0");
	
	cc = abc_add (incc, &incc_rec);
	if (cc) 
		return (cc);

	incc_rec.hhcc_hash = hhccHash;
	incc_rec.hhbr_hash = alt_hash (inmr_rec.hhbr_hash, inmr_rec.hhsi_hash);
	cc = find_rec (incc, &incc_rec, COMPARISON, "r");
	return (cc);
}

void
SetRequistion (void)
{
	strcpy (cmrh_rec.full_supply, "N");
	strcpy (cmrh_rec.printed,     "N");
	strcpy (cmrh_rec.stat_flag,   "R");
}

int
Update (void)
{
	int		prntCounter		= 0,
			newCmrd			= 0,
			allBackorder  	= 0,
			forceUpdate 	= FALSE;

	/*
	 * Only status R requisitions can have had information modified.
	 */
	if (!newReq && cmrh_rec.stat_flag [0] != 'R')
		return (FALSE);

	/*
	 * Clear screen and ready for scn SCN_HEADER processing. 
	 */
	clear ();
	scn_set (SCN_HEADER);

	/*
	 * Get date/time/op_id information
	 */
	sprintf (cmrh_rec.op_id, "%-14.14s", envLogname);

	cmrh_rec.date_create = TodaysDate ();
	cmrh_rec.time_create = atot (TimeHHMM ());

	/*
	 * No lines on requisition
	 */
	if (newReq && lcount [SCN_ITEMS] == 0)
	{
		print_at (prntCounter++, 0, ML (mlCmMess030));
		PauseForKey (prntCounter++, 0, ML (mlStdMess042), 0);
	    return (EXIT_FAILURE);
	}

	/*
	 * New Requisition
	 */
	if (newReq) 
	{
	    /*
	     * Generate order number
	     */
	    cmrh_rec.req_no = GenReqNumber ();

	    /*
	     * Put information into cmrh. 
	     */
	    strcpy (cmrh_rec.co_no, comm_rec.co_no);
	    strcpy (cmrh_rec.full_supply, (cmrh_rec.full_supply [0] == 'Y') ? "Y" : "N");
	    strcpy (cmrh_rec.stat_flag, "R");
	    cmrh_rec.hhhr_hash = cmhr_rec.hhhr_hash;

	    /*
	     * Forward Order Overwrites all others.
	     */
	    if (cmrh_rec.rqrd_date > local_rec.lsystemDate)
			strcpy (cmrh_rec.stat_flag, "F");

	    /*
	     * Add cmrh record.
	     */
	    strcpy (cmrh_rec.br_no, reqBranchNo);
	    cc = abc_add (cmrh, &cmrh_rec);
	    if (cc) 
		    file_err (cc, cmrh, "DBADD");
	
	    /*
	     * Read back cmrh record.
	     */
	    cc = find_rec (cmrh, &cmrh_rec, COMPARISON, "u");
	    if (cc)
		    file_err (cc, cmrh, "DBFIND");

	    /*
	     * Display Contract Number Created
	     */
		print_at (prntCounter++, 0, ML (mlCmMess078), cmrh_rec.req_no);

	    if (overrideManual)
			print_at (prntCounter++, 0, ML (mlCmMess034));

		print_at (prntCounter++, 0, ML (mlCmMess079));
	}
	/*
	 * Set screen 2 for processing.
	 */
	scn_set (SCN_ITEMS);

	/*
	 * Full supply requisition 
	 */
	if (cmrh_rec.full_supply [0] == 'Y' && NoneOnBackorder ())
		strcpy (cmrh_rec.full_supply, "N");

	if (cmrh_rec.full_supply [0] == 'Y' && newReq)
	{
	    /*
	     * Check if all lines are backorded.
	     */
	    for (line_cnt = 0; line_cnt < lcount [SCN_ITEMS]; line_cnt++) 
	    {
			getval (line_cnt);
			if (local_rec.qtyOrder != 0.00)
			{
				local_rec.qtyBackorder += local_rec.qtyOrder;
				local_rec.qtyOrder = 0.00;
				putval (line_cnt);
			}
	    }
	}

	/*
	 * Process cmrd records.
	 */
	for (line_cnt = 0; line_cnt < lcount [SCN_ITEMS]; line_cnt++) 
	{
	    /*
	     * Find cmrd record if it exists 
	     */
	    cmrd_rec.hhrq_hash = cmrh_rec.hhrq_hash;
	    cmrd_rec.line_no = line_cnt;
	    newCmrd = find_rec (cmrd, &cmrd_rec, COMPARISON, "u");

	    /*
	     * Get info from tabular screen.
	     */
	    getval (line_cnt);

	    /*
	     * Find inventory record. 
	     */
		inmr_rec.hhbr_hash	= SR.hhbrHash;
	    cc = find_rec (inmr2, &inmr_rec, COMPARISON, "r");
	    if (cc)
			file_err (cc, inmr, "DBFIND");

	    /*
	     * Put info into cmrd record.
	     */
	    cmrd_rec.hhrq_hash 		= cmrh_rec.hhrq_hash;
	    cmrd_rec.line_no 		= line_cnt;
		
	    cmrd_rec.hhcm_hash  	= local_rec.hhcmHash;
	    cmrd_rec.hhbr_hash  	= inmr_rec.hhbr_hash;
	    cmrd_rec.hhcc_hash  	= local_rec.hhccHash;
	    cmrd_rec.hhum_hash  	= SR.hhumHash;
	    cmrd_rec.qty_order 		= ToStdUom (local_rec.qtyOrder);
	    cmrd_rec.qty_border   	= ToStdUom (local_rec.qtyBackorder);
	    cmrd_rec.cost       	= SR.costPrice;
		cmrd_rec.reg_pc			= SR.regPc;
		cmrd_rec.disc_a			= SR.discA;
		cmrd_rec.disc_b			= SR.discB;
		cmrd_rec.disc_c			= SR.discC;
		cmrd_rec.cumulative		= SR.cumulative;
		cmrd_rec.sale_price		= SR.salePrice / SR.cnvFct;
		cmrd_rec.gsale_price	= SR.gSalePrice / SR.cnvFct;
		cmrd_rec.cont_status	= SR.contStatus;
	    cmrd_rec.due_date   	= cmrh_rec.rqrd_date;
	    sprintf (cmrd_rec.item_desc, "%-40.40s", inmr_rec.description);
	    sprintf (cmrd_rec.serial_no, "%-25.25s", SR.serialNo);

	    /*
	     * Serial item.
	     */
	    if (SR.serialFlag [0] == 'Y')
	    {
			if (newCmrd)
				CommitInsf (line_cnt, cmrd_rec.serial_no);
			else
				IntUpdateInsf (line_cnt, SR.serialNo);
	    }

	    /*
	     * New lines should be R.  This will be       
	     * changes if order is Forward or Backordered. 
	     */
	    strcpy (cmrd_rec.stat_flag, "R");

	    /*
	     * Header is Forward so all lines are forward.
	     */
	    if (cmrh_rec.stat_flag [0] == 'F')
	    {
			strcpy (cmrd_rec.stat_flag, "F");
			cmrd_rec.qty_order += cmrd_rec.qty_border;
			cmrd_rec.qty_border = 0.00;
	    }

	    /*
	     * If line is fully backordered then stat_flag is B.
	     */
	    if (cmrd_rec.qty_order == 0.00 && cmrd_rec.qty_border != 0.00)
			strcpy (cmrd_rec.stat_flag, "B");

	    /*
	     * If line was complete leave it as a C. 
	     */
	    if (SR.oldStatFlag [0] == 'C' && !newReq)
			strcpy (cmrd_rec.stat_flag, "C");

	    if (!newCmrd)
	    {
			/*
			 * Update existing order. 
			 */
			cc = abc_update (cmrd, &cmrd_rec);
			if (cc) 
				file_err (cc, cmrd, "DBUPDATE");
	    }
	    else 
	    {
			/*
			 * Put information into cmrd. 
			 */
			cmrd_rec.qty_iss = 0.00;

			cc = abc_add (cmrd, &cmrd_rec);
			if (cc) 
				file_err (cc, cmrd, "DBADD");

	    	cmrd_rec.hhrq_hash = cmrh_rec.hhrq_hash;
	    	cmrd_rec.line_no = line_cnt;
	    	cc = find_rec (cmrd, &cmrd_rec, COMPARISON, "r");
			if (cc)
				file_err (cc, cmrd, "DBFIND");
	    }

	    if (cmrd_rec.stat_flag [0] == 'B')
			allBackorder++;

	    /*
	     * Add sobg record for recalc of stock
	     */
	    add_hash (comm_rec.co_no, comm_rec.est_no, "RC", 0, 
		     cmrd_rec.hhbr_hash, cmrd_rec.hhcc_hash,0L,
		    (double) 0.00);

		if (SK_BATCH_CONT || MULT_LOC)
		{
			AllocLotLocation 
			(
				line_cnt,
				TRUE,
				LL_LOAD_CM,
				cmrd_rec.cmrd_hash
			);
		}
	}
	abc_selfield (inmr,"inmr_id_no");

	/*
	 * Delete extraneous lines. 
	 */
	cmrd_rec.hhrq_hash = cmrh_rec.hhrq_hash;
	cmrd_rec.line_no = line_cnt;
	cc = find_rec (cmrd, &cmrd_rec, GTEQ, "u");
	while (!cc && cmrd_rec.hhrq_hash == cmrh_rec.hhrq_hash)
	{
	    cc = abc_delete (cmrd);
	    if (cc) 
			file_err (cc, cmrd, "DBDELETE");

	    cmrd_rec.hhrq_hash = cmrh_rec.hhrq_hash;
	    cmrd_rec.line_no = line_cnt;
	    cc = find_rec (cmrd, &cmrd_rec, GTEQ, "u");
	}
	abc_unlock (cmrd);

	/*
	 * Update existing order header. 
	 */
	if (!newReq) 
	{	
	    /*
	     * Delete cancelled order.
	     */
	    if (lcount [SCN_ITEMS] == 0) 
	    {
			print_at (prntCounter++, 0, ML (mlCmMess080));
			cc = abc_delete (cmrh);
			if (cc)
				file_err (cc, cmrh, "DBDELETE");
	    }
	}

	if (allBackorder == lcount [SCN_ITEMS] && lcount [SCN_ITEMS] != 0)
	{
	    strcpy (cmrh_rec.stat_flag, "B");
	    forceUpdate = TRUE;
		print_at (prntCounter++, 0, ML (mlCmMess081));
	}
	if (cmrh_rec.stat_flag [0] == 'F')
		print_at (prntCounter++, 0, ML (mlCmMess082));
	
	if ((!newReq && lcount [SCN_ITEMS] != 0) || forceUpdate) 
	{
	    /*
	     * Just update stat flag and rewrite.
	     */
	    cc = abc_update (cmrh, &cmrh_rec);
	    if (cc) 
			file_err (cc, cmrh, "DBUPDATE");
	}
	
	abc_unlock (cmrh);

	if (lcount [SCN_ITEMS] != 0)
	{
		/*
		 * Call requisition print program. 
		 */
		sprintf (err_str,"cm_req_prt %d %010ld", printerNo, cmrh_rec.hhrq_hash);
		sys_exec (err_str);
	}
		
	recalc_sobg ();

	/*
	 * Wait for user to hit a key
	 */
	PauseForKey (prntCounter++, 0, ML (mlStdMess042), 0);

	sprintf (local_rec.previousRef, "%06ld", cmrh_rec.req_no);

	return (TRUE);
}

/*
 * Generate next requisition number. 
 */
long
GenReqNumber (void)
{
	long next_man_no	=	-1L;

	overrideManual = FALSE;
	/*
	 * Check manually entered req no.
	 */
	if (local_rec.requisitionNo != 0L)
	{
		if (envCmAutoReq == MANUAL)
		{
			/*
			 * Check manually entered req no If used then increment until unused
			 * req no is found.  If different from manually entered req no set 
			 * the overrideManual flag.                
			 */
			next_man_no = local_rec.requisitionNo;
			while (!CheckReqNumber (++next_man_no));

			if (next_man_no != local_rec.requisitionNo)
				overrideManual = TRUE;

			return (next_man_no);
		}
		else
		{
			/*
			 * Check manually entered req no.  If used then auto generate a 
			 * req no and set the overrideManual flag.  
			 */
			if (CheckReqNumber (local_rec.requisitionNo))
				return (local_rec.requisitionNo);
			else
				overrideManual = TRUE;
		}
	}

	switch (envCmAutoReq)
	{
	case BRANCH:
		strcpy (esmr_rec.co_no,  comm_rec.co_no);
		strcpy (esmr_rec.est_no, comm_rec.est_no);
		cc = find_rec (esmr, &esmr_rec, COMPARISON, "u");
		if (cc)
		    file_err (cc, esmr, "DBFIND");

		/*
		 * Check if Order No Already Allocated If it has been then skip		
		 */
		while (!CheckReqNumber (++esmr_rec.nx_requis_no));

		cc = abc_update (esmr,&esmr_rec);
		if (cc)
			file_err (cc, esmr, "DBUPDATE");

		return (esmr_rec.nx_requis_no);

	case COMPANY:
		strcpy (comr_rec.co_no,  comm_rec.co_no);
		cc = find_rec (comr, &comr_rec, COMPARISON, "u");
		if (cc)
		    file_err (cc, comr, "DBFIND");

		/*
		 * Check if Order No Already Allocated. If it has been then skip.
		 */
		while (!CheckReqNumber (++comr_rec.nx_requis_no));

		cc = abc_update (comr, &comr_rec);
		if (cc)
			file_err (cc, comr, "DBUPDATE");

		return (comr_rec.nx_requis_no);
	}
	return (next_man_no);
}

int
CheckReqNumber (
	long	requisition_no)
{
	strcpy (cmrh2_rec.co_no, comm_rec.co_no);
	strcpy (cmrh2_rec.br_no, reqBranchNo);
	cmrh2_rec.req_no = requisition_no;
	cc = find_rec (cmrh2, &cmrh2_rec, COMPARISON, "r");
	if (cc)
		return (TRUE);

	return (FALSE);
}

/*
 * Check if all lines are backorded.
 */
int
NoneOnBackorder (void)
{
	int	i;

	for (i = 0; i < lcount [SCN_ITEMS]; i++) 
	{
		if (store [i].qtyBackorder != 0.00)
			return (FALSE);
	}
	return (TRUE);
}

void
IntUpdateInsf (
	int		lineNo,
	char 	*serialNo)
{
	/*
	 *	line being deleted
	 */
	if (local_rec.qtyOrder == 0.00)
	{
		/*
		 *	free old serial number
		 */
		if (strcmp (serialNo, twentyFiveSpaces))
			FreeInsf (lineNo, serialNo);

		/*
		 *	free new serial number	
		 */
		if (strcmp (cmrd_rec.serial_no, twentyFiveSpaces))
			FreeInsf (lineNo, cmrd_rec.serial_no);

		return;
	}

	/*
	 *	if serial number has changed
	 */
	if (strcmp (serialNo, cmrd_rec.serial_no))
	{
		/*
		 *	free old serial number	
		 */
		if (strcmp (serialNo, twentyFiveSpaces))
			FreeInsf (lineNo, serialNo);

		/*
		 *	commit new serial number	
		 */
		if (strcmp (cmrd_rec.serial_no, twentyFiveSpaces))
			CommitInsf (lineNo, cmrd_rec.serial_no);
	}
}

/*
 *	Free insf record
 */
void
FreeInsf (
	int		line_no,
	char 	*serialNo)
{
	if (!strcmp (serialNo,twentyFiveSpaces))
		return;

	/*
	 * serial_item and serial number input	
	 */
	if (store [line_no].hhwhHash > 0L)
	{
		cc = UpdateInsf (store [line_no].hhwhHash, 0L, serialNo, "C", "F");
		if (cc && cc < 1000)
			file_err (cc, insf, "DBUPDATE");
	}
}

/*
 *	Commit insf record		
 */
void
CommitInsf (
	int		lineNo,
	char 	*serialNo)
{
	if (!strcmp (serialNo,twentyFiveSpaces))
		return;

	/*
	 * serial_item and serial number input	
	 */
	if (store [lineNo].hhwhHash > 0L)
	{
		cc = UpdateInsf (store [lineNo].hhwhHash, 0L, serialNo, "F", "C");
		if (cc && cc < 1000)
			file_err (cc, insf, "DBUPDATE");
	}
}


void
tab_other (
	int		line_no)
{
	if (cur_screen != 2)
		return;

	print_at (5, 2, ML (mlCmMess092));
	print_at (5, 60, ML (mlCmMess093));

	/*
	 * Disable/Enable fields depending on status
	 */
	if ((store [line_no].oldStatFlag [0] == 'R' || 
	     line_no >= lcount [SCN_ITEMS] ||
	     newReq) &&
	    (cmrh_rec.stat_flag [0] !='F' && cmrh_rec.stat_flag [0] != 'B'))
	{
		FLD ("costhd")   		= YES;
		FLD ("wh_no")    		= YES;
		FLD ("item_no")  		= YES;
		FLD ("qtyOrder")  		= YES;
		FLD ("qtyBackorder") 	= YES;
		FLD ("sale_price")  	= NO;
		FLD ("disc_pc")  		= NO;


		if (store [line_no].serialFlag [0] == 'Y')
		{
			if (!F_HIDE (label ("serialNo")))
				FLD ("serialNo") = YES;
		}
		else
		{
			if (!F_HIDE (label ("serialNo")))
				FLD ("serialNo") = NA;
		}
	}
	else
	{
		FLD ("costhd")   		= NA;
		FLD ("wh_no")    		= NA;
		FLD ("item_no")  		= NA;
		FLD ("qtyOrder")  		= NA;
		FLD ("qtyBackorder") 	= NA;
		FLD ("sale_price")  	= NA;
		FLD ("disc_pc")  		= NA;
		if (!F_HIDE (label ("serialNo")))
			FLD ("serialNo") 	= NA;
	}

	/*
	 * Print description for costHead and item.
	 */
	if (line_no >= lcount [SCN_ITEMS] && prog_status != ENTRY)
	{
		print_at (5, 20, "%-40.40s", " ");
		print_at (5, 80, "%-40.40s", " ");
	}
	else
	{
		print_at (5, 20, "%-40.40s", store [line_no].costHeadDesc);
		print_at (5, 80, "%-40.40s", store [line_no].itemDesc);
	}

	return;
}

void
PrintCoStuff (void)
{
	print_at (21, 0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
	print_at (22, 0, ML (mlStdMess039), comm_rec.est_no, comm_rec.est_name);
}


/*
 * Save vars [?].required values. 
 */
int
SaveRequired (void)
{
	int	i;

	num_flds = 0;
	for (i = 0; i < MAX_FLDS && vars [i].scn != 0; i++, num_flds++)
		fieldRequired [i] = vars [i].required;

	return (EXIT_SUCCESS);
}

/*
 * Set vars [?].required values. 
 */
int
SetRequired (
	int		setType)
{
	int	i;

	for (i = 1; i < num_flds; i++) /* Skip first field */
	{
		/*
		 * Skip ND fields. 
		 */
		if (vars [i].required == ND)
			continue;

		/*
		 * Set required values. 
		 */
		if (setType == NORMAL)
			vars [i].required = fieldRequired [i];
		else
			vars [i].required = NA;
	}

	return (EXIT_SUCCESS);
}

/*
 * Search on UOM (inum)     
 */
void
SrchInum (
	char	*keyValue)
{
    _work_open (4,0,40);
    save_rec ("#UOM ","#Description");

    strcpy (inum2_rec.uom_group, inum_rec.uom_group);
    strcpy (inum2_rec.uom, keyValue);
    cc = find_rec (inum2, &inum2_rec, GTEQ, "r");
    while (!cc && !strcmp (inum2_rec.uom_group, inum_rec.uom_group))
    {
        if (strncmp (inum2_rec.uom_group, keyValue, strlen (keyValue)))
        {
            cc = find_rec (inum2, &inum2_rec, NEXT, "r");
            continue;
        }

		if (!ValidItemUom (SR.hhbrHash, inum2_rec.hhum_hash))
		{
			cc = save_rec (inum2_rec.uom,inum2_rec.desc);
			if (cc)
				break;
		}

        cc = find_rec (inum2, &inum2_rec, NEXT, "r");
    }

    cc = disp_srch ();
    work_close ();
    if (cc)
            return;

    strcpy (inum2_rec.uom_group, inum_rec.uom_group);
    strcpy (inum2_rec.uom, keyValue);
    cc = find_rec (inum2, &inum2_rec, COMPARISON, "r");
    if (cc)
            file_err (cc, inum2, "DBFIND");
}

/*
 * To standard unit of measure 
 */
float
ToStdUom (
 float   lclQty)
{
    float   cnvQty;

    if (F_HIDE (label ("UOM")))
        return (lclQty);

    if (SR.cnvFct == 0.00 || lclQty == 0.00)
        return (0.00);

    cnvQty = lclQty * SR.cnvFct;

    return (cnvQty);
}

/*
 * To local unit of measure 
 */
float
ToLclUom (
 float   lclQty)
{
    float   cnvQty;

    if (F_HIDE (label ("UOM")))
        return (lclQty);

    if (SR.cnvFct == 0.00 || lclQty == 0.00)
        return (0.00);

    cnvQty = lclQty / SR.cnvFct;

    return (cnvQty);
}

int
heading (
	int scn)
{
	if (restart)
		return (EXIT_SUCCESS);

	if (scn != cur_screen)
		scn_set (scn);

	swide ();
	clear ();

	line_at (1,0,130);

	rv_pr (ML (mlCmMess083), (130 - strlen (ML (mlCmMess083))) / 2, 0, 1);

	print_at (0,100,ML (mlCmMess088), local_rec.previousRef);

	if (scn == SCN_HEADER)
	{
		box (0, 2, 130, 17);

		line_at (12,1,129);
		line_at (15,1,129);
	}

	if (scn == 2)
	{
		box (0, 3, 132, 2);
		if (local_rec.requisitionNo == 0L)
			print_at (4, 2, ML (mlCmMess089));
		else
			print_at (4, 2, ML (mlCmMess090), local_rec.requisitionNo);
		print_at (4, 60, ML (mlCmMess091), local_rec.contractNo);

		print_at (5, 2, ML (mlCmMess092));
		print_at (5, 20, "%-40.40s", local_rec.costHeadDesc);
		print_at (5, 60, ML (mlCmMess093));

		if (prog_status != ENTRY)
			tab_other (0);

		line_at (20,0,130);

	}

	PrintCoStuff ();

	line_cnt = 0;
	scn_write (scn);
	return (EXIT_SUCCESS);
}

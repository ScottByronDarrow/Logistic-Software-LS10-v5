/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: cm_des_conf.c,v 5.4 2002/07/24 08:38:42 scott Exp $
|  Program Name  : (cm_des_conf.c)  
|  Program Desc  : (Contract Management Requisition Despatch)
|---------------------------------------------------------------------|
|  Author        : Simon Dubey.    | Date Written : 05/03/93          |
|---------------------------------------------------------------------|
| $Log: cm_des_conf.c,v $
| Revision 5.4  2002/07/24 08:38:42  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.3  2002/07/03 04:21:39  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.2  2002/01/17 09:35:17  scott
| Updated as first phase of contract management cleanup on it's way to becoming
| useable for 3PL.
|
| Revision 5.1  2002/01/17 08:33:21  scott
| Updated as first phase of contract management cleanup on it's way to becoming
| useable for 3PL.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cm_des_conf.c,v $", 
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CM/cm_des_conf/cm_des_conf.c,v 5.4 2002/07/24 08:38:42 scott Exp $";

#define	TABLINES	10

#include <pslscr.h>
#include <ml_cm_mess.h>
#include <ml_std_mess.h>
#include <GlUtils.h>
#include <twodec.h>
#include <proc_sobg.h>
#include <LocHeader.h>
#include <Costing.h>

#ifdef	GVISION
#include <StockWindow.h>
#endif

#define	MANUAL		0
#define	BRANCH		1
#define	COMPANY		2

#define	SR		store [line_cnt]

#define	NON_STOCK	(SR.itemClass [0] == 'Z')
#define	NO_COST		(SR.itemClass [0] == 'N')
#define	E_PHANTOM	(SR.itemClass [0] == 'P' && prog_status != ENTRY)
#define	SER_COSTING	(SR.costingFlag [0] == 'S')
#define SERIAL		(SR.serialFlag [0] == 'Y')
#define	BLANK_SER	(!strcmp (local_rec.serialNo, twentyFiveSpace))
#define	FIFO		(inmr_rec.costing_flag [0] == 'F')
#define	LIFO		(inmr_rec.costing_flag [0] == 'L')
#define	ENTER_DATA	(prog_status == ENTRY)

#define	MAX_SUPER	500

#include <MoveRec.h>
		
#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct cmtrRecord	cmtr_rec;
struct cumrRecord	cumr_rec;
struct cmcbRecord	cmcb_rec;
struct cmcmRecord	cmcm_rec;
struct cmrdRecord	cmrd_rec;
struct cmrhRecord	cmrh_rec;
struct ccmrRecord	ccmr_rec;
struct cmhrRecord	cmhr_rec;
struct cmcdRecord	cmcd_rec;
struct inccRecord	incc_rec;
struct inwuRecord	inwu_rec;
struct inmrRecord	inmr_rec;
struct inumRecord	inum_rec;
struct inumRecord	inum2_rec;

	FILE	*pout, 
			*pp;

	char    envSoOverride [2];

	int		UPDATE_OK 			= FALSE, 
			envSoPermWin 		= FALSE, 
			envCmAutoReq		= 0, 
			envWinOk			= 0, 
			envGlByClass 		= TRUE, 
			envSerialItemsOk	= FALSE,
			np_fn				= 0, 
			printerNo			= 0, 
			wpipeOpen 			= FALSE, 
			pipeOpen 			= FALSE, 
			glwkUpdate 			= FALSE, 
			processID			= 0, 
			sup_flag			= 0, 
			alt_flag			= 0, 
			sup_counter 		= FALSE, 
			alternate_ok 		= TRUE;

	char	sup_part [17], 
			alt_part [17], 
			envCurrCode [4], 
			*curr_user, 
			*envSkIvalClass, 
			*result;

	char	*data  = "data", 
			*inum2 = "inum2", 
			*twentyFiveSpace = "                         ", 
			*sixteen_space = "                ";

struct storeRec {
		long	hhbrHash;
		long	hhcmHash;
		long	hhccHash;
		long	hhumHash;
		long	hhwhHash;
		long	creditAccountHash;

		char	itemClass [2];
		char	costingFlag [2];
		char	serialFlag [2];
		char	errFound [2];
		char	serialNo [26];
		char	orgSerialNo [26];
		char	lotControl [2];
		char	creditAccount [MAXLEVEL + 1];
		char	UOM [5];

		float	closing;
		float	avail;
		float	qtyOrder;
		float	qtyBackorder;
		float	qtyIssued;
		float	cnvFct;		/* Conversion Factor.	      		*/

		double	cost;

		int		line_no;
		int		dec_pt;
		int		costed;
		int		lotSelectFlag;
} store [MAXLINES];

	char	rq_branchNo [3];

	extern	int		TruePosition;

/*
 * Local & Screen Structures 
 */
struct {
	char	dummy [11];
	float	qtyOrder;
	float	qtyBackorder;
	float	qtyIssued;
	char	systemDate [11];
	long	lsystemDate;
	long	requisitionNo;
	char	costhd [5];
	char	contDesc [7][71];
	char	wh_no [3];
	double	sell_pr;
	float	disc_pc;
	char	serialNo [26];
	char	item_no [17];
	char	lotControl [2];
	char	LL [2];
	char	UOM [5];
} local_rec;

static struct	var vars [] =
{
	{1, LIN, "requisitionNo", 	 4, 2, LONGTYPE, 
		"NNNNNN", "          ", 
		"0", "0", "Requisition Number   ", " ", 
		NE, NO, JUSTRIGHT, "", "", (char *)&local_rec.requisitionNo}, 
	{1, LIN, "contractNo", 	 5, 2, CHARTYPE, 
		"UUUUUU", "          ", 
		" ", "0", "Contract Number      ", " ", 
		NA, NO, JUSTLEFT, "", "", cmhr_rec.cont_no}, 
	{1, LIN, "descr1", 	 6, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Description          ", " ", 
		 NA, NO, JUSTLEFT, "", "", local_rec.contDesc [0]}, 
	{1, LIN, "descr2", 	 7, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "                     ", " ", 
		 NA, NO, JUSTLEFT, "", "", local_rec.contDesc [1]}, 
	{1, LIN, "descr3", 	 8, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "                     ", " ", 
		 NA, NO, JUSTLEFT, "", "", local_rec.contDesc [2]}, 
	{1, LIN, "descr4", 	 9, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "                     ", " ", 
		 NA, NO, JUSTLEFT, "", "", local_rec.contDesc [3]}, 
	{1, LIN, "descr5", 	 10, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "                     ", " ", 
		 NA, NO, JUSTLEFT, "", "", local_rec.contDesc [4]}, 
	{1, LIN, "descr6", 	 11, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "                     ", " ", 
		 NA, NO, JUSTLEFT, "", "", local_rec.contDesc [5]}, 
	{1, LIN, "descr7", 	 12, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "                     ", " ", 
		 NA, NO, JUSTLEFT, "", "", local_rec.contDesc [6]}, 

	{1, LIN, "req_date", 	 14, 2, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", local_rec.systemDate, "Requisition Date     ", "", 
		NA, NO, JUSTRIGHT, "", "", (char *)&cmrh_rec.req_date}, 
	{1, LIN, "rqrd_date", 	 14, 65, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", " ", "Date Required        ", "", 
		NA, NO, JUSTRIGHT, "", "", (char *)&cmrh_rec.rqrd_date}, 
	{1, LIN, "req_by", 	 15, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Requested By         ", "", 
		NA, NO, JUSTLEFT, "", "", cmrh_rec.req_by}, 
	{1, LIN, "rq_full_supply", 15, 65, CHARTYPE, 
		"U", "          ", 
		" ", "N", "Full Supply          ", "Enter Y(es) or N(o)", 
		 NA, NO, JUSTLEFT, "YN", "", cmrh_rec.full_supply}, 
	{1, LIN, "del_name", 	 17, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Delivery Name        ", " ", 
		 NA, NO, JUSTLEFT, "", "", cmrh_rec.del_name}, 
	{1, LIN, "del_addr1", 	 18, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", cmhr_rec.adr1, "Delivery Address     ", " ", 
		 NA, NO, JUSTLEFT, "", "", cmrh_rec.del_adr1}, 
	{1, LIN, "del_addr2", 	 19, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", cmhr_rec.adr2, "                     ", " ", 
		 NA, NO, JUSTLEFT, "", "", cmrh_rec.del_adr2}, 
	{1, LIN, "del_addr3", 	 20, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", cmhr_rec.adr3, "                     ", " ", 
		 NA, NO, JUSTLEFT, "", "", cmrh_rec.del_adr3}, 
	{1, LIN, "add_int1", 	 17, 65, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "Extra Instruction 1  ", " ", 
		 NA, NO, JUSTLEFT, "", "", cmrh_rec.add_int1}, 
	{1, LIN, "add_int2", 	 18, 65, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "Extra Instruction 2  ", " ", 
		 NA, NO, JUSTLEFT, "", "", cmrh_rec.add_int2}, 
	{1, LIN, "add_int3", 	 19, 65, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "Extra Instruction 3  ", " ", 
		 NA, NO, JUSTLEFT, "", "", cmrh_rec.add_int3}, 
	{2, TAB, "costhd", 	MAXLINES, 2, CHARTYPE, 
		"UUUU", "          ", 
		" ", "", "Costhead", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.costhd}, 
	{2, TAB, "wh_no", 	0, 0, CHARTYPE, 
		"AA", "          ", 
		" ", comm_rec.cc_no, "WH", "Enter warehouse number.", 
		 NA, NO, JUSTRIGHT, "", "", ccmr_rec.cc_no}, 
	{2, TAB, "item_no", 		0, 0, CHARTYPE, 
		"UUUUUUUUUUUUUUUU", "          ", 
		" ", "", "    Item no.    ", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.item_no}, 
	{2, TAB, "UOM", 	 0, 0, CHARTYPE, 
		"AAAA", "          ", 
		" ", "", "UOM.", " Unit of Measure ", 
		 NA, NO, JUSTLEFT, "", "", local_rec.UOM}, 
	{2, TAB, "qtyOrder", 	 0, 0, FLOATTYPE, 
		"NNNNN.NN", "          ", 
		" ", " ", "Qty Ord.", "", 
		NA, NO, JUSTRIGHT, "", "", (char *) &local_rec.qtyOrder}, 
	{2, TAB, "qtyIssued", 	 0, 0, FLOATTYPE, 
		"NNNNN.NN", "          ", 
		" ", " ", "Qty Des.", "", 
		YES, NO, JUSTRIGHT, "0.00", "99999.99", (char *) &local_rec.qtyIssued}, 
	{2, TAB, "qtyBackorder", 	 0, 0, FLOATTYPE, 
		"NNNNN.NN", "          ", 
		" ", " ", "Qty B/O.", "", 
		NI, NO, JUSTRIGHT, "0.00", "99999.99", (char *) &local_rec.qtyBackorder}, 
	{2, TAB, "sell_pr", 		 0, 0, DOUBLETYPE, 
		"NNNNNNNNN.NN", "          ", 
		" ", "", " Sell Price ", "", 
		NA, NO, JUSTRIGHT, "", "", (char *) &local_rec.sell_pr}, 
	{2, TAB, "disc_pc", 		 0, 0, FLOATTYPE, 
		"NNN.NN", "          ", 
		" ", "", " Disc ", "", 
		NA, NO, JUSTRIGHT, "", "", (char *) &local_rec.disc_pc}, 
	{2, TAB, "lotControl", 	 0, 0, CHARTYPE, 
		"U", "          ", 
		"", "", "", "", 
		 ND, NO, JUSTLEFT, "", "", local_rec.lotControl}, 
	{2, TAB, "LL", 	 0, 0, CHARTYPE, 
		"U", "          ", 
		" ", "N", "L", "Lot / Location Selection. <return> ", 
		 YES, NO, JUSTLEFT, "", "", local_rec.LL}, 
	{2, TAB, "serialNo", 	 0, 0, CHARTYPE, 
		"UUUUUUUUUUUUUUUUUUUUUUUUU", "          ", 
		" ", " ", "        Serial No        ", "Enter Serial Number, or [SEARCH] key. ", 
		 NA, NO, JUSTLEFT, "", "", local_rec.serialNo}, 
	{2, TAB, "ln_status", 	 0, 0, CHARTYPE, 
		"U", "          ", 
		" ", " ", "", "", 
		 NA, NO, JUSTLEFT, "", "", cmrd_rec.stat_flag}, 
	{0, LIN, "", 		 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}
};

/*
 * Function declarations
 */
float	ToLclUom		(float);
float	ToStdUom		(float);
int		CheckOK		 	(int);
int		ChkDupInsf		(char *, long, int);
int		FindCost		(void);
int		heading			(int);
int		InputRes		(void);
int		IntFindInmr		(char *, char *);
int		IntFindSuper	(char *, char *, int);
int		ItemError		(char *);
int		LoadItems		(void);
#ifndef GVISION
int		OpenSkWin		(void);
#endif
int		spec_valid		(int);
int		Update			(void);
void	AddCmtr			(void);
void	AddGlwk			(void);
void	BusyFunc		(int);
void	CalcAvailable	(void);
void	ClearWin		(void);
void	CloseAudit		(void);
void	CloseDB		 	(void);
void	Confirm			(void);
void	IntSupAltErr	(void);
void	OpenAudit		(void);
void	OpenDB			(void);
void	PrintDetails	(double);
void	SrchCmrh		(char *);
void	SrchInsf		(char *);
void	tab_other		(int);
void	UpdateFiles		(float, double);

#include	<MoveAdd.h>

/*
 * Main Processing Routine. 
 */
int
main (
 int	argc, 
 char * argv [])
{
	char	*sptr;
	int	i;

	TruePosition	=	TRUE;

	SETUP_SCR (vars);


	if (argc < 3)
	{
		print_at (0, 0, ML (mlStdMess072), argv [0]);
		return (EXIT_FAILURE);
	}

	processID = atoi (argv [1]);
	printerNo = atoi (argv [2]);

	curr_user = getenv ("LOGNAME");
	sprintf (envSoOverride, "%-1.1s", get_env ("SO_OVERRIDE_QTY"));

	/*
	 * Validate if serial items allowed.
	 */
	sptr = chk_env ("SK_SERIAL_OK");
	envSerialItemsOk = (sptr == (char *)0) ? FALSE : atoi (sptr);

	FLD ("serialNo") = (envSerialItemsOk) ? YES : ND;

	tab_col = (envSerialItemsOk) ? 16 : 30;
	/*
	 * Check if stock information window is loaded at load time. 
	 */
	sptr = chk_env ("SO_PERM_WIN");
	envSoPermWin = (sptr == (char *)0) ? 0 : atoi (sptr);

#ifndef GVISION
	if (envSoPermWin)
	{
		if (OpenSkWin ())
			envWinOk = FALSE;
	}
#endif

	sptr = chk_env ("GL_BYCLASS");
	envGlByClass = (sptr == (char *)0) ? TRUE : atoi (sptr);

	sptr = chk_env ("WIN_OK");
	envWinOk = (sptr == (char *)0) ? 1 : atoi (sptr);

	sptr = chk_env ("SK_IVAL_CLASS");
	if (sptr)
		envSkIvalClass = strdup (sptr);
	else
		envSkIvalClass = "ZKPN";

	upshift (envSkIvalClass); 

	for (i = 0; i < MAXLINES; i++)
	{
		strcpy (store [i].serialNo,    twentyFiveSpace);
		strcpy (store [i].orgSerialNo, twentyFiveSpace);
		store [i].costed = FALSE;
	}

	/*
	 * Check contract number level. 
	 */
	sptr = chk_env ("CM_AUTO_REQ");
	envCmAutoReq = (sptr == (char *)0) ? 2 : atoi (sptr);

	tab_col = 5;
	tab_row = 8;

	init_scr 	();		/*  sets terminal from termcap	*/
	set_tty 	();		/*  get into raw mode		*/
	set_masks 	();		/*  setup print using masks	*/

/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (2, store, sizeof (struct storeRec));
#endif
	init_vars 	(1);		/*  set default values		*/

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));
	local_rec.lsystemDate = TodaysDate ();

	/*
	 * Read comm record and open database. 
	 */
	OpenDB ();
	OpenAudit ();

	FLD ("LL") = ND;
	if (SK_BATCH_CONT || MULT_LOC)
		FLD ("LL") = YES;

	strcpy (rq_branchNo, (envCmAutoReq == COMPANY) ? " 0" : comm_rec.est_no);
	while (prog_exit == 0)
	{
		entry_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;
		edit_exit 	= FALSE;
		prog_exit 	= FALSE;
		init_vars (1);
		init_vars (2);
		lcount [2] = 0;

		/*
		 * Enter screen 1 linear input. 
		 * Turn screen initialise on.  
		 */
		init_ok = TRUE;
		eoi_ok 	= FALSE;

		heading (1);
		entry (1);
		scn_display (1);

		if (prog_exit || restart)
			continue;

		last_char = prmptmsg (ML (mlCmMess166), "YyNn", 0, 2);

		if (last_char == 'N' || last_char == 'n')
		{
			init_ok = FALSE;
			eoi_ok 	= FALSE;
			heading (2);
			scn_display (2);
			entry (2);
			if (restart)
				continue;
		
			eoi_ok = TRUE;
		}
		else
			Confirm ();

		edit_all ();

		if (restart)
			continue;

		/*
		 * Check for blank Serial items. 
		 */
		while (CheckOK (TRUE))
		{
			sprintf (err_str, ML (mlCmMess159), CheckOK (TRUE));
			errmess (err_str);
			edit_all ();
			if (restart)
				break;
		}

		if (restart)
			continue;

		Update ();
	}

#ifdef GVISION
	CloseStockWindow ();
#else
	if (wpipeOpen)
	{
		pclose (pout);
		IP_CLOSE (np_fn);
		IP_UNLINK (getpid ());
	}
#endif	/* GVISION */

	if (pipeOpen)
		CloseAudit ();

	CloseDB (); 
	FinishProgram ();
	return (EXIT_FAILURE);
}

int
spec_valid (
 int	field)
{
	int	i;
	int		TempLine;
	int		TempLineCnt;

	/*
	 * Validate Requisition Number. 
	 */
	if (LCHECK ("requisitionNo"))
	{
		strcpy (cmrh_rec.full_supply, "N");

		if (SRCH_KEY)
		{
			SrchCmrh (temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used || local_rec.requisitionNo == 0L)
		{
			print_mess (ML (mlCmMess026));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (cmrh_rec.co_no, comm_rec.co_no);
		strcpy (cmrh_rec.br_no, rq_branchNo);
		cmrh_rec.req_no = local_rec.requisitionNo;

		cc =  find_rec (cmrh, &cmrh_rec, COMPARISON, "r");

		if (cc)
		{
			print_mess (ML (mlCmMess015));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		
		/*
		 * Forward order.
		 */
		if (cmrh_rec.stat_flag [0] == 'F')
		{
			print_mess (ML (mlCmMess168));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		/*
		 * Backorder Status.
		 */
		if (cmrh_rec.stat_flag [0] == 'B')
		{
			print_mess (ML (mlCmMess169));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		/*
		 * Completed Status.
		 */
		if (cmrh_rec.stat_flag [0] == 'C')
		{
			print_mess (ML (mlCmMess170));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		cc = LoadItems ();

		/*
		 * No lines for requisition.
		 */
		if (cc == 1)
		{
			scn_set (1);
			print_mess (ML (mlCmMess074));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		/*
		 * Error loading.
		 */
		if (cc == 2)
		{
			scn_set (1);
			print_mess (ML (mlCmMess171));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("cont_no"))
	{
		cmhr_rec.hhhr_hash = cmrh_rec.hhhr_hash;
		cc = find_rec (cmhr, &cmhr_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess075));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		/*
		 * Load contract decsription. 
		 */
		cmcd_rec.hhhr_hash = cmhr_rec.hhhr_hash;
		cmcd_rec.stat_flag [0] = 'D';
		cmcd_rec.line_no = 0;
		cc = find_rec (cmcd, &cmcd_rec, EQUAL, "r");

		while (!cc && 
			cmcd_rec.stat_flag [0] == 'D' &&
			cmcd_rec.line_no != 7 && 
			cmcd_rec.hhhr_hash == cmhr_rec.hhhr_hash)
		{
			strcpy (local_rec.contDesc [cmcd_rec.line_no], cmcd_rec.text);
			cmcd_rec.line_no++;
			cc = find_rec (cmcd, &cmcd_rec, EQUAL, "r");
		}

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Quantity Despatched. 
	 */
	if (LCHECK ("qtyIssued"))
	{
		if (ENTER_DATA)
			getval (line_cnt);

		if (SERIAL)
		{
			if (!F_HIDE (label ("serialNo")))
				FLD ("serialNo") = YES;
		}
		else
		{
			if (!F_HIDE (label ("serialNo")))
				FLD ("serialNo") = NA;
		}

		if (dflt_used)
		{
			local_rec.qtyIssued += SR.qtyOrder;
			local_rec.qtyOrder = 0.00;
			DSP_FLD ("qtyOrder");
			DSP_FLD ("qtyIssued");
		}
		else
			local_rec.qtyIssued = (float) atof (temp_str);

		if (local_rec.qtyIssued > SR.qtyOrder)
		{
			sprintf (err_str, ML (mlCmMess161),SR.qtyOrder,local_rec.qtyIssued);

			i = prmptmsg (err_str, "YyNn", 1, 2);
			if (i == 'n' || i == 'N')
				return (EXIT_FAILURE);

			move (1, 2);
			cl_line ();

			if (((local_rec.qtyIssued - SR.qtyOrder) - SR.avail) > 0.00)
			{
				cc = InputRes ();
				if (cc)
					return (cc);
			}
			else
			{
				local_rec.qtyBackorder += SR.qtyOrder - local_rec.qtyIssued;
				if (local_rec.qtyBackorder < 0.00)
					local_rec.qtyBackorder = 0.00;

				local_rec.qtyOrder = 0.00;
			}
		}
		else
		{
			if (ENTER_DATA)
			{
				local_rec.qtyBackorder += SR.qtyOrder - local_rec.qtyIssued;
				local_rec.qtyOrder = 0.00;
			}
		}

		if (SER_COSTING)
		{
			if (local_rec.qtyIssued != 0.00 &&
			     local_rec.qtyIssued != 1.00)
			{
				print_mess (ML (mlStdMess029));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}

		}

		SR.qtyIssued = local_rec.qtyIssued;

		if (NO_COST)
			SR.cost = 0.00;
		else
		{
			cc = FindCost ();
			if (cc)
			{
				SR.cost = 0.00;
				return (EXIT_FAILURE);
			}
		}

        skip_entry = (local_rec.qtyIssued != 0.00 && SERIAL && !strcmp (SR.serialNo, twentyFiveSpace)) ? 5 : 7;

		DSP_FLD ("qtyOrder");
		DSP_FLD ("qtyIssued");
		DSP_FLD ("qtyBackorder");
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate Quantity Backordered. 
	 */
	if (LCHECK ("qtyBackorder"))
	{
		if ((NON_STOCK || E_PHANTOM) && local_rec.qtyBackorder != 0.00)
		{
			local_rec.qtyBackorder = 0.00;

			if (NON_STOCK)
				print_mess (ML (mlCmMess181));
			else
				print_mess (ML (mlCmMess182));
			sleep (sleepTime);
			return (EXIT_SUCCESS);
		}

		if (local_rec.qtyBackorder < 0.00)
			local_rec.qtyBackorder = 0.00;

		/*
		 * Serial Items Can only have Qty of 0.00 or 1.00	
		 */
		if (SER_COSTING)
		{
			if (SR.qtyOrder == 1.00 && local_rec.qtyBackorder != 0.00)
			{
				print_mess (ML (mlStdMess190));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
			if (local_rec.qtyBackorder != 0.0 && local_rec.qtyBackorder != 1.00)
			{
				print_mess (ML (mlStdMess029));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
			if (SR.qtyOrder == 0.00 && local_rec.qtyBackorder == 0.00)
			{
				print_mess (ML (mlCmMess183));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
			if (local_rec.qtyBackorder == 1.00)
			{
				if (!F_HIDE (label ("serialNo")))
					FLD ("serialNo") = NI;
			}
			else
			{
				if (!F_HIDE (label ("serialNo")))
					FLD ("serialNo") = (BLANK_SER) ? YES : NI;
			}
		}
		return (EXIT_SUCCESS);
	}

	/*
	 * Check If Serial No already exists 
	 */
	if (LCHECK ("serialNo"))
	{
		if (F_HIDE (field) || FIELD.required == NA || !SERIAL)
			return (EXIT_SUCCESS);

		if (!SERIAL)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			SrchInsf (temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			strcpy (local_rec.serialNo, SR.serialNo);
			strcpy (insfRec.status, "C");
		}
		else
			strcpy (insfRec.status, "F");


		insfRec.hhwh_hash = SR.hhwhHash;
		sprintf (insfRec.serial_no, "%-25.25s", local_rec.serialNo);
		cc = find_rec (insf, &insfRec, COMPARISON, "r");
		if (cc)
		{
			if (SR.qtyOrder == 0.00)
				return (EXIT_SUCCESS);

			print_mess (ML (mlCmMess028));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (insfRec.receipted [0] != 'Y')
		{
			print_mess (ML (mlCmMess184));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		SR.cost = CENTS (SerialValue (insfRec.est_cost, insfRec.act_cost));

		SR.costed = TRUE;

		if (ChkDupInsf (local_rec.serialNo, SR.hhbrHash, line_cnt))
		{
			print_mess (ML (mlCmMess029));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		strcpy (SR.serialNo, local_rec.serialNo);

		strcpy (SR.errFound, "N");
		DSP_FLD ("serialNo");
		return (EXIT_SUCCESS);
	}

	/*
	 * Validate lots and locations. 
	 */
	if (LCHECK ("LL"))
	{
		if (FLD ("LL") == ND)
			return (EXIT_SUCCESS);

		TempLine	=	lcount [2];
		TempLineCnt	=	line_cnt;
		cc = DisplayLL
			(										/*----------------------*/
				line_cnt, 							/*	Line number.		*/
				tab_row + 3 + (line_cnt % TABLINES),/*  Row for window		*/
				tab_col + 22, 						/*  Col for window		*/
				4, 									/*  length for window	*/
				SR.hhwhHash, 						/*	Warehouse hash.		*/
				SR.hhumHash, 						/*	UOM hash			*/
				SR.hhccHash, 						/*	CC hash.			*/
				SR.UOM, 							/* UOM					*/
				SR.qtyIssued, 						/* Quantity.			*/
				SR.cnvFct, 							/* Conversion factor.	*/
				TodaysDate (), 						/* Expiry Date.			*/
				SR.lotSelectFlag, 					/* Silent mode			*/
				(local_rec.LL [0] == 'Y'), 			/* Input Mode.			*/
				SR.lotControl						/* Lot controled item. 	*/
													/*----------------------*/
			);
		/*
		 * Redraw screens. 
		 */
		strcpy (local_rec.LL, "Y");
		putval (line_cnt);

		lcount [2] = (line_cnt + 1 > lcount [2]) ? line_cnt + 1 : lcount [2];
		heading (2); 
		line_cnt = TempLineCnt;
		scn_write (2);
		scn_display (2);
		lcount [2] = TempLine;
		if (cc)
			return (EXIT_FAILURE);
		
		return (EXIT_SUCCESS);
	}

	return (EXIT_SUCCESS);
}

void
SrchCmrh (
	char	*keyValue)
{
	char	requisitionNo [7];
	char	desc [41];

	_work_open (6,0,40);
	save_rec ("#Req No", "#Contract | Requested By ");
	strcpy (cmrh_rec.co_no, comm_rec.co_no);
	strcpy (cmrh_rec.br_no, rq_branchNo);
	cmrh_rec.req_no = atol (keyValue);

	cc = find_rec (cmrh, &cmrh_rec, GTEQ, "r");
	while (!cc && !strcmp (cmrh_rec.co_no, comm_rec.co_no))
	{
		if (cmrh_rec.stat_flag [0] != 'R')
		{
			cc = find_rec (cmrh, &cmrh_rec, NEXT, "r");
			continue;
		}
		cmhr_rec.hhhr_hash = cmrh_rec.hhhr_hash;
		cc = find_rec (cmhr, &cmhr_rec, COMPARISON, "r");
		if (!cc)
		{
			sprintf (requisitionNo, "%06ld", cmrh_rec.req_no);
			sprintf (desc, "%-6.6s | %-20.20s", 
						cmhr_rec.cont_no, cmrh_rec.req_by);
			cc = save_rec (requisitionNo, cmrh_rec.req_by);
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
	strcpy (cmrh_rec.br_no, rq_branchNo);
	cmrh_rec.req_no = atol (temp_str);
	cc = find_rec (cmrh, &cmrh_rec, GTEQ, "r");
	if (cc)
		file_err (cc, cmrh, "DBFIND");
}

int
LoadItems (void)
{
	int		firstTime 	= TRUE;
	float	stdCnvFct	= 0.00;

	abc_selfield (inum, "inum_hhum_hash");

	/*
	 * Set screen 2 - for putval. 
	 */
	scn_set (2);
	lcount [2] = 0;

	vars [scn_start].row = 0;

	cmrd_rec.hhrq_hash 	= cmrh_rec.hhrq_hash;
	cmrd_rec.line_no 	= 0;
	cc = find_rec (cmrd, &cmrd_rec, GTEQ, "r");
	while (!cc && cmrd_rec.hhrq_hash == cmrh_rec.hhrq_hash)
	{
		/* 
		 * Not released.
		 */
		if (cmrd_rec.stat_flag [0] != 'R')
		{
			cc = find_rec (cmrd, &cmrd_rec, NEXT, "r");
			continue;
		}
		firstTime = FALSE;

		ccmr_rec.hhcc_hash	= cmrd_rec.hhcc_hash;
		cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, ccmr, "DBFIND");

		/*
		 * Read Lot / Location control file. 
		 */
		ReadLLCT (cmrd_rec.hhcc_hash);

		if (llctDesConf [0] == 'V')
			SR.lotSelectFlag	=	INP_VIEW;
		if (llctInput [0] == 'A')
			SR.lotSelectFlag	=	INP_AUTO;
		if (llctInput [0] == 'M')
		{
			strcpy (StockTake, "Y");
			SR.lotSelectFlag	=	INP_VIEW;
		}

		cmcm_rec.hhcm_hash = cmrd_rec.hhcm_hash;
		cc = find_rec (cmcm, &cmcm_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, cmcm, "DBFIND");

		strcpy (local_rec.costhd, cmcm_rec.ch_code);

		store [lcount [2]].hhcmHash = cmrd_rec.hhcm_hash;
		store [lcount [2]].hhccHash = cmrd_rec.hhcc_hash;
		store [lcount [2]].line_no 	= cmrd_rec.line_no;

		/*
		 * Get part number. 
		 */
		inmr_rec.hhbr_hash	= cmrd_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, inmr, "DBFIND");

		inum_rec.hhum_hash = inmr_rec.std_uom;
		cc = find_rec (inum, &inum_rec, EQUAL, "r");
		if (cc)
			file_err (cc, inum, "DBFIND");

		stdCnvFct = inum_rec.cnv_fct;

		inum_rec.hhum_hash = cmrd_rec.hhum_hash;
		cc = find_rec (inum, &inum_rec, EQUAL, "r");
		if (cc)
			strcpy (local_rec.UOM, "    ");

		strcpy (local_rec.UOM, inum_rec.uom);
		strcpy (store [lcount [2]].UOM, inum_rec.uom);
	    store [lcount [2]].hhumHash  = cmrd_rec.hhum_hash;
		line_cnt = lcount [2];

		if (stdCnvFct == 0.00)
			stdCnvFct = 1;

		store [lcount [2]].cnvFct = inum_rec.cnv_fct / stdCnvFct;
		cmrd_rec.sale_price  *= store [lcount [2]].cnvFct;
		cmrd_rec.gsale_price *= store [lcount [2]].cnvFct;

		CalcAvailable ();

		GL_GLI 
		(
			comm_rec.co_no, 
			comm_rec.est_no, 
			comm_rec.cc_no, 
			"COSTSALE C", 
			(envGlByClass) ? cumr_rec.class_type : cumr_rec.sman_code, 
			inmr_rec.category
		);
		strcpy (store [lcount [2]].creditAccount, glmrRec.acc_no);
		store [lcount [2]].creditAccountHash = glmrRec.hhmr_hash;

		store [lcount [2]].hhbrHash = inmr_rec.hhbr_hash;
		store [lcount [2]].dec_pt	 = inmr_rec.dec_pt;
		strcpy (store [lcount [2]].itemClass, inmr_rec.inmr_class);
		strcpy (local_rec.item_no, inmr_rec.item_no);
		strcpy (store [lcount [2]].serialFlag, inmr_rec.serial_item);
		strcpy (store [lcount [2]].costingFlag, inmr_rec.costing_flag);
		strcpy (store [lcount [2]].lotControl, inmr_rec.lot_ctrl);
		strcpy (local_rec.lotControl, inmr_rec.lot_ctrl);

		/*
		 * orignal quantity.
		 */
		local_rec.qtyOrder 				= ToLclUom (cmrd_rec.qty_order);
		store [lcount [2]].qtyOrder 	= ToLclUom (cmrd_rec.qty_order);

		/*
		 * backorder quantity.
		 */
		local_rec.qtyBackorder 			= ToLclUom (cmrd_rec.qty_border);
		store [lcount [2]].qtyBackorder = ToLclUom (cmrd_rec.qty_border);
		
		/*
		 * Issued quantity.
		 */
		local_rec.qtyIssued 			= ToLclUom (cmrd_rec.qty_iss);
		store [lcount [2]].qtyIssued  	= ToLclUom (cmrd_rec.qty_iss);

		local_rec.sell_pr  = DOLLARS (cmrd_rec.sale_price);
		local_rec.disc_pc  = (cmrd_rec.disc_pc);

		if (inmr_rec.serial_item [0] == 'Y')
		{
			strcpy (store [lcount [2]].serialNo, cmrd_rec.serial_no);
			strcpy (local_rec.serialNo, cmrd_rec.serial_no);
			strcpy (store [lcount [2]].orgSerialNo, cmrd_rec.serial_no);

		}
		else
		{
			sprintf (store [lcount [2]].serialNo , "%25.25s", " ");
			sprintf (local_rec.serialNo, "%25.25s", " ");
			sprintf (store [lcount [2]].orgSerialNo, "%25.25s", " ");
		}

		if (FLD ("LL") != ND)
		{
			cc = Load_LL_Lines
			(
				lcount [2], 
				LL_LOAD_CM, 
				cmrd_rec.cmrd_hash, 
				store [lcount [2]].hhccHash, 
				store [lcount [2]].UOM, 
				store [lcount [2]].cnvFct, 
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
						store [lcount [2]].hhwhHash, 						
						store [lcount [2]].hhumHash, 						
						store [lcount [2]].hhccHash, 						
						store [lcount [2]].UOM, 							
						store [lcount [2]].qtyIssued, 
						store [lcount [2]].cnvFct, 						
						TodaysDate (), 
						TRUE, 
						FALSE, 
						store [lcount [2]].lotControl						
					);
			}
			strcpy (local_rec.LL, "Y");
		}

		putval (lcount [2]++);
		if (lcount [2] > MAXLINES)
			break;

		cc = find_rec (cmrd, &cmrd_rec, NEXT, "r");
	}

	if (firstTime)
		return (EXIT_FAILURE);

	vars [scn_start].row = lcount [2];

	abc_selfield (inum, "inum_uom");

	scn_set (1);
	return (EXIT_SUCCESS);
}

int
heading (
	int		scn)
{
	if (restart)
		return (EXIT_SUCCESS);

	if (scn != cur_screen)
		scn_set (scn);

	swide ();
	clear ();

	line_at (1, 0, 130);

	rv_pr (ML (mlCmMess167), (130 - strlen (ML (mlCmMess167))) / 2, 0, 1);

	if (scn == 1)
	{
		box (0, 3, 130, 17);
		line_at (13, 1, 129);
		line_at (16, 1, 129);
	}

	if (scn == 2)
	{
		box (0, 3, 132, 2);
		print_at (4, 2, ML (mlCmMess090), local_rec.requisitionNo);
		print_at (4, 60, ML (mlCmMess091), cmhr_rec.cont_no);

		print_at (5, 2, ML (mlCmMess197));
		print_at (5, 20, "%-40.40s", cmcm_rec.desc);
		print_at (5, 60, ML (mlCmMess093));
		line_at (21, 0, 132);
	}
		

	print_at (22, 0, ML (mlStdMess038),  comm_rec.co_no,  comm_rec.co_short);
	print_at (22, 45, ML (mlStdMess039), comm_rec.est_no, comm_rec.est_short);
	print_at (22, 90, ML (mlStdMess099), comm_rec.cc_no,  comm_rec.cc_short);

	line_cnt = 0;
	scn_write (scn);
	return (EXIT_SUCCESS);
}

void
tab_other (
	int		line_no)
{
	if (cur_screen != 2 || ENTER_DATA)
		return;

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

	inmr_rec.hhbr_hash = store [line_no].hhbrHash;
	cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, inmr, "DBFIND");


	cmcm_rec.hhcm_hash = store [line_no].hhcmHash;
	cc = find_rec (cmcm, &cmcm_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, cmcm, "DBFIND");

	print_at (5, 2, ML (mlCmMess197));
	print_at (5, 60, ML (mlCmMess093));
	print_at (5, 20, "%-40.40s", cmcm_rec.desc);
	print_at (5, 80, "%-40.40s", inmr_rec.description);

	return;
}

/*
 * Confirm Complete order and allow user to Change. 
 */
void
Confirm (void)
{
	scn_set (2);

	for (line_cnt = 0;line_cnt < lcount [2];line_cnt++)
	{
		getval (line_cnt);

		local_rec.qtyIssued += local_rec.qtyOrder;
		SR.qtyIssued = local_rec.qtyIssued;

		if (SR.serialFlag [0] == 'Y')
		{
			if (!strcmp (local_rec.serialNo, twentyFiveSpace))
				strcpy (SR.errFound, "Y");
			else
				strcpy (SR.errFound, "N");
		}
		else
			strcpy (SR.errFound, "N");

		putval (line_cnt);
	}
}

int
CheckOK (
	int		serial)
{
	int	i;

	for (i = 0; i < MAXLINES; i++)
	{
		getval (i);

		if (store [i].serialFlag [0] == 'Y' && serial &&
		     store [i].errFound [0] == 'Y' &&
		     local_rec.qtyIssued > 0.00)
			return (i + 1);

		if (store [i].serialFlag [0] == 'N' && !serial &&
		     store [i].errFound [0] == 'Y' &&
		     local_rec.qtyIssued > 0.00)
			return (i + 1);
	}
	return (EXIT_SUCCESS);
}

/*
 * Open Data Dase Files.
 */
void
OpenDB (void)
{
	if ((pp = popen ("pformat", "w")) == 0) 
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);

	MoveOpen	=	FALSE;
	pipeOpen = TRUE;

	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");
	strcpy (comr_rec.co_no, comm_rec.co_no);
	cc = find_rec (comr, &comr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, comr, "DBFIND");

	if (!strcmp (comr_rec.base_curr, "   "))
		sprintf (envCurrCode, "%-3.3s", get_env ("CURR_CODE"));
	else
		sprintf (envCurrCode, "%-3.3s", comr_rec.base_curr);
	abc_fclose (comr);

	abc_alias (inum2, inum);

	open_rec (cmcb, cmcb_list, CMCB_NO_FIELDS, "cmcb_id_no");
	open_rec (cmcd, cmcd_list, CMCD_NO_FIELDS, "cmcd_id_no");
	open_rec (cmcm, cmcm_list, CMCM_NO_FIELDS, "cmcm_hhcm_hash");
	open_rec (cmhr, cmhr_list, CMHR_NO_FIELDS, "cmhr_hhhr_hash");
	open_rec (cmrh, cmrh_list, CMRH_NO_FIELDS, "cmrh_id_no");
	open_rec (cmrd, cmrd_list, CMRD_NO_FIELDS, "cmrd_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (inwu, inwu_list, INWU_NO_FIELDS, "inwu_id_no");
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_hhcc_hash");
    open_rec (inum, inum_list, INUM_NO_FIELDS, "inum_uom");
    open_rec (inum2,inum_list, INUM_NO_FIELDS, "inum_id_no2");
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (cmtr, cmtr_list, CMTR_NO_FIELDS, "cmtr_hhhr_hash");
	open_rec ("move", move_list, MOVE_NO_FIELDS, "move_move_hash");

	OpenGlmr ();
	OpenGlwk ();
	GL_OpenBatch (comm_rec.co_no, comm_rec.est_no);

	/*
	 * Read ccmr for current warehouse. 
	 */
	abc_selfield (ccmr, "ccmr_id_no");
	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, comm_rec.est_no);
	strcpy (ccmr_rec.cc_no, comm_rec.cc_no);
	cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");
	abc_selfield (ccmr, "ccmr_hhcc_hash");

	OpenLocation (ccmr_rec.hhcc_hash);
}

/*
 * Close data base files. 
 */
void
CloseDB (void)
{
	abc_fclose (cmcb);
	abc_fclose (cmcd);
	abc_fclose (cmcm);
	abc_fclose (cmhr);
	abc_fclose (cmrh);
	abc_fclose (cmrd);
	abc_fclose (inmr);
	abc_fclose (incc);
	abc_fclose (inwu);
	abc_fclose (ccmr);
	abc_fclose (inum);
	abc_fclose (inum2);
	abc_fclose (cumr);
	abc_fclose (cmtr);
	abc_fclose ("move");

	CloseCosting ();
	GL_CloseBatch (printerNo);
	GL_Close ();

	CloseLocation ();

	abc_dbclose (data);
}

/*
 * Check Whether A Serial Number For This Item Number	
 * Has Already Been Used.			
 * Return 1 if duplicate		
 */
int
ChkDupInsf (
	char 	*serialNo, 
	long	hhbrHash, 
	int		line_no)
{
	int	i;
	int	no_lines = 0;

	no_lines = (ENTER_DATA) ? line_cnt : lcount [2];

	for (i = 0;i < no_lines; i++)
	{
		/*
		 * Ignore Current Line	
		 */
		if (i == line_no)
			continue;

		/*
		 * Only compare serial numbers for the same item number
		 */
		if (store [i].hhbrHash == hhbrHash)
			if (!strcmp (store [i].serialNo, serialNo))
				return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}

/*
 * Update incc & add transaction. 
 */
int
Update (void)
{
	int		allBackordered = FALSE;
	int		someBackordered = FALSE;
	int 	cc1;
	double	value;
	float	workQty = 0.00;
	double	workValue = 0.00;

	clear ();
	print_at (0, 0, ML (mlStdMess035));
	fflush (stdout);

	scn_set (2);

	allBackordered = TRUE;
	/*
	 * Check if all/any lines are backorded. 
	 */
	for (line_cnt = 0;line_cnt < lcount [2];line_cnt++)
	{
		getval (line_cnt);
		if (local_rec.qtyBackorder != 0.00 && cmrh_rec.full_supply [0] == 'Y')
		{
			local_rec.qtyBackorder += local_rec.qtyIssued;
			local_rec.qtyIssued = 0.00;
			putval (line_cnt);
			someBackordered = TRUE;
		}
		if (local_rec.qtyIssued != 0.00)
			allBackordered = FALSE;
	}

	/*
	 * If ANY were B/Ordered AND FULL-SUPPLY then make ALL B/Ordered.	
	 */
	if ((allBackordered || someBackordered) && cmrh_rec.full_supply [0] == 'Y')
	{
	    allBackordered = TRUE;
	    for (line_cnt = 0;line_cnt < lcount [2];line_cnt++)
	    {
			getval (line_cnt);
			local_rec.qtyBackorder += local_rec.qtyIssued;
			local_rec.qtyOrder 		= 0.00;
			local_rec.qtyIssued 	= store [line_cnt].qtyIssued;
			putval (line_cnt);
	    }
	}

	/*
	 * Update inventory cost centre stock record (file incc).
	 */
	for (line_cnt = 0; line_cnt < lcount [2]; line_cnt++)
	{
		/*
		 * Get Tabular line. 
		 */
		getval (line_cnt);

		cc = FindCost ();

		result = strchr (envSkIvalClass, SR.itemClass [0]);
		UPDATE_OK = (result == (char *) NULL) ? TRUE : FALSE;

		/*
		 * Find inmr record from item number in structure. 
		 */
		strcpy (inmr_rec.co_no, comm_rec.co_no);
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "u");
		if (cc)
			file_err (cc, inmr, "DBFIND");

		workQty = ToStdUom (local_rec.qtyIssued);
		value = DOLLARS (SR.cost);

		workValue = (float) workQty;
		workValue *= value;
		workValue = twodec (out_cost (workValue, inmr_rec.outer_size));

	 	UpdateFiles (workQty, value);

		/*
		 * Add General Ledger inventory transactions. 
		 */
		AddGlwk ();

		if (SK_BATCH_CONT || MULT_LOC)
			UpdateLotLocation (line_cnt, TRUE);

		add_hash 
		(
			comm_rec.co_no, 
			comm_rec.est_no, 
			"RC", 
			0, 
			SR.hhbrHash, 
			SR.hhccHash, 
			0L, 
			(double) 0.00
		);
		recalc_sobg ();
	}

	cc = find_rec (cmrh, &cmrh_rec, EQUAL, "u");
	if (cc)
		file_err (cc, cmrh, "DBFIND");

	/*
	 * if all lines are 'C' then header may be set to 'C' 
	 */
	abc_selfield (cmrd, "cmrd_id_no2");

	cmrd_rec.hhrq_hash = cmrh_rec.hhrq_hash;
	strcpy (cmrd_rec.stat_flag, "B");
	cc1 = find_rec (cmrd, &cmrd_rec, EQUAL, "r");

	cmrd_rec.hhrq_hash = cmrh_rec.hhrq_hash;
	strcpy (cmrd_rec.stat_flag, "R");
	cc = find_rec (cmrd, &cmrd_rec, EQUAL, "r");

	if (cc && cc1)
		strcpy (cmrh_rec.stat_flag, "C");

	if (allBackordered)
	{
		strcpy (cmrh_rec.stat_flag, "B");
		sprintf (err_str, "%s %s", ML (mlCmMess081), ML (mlStdMess042));
		PauseForKey (0, 0, err_str, 0);

	}

	cmrh_rec.iss_date 	= TodaysDate ();
	cmrh_rec.time_create = atot (TimeHHMM ());

	cc = abc_update (cmrh, &cmrh_rec);
	if (cc)
		file_err (cc, cmrh, "DBUPDATE");

	return (EXIT_SUCCESS);
}

/*
 * Process Stock Issues. 
 */
void
UpdateFiles (
	float	processQty, 
	double	value)
{
	char	expandContNo [11];
	int		NoLots	=	TRUE;
	int		i;
	/*
	 * Update issuing Cost Centre 
	 */
	incc_rec.hhcc_hash = SR.hhccHash;
	incc_rec.hhbr_hash = SR.hhbrHash;
	cc = find_rec (incc, &incc_rec, COMPARISON, "u");
	if (cc)
		file_err (cc, incc, "DBFIND");

	if (UPDATE_OK)
	{
		/*
		 * Find Warehouse unit of measure file. 
		 */
		inwu_rec.hhwh_hash	=	incc_rec.hhwh_hash;
		inwu_rec.hhum_hash	=	inmr_rec.std_uom;
		cc = find_rec (inwu, &inwu_rec, COMPARISON, "u");
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
			cc = find_rec (inwu, &inwu_rec, COMPARISON, "u");
			if (cc)
				file_err (cc, inwu, "DBFIND");
		}
		incc_rec.issues 		+= processQty;
		incc_rec.ytd_issues 	+= processQty;
		incc_rec.closing_stock 	= 	incc_rec.opening_stock +
				    	    		incc_rec.pur +
				    	    		incc_rec.receipts +
				    	    		incc_rec.adj -
				    	    		incc_rec.issues -
				    	    		incc_rec.sales;

		cc = abc_update (incc, &incc_rec);
		if (cc)
			file_err (cc, incc, "DBUPDATE");

		inwu_rec.issues	+= processQty;
		inwu_rec.closing_stock = inwu_rec.opening_stock +
								 inwu_rec.pur +
								 inwu_rec.receipts +
								 inwu_rec.adj -
								 inwu_rec.issues -
								 inwu_rec.sales;

		cc = abc_update (inwu, &inwu_rec);
		if (cc)
			file_err (cc, inwu, "DBUPDATE");
	}
	else
		abc_unlock (incc);

	if (SERIAL)
	{
		cc	=	UpdateInsf
				(
					SR.hhwhHash, 
					0L, 
					local_rec.serialNo, 
					"C", 
					"S"
				);
	}

	if (UPDATE_OK)
	{
		inmr_rec.on_hand -= processQty;
		cc = abc_update (inmr, &inmr_rec);
		if (cc)
			file_err (cc, inmr, "DBUPDATE");
	}
	else
		abc_unlock (inmr);

	sprintf (err_str, "REQ%06ld", cmrh_rec.req_no);
	sprintf (expandContNo, "CN:%-6.6s", cmhr_rec.cont_no);

	NoLots	=	TRUE;
	if (SK_BATCH_CONT && MULT_LOC)
	{
		for (i = 0; i < MAX_LOTS; i++)
		{
			if (!LL_Valid (line_cnt, i))
				break;

			NoLots	=	FALSE;
			/*
			 * Log inventory movements. 
			 */
			MoveAdd 
			(
				comm_rec.co_no, 
				comm_rec.est_no, 
				ccmr_rec.cc_no, 
				SR.hhbrHash, 
				SR.hhccHash, 
				SR.hhumHash, 
				TodaysDate (), 
				3, 
				GetLotNo (line_cnt, i), 
				inmr_rec.inmr_class, 
				inmr_rec.category, 
				expandContNo, 
				err_str, 
				GetBaseQty (line_cnt, i), 
				0.00, 
				CENTS (value)
			); 

		}
	}
	if (NoLots)
	{
		/*
		 * Log inventory movements. 
		 */
		MoveAdd 
		(
			comm_rec.co_no, 
			comm_rec.est_no, 
			ccmr_rec.cc_no, 
			SR.hhbrHash, 
			SR.hhccHash, 
			SR.hhumHash, 
			TodaysDate (), 
			3, 
			" ",
			inmr_rec.inmr_class, 
			inmr_rec.category, 
			expandContNo, 
			err_str, 
			processQty, 
			0.00, 
			CENTS (value)
		); 
	}
}

/*
 * Search for serial number. 
 */
void
SrchInsf (
	char	*keyValue)
{
	int	prev_done = FALSE;
	char	*ser_no = clip (keyValue);

	_work_open (25,0,20);
	cc = FindInsf (SR.hhwhHash, 0L, "", "F", "r");
	while (!cc && SR.hhwhHash == insfRec.hhwh_hash)
	{
		if (strncmp (ser_no, insfRec.serial_no, strlen (ser_no)) < 0)
			break;

		if (!strncmp (ser_no, insfRec.serial_no, strlen (ser_no)))
		{
			if (!strcmp (insfRec.serial_no, prv_ntry))
				prev_done = TRUE;
			cc = save_rec (insfRec.serial_no, local_rec.item_no);
			if (cc)
				break;
		}
		cc = FindInsf (0L, 0L, "", "F", "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	ser_no = clip (keyValue);
	cc = FindInsf (SR.hhwhHash, 0L, ser_no, "F", "r");
}

/*
 * Find inventory Master file record.       
 * Check for Supercessions and Short Codes.
 */
int
IntFindInmr (
	char	*co_no, 
	char	*item)
{
	sup_counter = 0;
	sup_flag = FALSE;
	alt_flag = FALSE;

	sprintf (inmr_rec.co_no, "%-2.2s", co_no);
	sprintf (inmr_rec.item_no, "%-16.16s", item);
	if (find_rec (inmr, &inmr_rec, COMPARISON, "r"))
	{
		if (check_short (inmr_rec.item_no))
			return (EXIT_FAILURE);

		abc_selfield (inmr, "inmr_quick_id");
		sprintf (inmr_rec.co_no, "%-2.2s", co_no);
		sprintf (inmr_rec.quick_code, "%-8.8s", inmr_rec.item_no);
		if (find_rec (inmr, &inmr_rec, COMPARISON, "r"))
		{
			abc_selfield (inmr, "inmr_id_no");
			return (EXIT_FAILURE);
		}

		abc_selfield (inmr, "inmr_id_no");
	}
	if (inmr_rec.hhsi_hash != 0L && alternate_ok)
	{
		sprintf (alt_part, "%-16.16s", inmr_rec.item_no);
		if (IntFindSuper (co_no, inmr_rec.alternate, FALSE))
		{
			alt_flag = FALSE;
			return (EXIT_FAILURE);
		}
	}
	sprintf (sup_part, "%-16.16s", inmr_rec.item_no);
	if (IntFindSuper (co_no, inmr_rec.supercession, TRUE))
	{
		sup_flag = FALSE;
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}

/*
 * Validate Supercession Number. 
 */
int
IntFindSuper (
 char *	co_no, 
 char *	sitem_no, 
 int	super)
{
	/*
	 * At the end of the supercession chain	
	 */
	if (!strcmp (sitem_no, sixteen_space) ||
	  (!super && inmr_rec.hhsi_hash == 0L))
		return (EXIT_SUCCESS);

	sup_flag = (super) ? TRUE : FALSE;
	alt_flag = (super) ? FALSE: TRUE;

	if (sup_counter++ > MAX_SUPER)
	{
		/*sprintf (err_str, "Looping %s, Check Item %s", (super) ?
					"Supercession" : "Alternate", 
				         inmr_rec.item_no);*/
		sprintf (err_str, (super) ? ML (mlCmMess162) : ML (mlCmMess163), inmr_rec.item_no);

		return (ItemError (err_str));
	}
	/*
	 * Find the superceeding item
	 */
	sprintf (inmr_rec.co_no, "%-2.2s", co_no);
	strcpy (inmr_rec.item_no, sitem_no);
	if (!find_rec (inmr, &inmr_rec, COMPARISON, "r"))
	{
		return (IntFindSuper (co_no, (super) ? inmr_rec.supercession
					          : inmr_rec.alternate, 
					          super));
	}


	/*
	 * Couldn't find the superceeding item	
	 */
	sprintf (err_str, (super) ? ML (mlCmMess164) : ML (mlCmMess165), 
						  (super) ? sup_part : alt_part);
	return (ItemError (err_str));
}

void
IntSupAltErr (void)
{
	if (!alt_flag && !sup_flag)
		return;

	clear_mess ();
	if (alt_flag)
	{
		/*sprintf (err_str, "Item # %s has Master alternate Item # %s %c", 
				clip (alt_part), inmr_rec.item_no, BELL);*/
		sprintf (err_str, ML (mlCmMess177), clip (alt_part), inmr_rec.item_no, BELL);
		alt_flag = FALSE;
	}
	else
	{
		/*sprintf (err_str, "Item # %s has been Superceded by Item # %s %c", 
				clip (sup_part), inmr_rec.item_no, BELL);*/
		sprintf (err_str, ML (mlCmMess178), clip (sup_part), inmr_rec.item_no, BELL);
		sup_flag = FALSE;
	}
	print_mess (err_str);
	sleep (sleepTime);
}

/*
 * Clear popup window ready for new item. 
 */
void
ClearWin (void)
{
	int	i;
	for (i = 18;i < 24;i++)
	{
		move (0, i);
		cl_line ();
	}
	heading (2);
	scn_display (2);
}

/*
 * Input responses to stock quantity on hand less-than input quantity.
 */
int
InputRes (void)
{
	int	i;
	int	fs_flag = FALSE;
	int	displayed = FALSE;
	char	val_keys [21];
	char	disp_str [150];
	float	value;

	cc = 0;

	if (SR.closing <= 0.00 && (SERIAL || envSoOverride [0] == 'N'))
	{
		strcpy (val_keys, "CcRrDd");
		sprintf (disp_str, 
			"%s (C)ancel%s  %s (R)educe%s  %s (D)isplay%s  ", 
			ta [8], ta [9], ta [8], 
			ta [9], ta [8], ta [9]);
	}
	else
	{
		strcpy (val_keys, "OoCcRrDd");
		sprintf (disp_str, 
			"%s (O)verride%s  %s (C)ancel%s  %s (R)educe%s  %s (D)isplay%s  ", 
			ta [8], ta [9], ta [8], ta [9], 
			ta [8], ta [9], ta [8], ta [9]);
	}


	if (strcmp (inmr_rec.alternate, sixteen_space))
	{
		sprintf (err_str, "%s (S)ubstitute%s  ", ta [8], ta [9]);

		strcat (val_keys, "Ss");
		strcat (disp_str, err_str);
	}

	sprintf (err_str, "%s (B)ackorder bal%s  %s (F)orce b/o%s ", 
			ta [8], ta [9], ta [8], ta [9]);
	strcat (disp_str, err_str);
	strcat (val_keys, "BbFf");

	while (1)
	{
		i = prmptmsg (disp_str, val_keys, 1, 2);

		BusyFunc (0);
		switch (i)
		{
		/*
		 * Accept Quantity input. 
		 */
		case	'O':
		case	'o':
			local_rec.qtyOrder = 0.00;
			break;

		/*
		 * Backorder Balance. 
		 */
		case	'B':
		case	'b':
			value = local_rec.qtyIssued;
			if (SR.avail > 0.00)
			{
				local_rec.qtyIssued = SR.qtyOrder + ToLclUom (SR.avail);
				SR.avail = 0.00;
			}
			else
			{
				local_rec.qtyIssued = local_rec.qtyOrder;
			}

			local_rec.qtyOrder = 0.00;
			local_rec.qtyBackorder += value - local_rec.qtyIssued;

			break;

		/*
		 * Cancel Quantity input and check if log to lost sale. 
		 */
		case	'C':
		case	'c':
			fs_flag = TRUE;
			break;

		/*
		 * Display other details by running so_win_dsp. 
		 */
		case	'D':
		case	'd':
#ifdef GVISION
			DisplayStockWindow (SR.hhbrHash, SR.hhccHash);
#else
			BusyFunc (1);
			if (!wpipeOpen)
			{
				if (OpenSkWin ())
					break;
			}
			fprintf (pout, "%10ld%10ld\n", SR.hhbrHash, 
					      	SR.hhccHash);

			fflush (pout);
			IP_READ (np_fn);
			BusyFunc (0);
			displayed = TRUE;
#endif	/* GVISION */
			continue;

		/*
		 * Quantity has been reduced to equal quantity on hand. 
		 */
		case	'R':
		case	'r':
			if (SR.avail > 0.00)
			{
				local_rec.qtyIssued = SR.qtyOrder + ToLclUom (SR.avail);
				SR.avail = 0.00;
			}
			else
			{
				local_rec.qtyIssued = local_rec.qtyOrder;
			}

			local_rec.qtyOrder = 0.00;

			break;

		/*
		 * Substitute Alternate number. 
		 */
		case	'S':
		case	's':
			sprintf (err_str, "%s", clip (inmr_rec.alternate));
			sprintf (inmr_rec.item_no, "%-16.16s", err_str);
			cc = IntFindInmr (comm_rec.co_no, inmr_rec.item_no);
			skip_entry = (cc) ? -4 : -1;
			break;

		/*
		 * Force a complete backorder. 
		 */
		case	'F':
		case	'f':
			local_rec.qtyBackorder += local_rec.qtyIssued;
			local_rec.qtyOrder = 0.00;
			local_rec.qtyIssued = 0.00;
			break;
		}
		print_at (2, 1, "%90.90s", " ");

		break;
	}

	if (displayed)
		ClearWin ();

	return (fs_flag);
}

void
BusyFunc (
 int	flip)
{
	print_at (2, 1, "%-90.90s", " ");
	if (flip)
		print_at (2, 1, ML (mlStdMess035));
		/*print_at (2, 1, "Busy...\007\007");*/
	fflush (stdout);
}

#ifndef GVISION
int
OpenSkWin (void)
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
	wpipeOpen = TRUE;
	fprintf (pout, "%06d\n", getpid ());
	return (EXIT_SUCCESS);
}
#endif	/* GVISION */

int
ItemError (
 char *	message)
{
	int	i;

	for (i = 0; i < 5; i++)
	{
		errmess (message);
		sleep (sleepTime);
	}

	return (EXIT_FAILURE);
}

/*
 * Add transactions to glwk file. 
 */
void
AddGlwk (void)
{
	int		monthPeriod;
	double	value;

	/*
	 * get customer info 
	 */

	cumr_rec.hhcu_hash = cmhr_rec.hhcu_hash;
	cc = find_rec (cumr, &cumr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, cumr, "DBFIND");

	/*
	 * Somes details have been printed 
	 */
	value = local_rec.sell_pr * ((100 - local_rec.disc_pc) / 100);
	value = CENTS (value);

	/*
	 * post WIP 
	 * lookup hhgl_hash 
	 */
	strcpy (glmrRec.co_no, comm_rec.co_no);
	strcpy (glmrRec.acc_no, cmhr_rec.wip_glacc);
	cc = find_rec (glmr, &glmrRec, EQUAL, "r");
	if (cc)
		file_err (cc, glmr, "DBFIND");

	strcpy (glwkRec.co_no, comm_rec.co_no);
	strcpy (glwkRec.est_no, comm_rec.est_no);
	strcpy (glwkRec.acc_no, cmhr_rec.wip_glacc);
	glwkRec.hhgl_hash = glmrRec.hhmr_hash;

	sprintf (glwkRec.acronym, "%-10.10s", cumr_rec.dbt_no);
	sprintf (glwkRec.name, "%-30.30s", cumr_rec.dbt_name);
	sprintf (glwkRec.chq_inv_no, "%-15.15s", cmhr_rec.cont_no);
	sprintf (glwkRec.jnl_type, "%-1.1s", "1");  
	sprintf (glwkRec.tran_type, "%-2.2s", "22"); 
	glwkRec.amount = local_rec.qtyIssued * SR.cost;
	glwkRec.ci_amt = glwkRec.amount;

	sprintf (glwkRec.narrative, "Stock Issue %s", cmhr_rec.cont_no);

	sprintf (glwkRec.sys_ref, "%010ld", (long) comm_rec.term);
	glwkRec.post_date 	= TodaysDate ();
	glwkRec.tran_date   = TodaysDate ();
	sprintf (glwkRec.user_ref, "%6.6s", cmhr_rec.cont_no);
	strcpy (glwkRec.alt_desc1, " ");
	strcpy (glwkRec.alt_desc2, " ");
	strcpy (glwkRec.alt_desc3, " ");
	strcpy (glwkRec.batch_no, " ");
	sprintf (glwkRec.stat_flag, "%-1.1s", "2");  

	DateToDMY (local_rec.lsystemDate, NULL, &monthPeriod, NULL);
	sprintf (glwkRec.period_no, "%02d", monthPeriod);

	if (local_rec.qtyIssued != 0.00)
	{
		glwkRec.exch_rate 	= 1.00;
		glwkRec.loc_amount 	= glwkRec.amount;
		strcpy (glwkRec.currency, envCurrCode);
		GL_AddBatch ();
		glwkUpdate = TRUE;
	}

	sprintf (glwkRec.narrative, "Issue From WH %s", ccmr_rec.cc_no);

	sprintf (glwkRec.acc_no, "%-*.*s", MAXLEVEL, MAXLEVEL, SR.creditAccount);
	glwkRec.hhgl_hash = SR.creditAccountHash;
	strcpy (glwkRec.jnl_type, "2");

	if (local_rec.qtyIssued != 0.00)
	{
		glwkRec.exch_rate 	= 1.00;
		glwkRec.loc_amount 	= glwkRec.amount;
		strcpy (glwkRec.currency, envCurrCode);
		GL_AddBatch ();
		glwkUpdate = TRUE;
	}

	if (local_rec.qtyIssued != 0.00)
		AddCmtr ();

	cmrd_rec.hhrq_hash = cmrh_rec.hhrq_hash;
	cmrd_rec.line_no = SR.line_no;
	cc = find_rec (cmrd, &cmrd_rec, EQUAL, "u");
	if (cc)
		file_err (cc, cmrd, "DBFIND");

	cmrd_rec.qty_order = ToStdUom (local_rec.qtyOrder);
	cmrd_rec.qty_border = ToStdUom (local_rec.qtyBackorder);
	cmrd_rec.qty_iss = ToStdUom (local_rec.qtyIssued);
	cmrd_rec.cost = SR.cost;
	cmrd_rec.sale_price  /= SR.cnvFct;
	cmrd_rec.gsale_price /= SR.cnvFct;

	strcpy (cmrd_rec.serial_no, local_rec.serialNo);

	if (cmrd_rec.qty_border != 0.00 && cmrd_rec.qty_order == 0.00)
	{
		strcpy (cmrd_rec.stat_flag, "B");
	}
	else
	{
		if ((cmrd_rec.qty_order) <= 0.00)
			strcpy (cmrd_rec.stat_flag, "C");
		else
			strcpy (cmrd_rec.stat_flag, "R");
	}

	cc = abc_update (cmrd, &cmrd_rec);
	if (cc)
		file_err (cc, cmrd, "DBUPDATE");

	PrintDetails (value);
}

void
AddCmtr (void)
{
	cmtr_rec.hhhr_hash = cmrh_rec.hhhr_hash;
	cmtr_rec.hhcm_hash = SR.hhcmHash;
	cmtr_rec.hhbr_hash = SR.hhbrHash;
	cmtr_rec.sale_price = CENTS (local_rec.sell_pr);
	cmtr_rec.disc_pc = local_rec.disc_pc;
	cmtr_rec.cost_price =  SR.cost;
	cmtr_rec.qty = local_rec.qtyIssued;
	cmtr_rec.date = local_rec.lsystemDate;
	strcpy (cmtr_rec.ser_no, local_rec.serialNo);

	strcpy (cmtr_rec.time, TimeHHMMSS ());

	cc = abc_add (cmtr, &cmtr_rec);
	if (cc)
		file_err (cc, cmtr, "DBADD");
}

void
CalcAvailable (void)
{
	incc_rec.hhcc_hash = cmrd_rec.hhcc_hash;
	incc_rec.hhbr_hash = cmrd_rec.hhbr_hash;
	cc = find_rec (incc, &incc_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, incc, "DBFIND");

	store [lcount [2]].hhwhHash = incc_rec.hhwh_hash;
	store [lcount [2]].closing   = incc_rec.closing_stock;
	store [lcount [2]].avail     = 	incc_rec.closing_stock -
								   	incc_rec.backorder -
								   	incc_rec.committed -
								   	incc_rec.forward;
}

/*
 * Find item cost. 
 */
int
FindCost (void)
{
	double	wk_cost	=	0.00;

	if (SR.costed)
		return (EXIT_SUCCESS);

	if (SERIAL)
	{
		SR.cost	=	FindInsfCost
					(
						SR.hhwhHash, 
						0L, 
						SR.serialNo, 
						"C"
					);
		if (SR.cost == -1.00)
		{
			SR.costed = FALSE;
			return (EXIT_FAILURE);
		}
		SR.cost	=	CENTS (SR.cost);
		SR.costed = TRUE;
		return (EXIT_SUCCESS);
	}
	switch (SR.costingFlag [0])
	{
	case 'A':
	case 'L':
	case 'P':
	case 'T':
		wk_cost = 	FindIneiCosts 
					(
						inmr_rec.costing_flag, 
						comm_rec.est_no, 
						SR.hhbrHash
					);
		break;

	case 'F':
	case 'I':
		wk_cost = 	FindIncfCost 
					(
						SR.hhwhHash, 
				     	SR.closing, 
				     	local_rec.qtyIssued, 
						(inmr_rec.costing_flag [0] == 'F') ? TRUE : FALSE, 
						SR.dec_pt
					);
		break;

	case 'S':
		wk_cost = FindInsfValue (SR.hhwhHash, TRUE);
		break;
	}

	if (wk_cost < 0.00)
	{
		wk_cost = 	FindIneiCosts 
					(
						"L", 
						comm_rec.est_no, 
						SR.hhbrHash
					);
	}
	SR.cost = no_dec (CENTS (wk_cost));

	return (EXIT_SUCCESS);
}

/*
 * Routine to open output pipe to standard print to provide an audit trail 
 * of events. This also sends the output straight to the spooler.         
 */
void
OpenAudit (void)
{
	sprintf (err_str, "%s <%s>", DateToString (comm_rec.inv_date), PNAME);
	fprintf (pp, ".START%s\n", clip (err_str));
	fprintf (pp, ".SO\n");
	fprintf (pp, ".LP%d\n", printerNo);
	fprintf (pp, ".12\n");
	fprintf (pp, ".L158\n");
	fprintf (pp, ".EDESPATCH ISSUES TO CONTRACTS\n");

	fprintf (pp, ".B1\n");
	fprintf (pp, ".E%s as at %24.24s\n", clip (comm_rec.co_short), SystemTime ());
	fprintf (pp, ".B2\n");

	fprintf (pp, ".EBRANCH %s  \n", clip (comm_rec.est_name));

	fprintf (pp, ".R===");
	fprintf (pp, "=======");
	fprintf (pp, "=======");
	fprintf (pp, "==========");
	fprintf (pp, "=================");
	fprintf (pp, "===================");
	fprintf (pp, "===========");
	fprintf (pp, "=====");
	fprintf (pp, "=========");
	fprintf (pp, "=========");
	fprintf (pp, "===========");
	fprintf (pp, "=================");
	fprintf (pp, "=================");
	fprintf (pp, "============\n");

	fprintf (pp, "===");
	fprintf (pp, "=======");
	fprintf (pp, "=======");
	fprintf (pp, "==========");
	fprintf (pp, "=================");
	fprintf (pp, "===================");
	fprintf (pp, "===========");
	fprintf (pp, "=====");
	fprintf (pp, "=========");
	fprintf (pp, "=========");
	fprintf (pp, "===========");
	fprintf (pp, "=================");
	fprintf (pp, "=================");
	fprintf (pp, "============\n");

	fprintf (pp, "|WH");
	fprintf (pp, "| REQ  ");
	fprintf (pp, "| CONT ");
	fprintf (pp, "|CUSTOMER ");
	fprintf (pp, "| ITEM NUMBER    ");
	fprintf (pp, "| ITEM  DESCRIPTION ");
	fprintf (pp, "|  @COST   ");
	fprintf (pp, "|UOM ");
	fprintf (pp, "|QUANTITY");
	fprintf (pp, "|   DATE   ");
	fprintf (pp, "| LOCATION ");
	fprintf (pp, "|      DEBIT     ");
	fprintf (pp, "|     CREDIT     ");
	fprintf (pp, "| EXTENDED |\n");

	fprintf (pp, "|  ");
	fprintf (pp, "|  NO. ");
	fprintf (pp, "|  NO. ");
	fprintf (pp, "| ACRONYM ");
	fprintf (pp, "|                ");
	fprintf (pp, "|                   ");
	fprintf (pp, "|          ");
	fprintf (pp, "|    ");
	fprintf (pp, "|        ");
	fprintf (pp, "| ISSUED ");
	fprintf (pp, "|          ");
	fprintf (pp, "|     ACCOUNT    ");
	fprintf (pp, "|     ACCOUNT    ");
	fprintf (pp, "|          |\n");

	fprintf (pp, "|--");
	fprintf (pp, "|------");
	fprintf (pp, "|------");
	fprintf (pp, "|---------");
	fprintf (pp, "|----------------");
	fprintf (pp, "|-------------------");
	fprintf (pp, "|----------");
	fprintf (pp, "|----");
	fprintf (pp, "|--------");
	fprintf (pp, "|--------");
	fprintf (pp, "|----------");
	fprintf (pp, "|----------------");
	fprintf (pp, "|----------------");
	fprintf (pp, "|----------|\n");

	fprintf (pp, ".PI12\n");
}

/*
 * Print details of data input. 
 */
void
PrintDetails (
	double	value)
{
	int		i;

	inmr_rec.hhbr_hash = store [line_cnt].hhbrHash;
	cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, inmr, "DBFIND");

	for (i = 0; i < MAX_LOTS; i++)
	{
		if (!LL_Valid (line_cnt, i))
			break;

		/*
		 * Somes details have been printed 
		 */
		if (i == 0)
		{
			fprintf (pp, "!%2.2s", 		ccmr_rec.cc_no);
			fprintf (pp, "!%06ld" , 	cmrh_rec.req_no);
			fprintf (pp, "!%-6.6s" , 	cmhr_rec.cont_no);
			fprintf (pp, "!%9.9s" , 	cumr_rec.dbt_acronym);
			fprintf (pp, "!%16.16s", 	inmr_rec.item_no);
			fprintf (pp, "!%-19.19s" , 	inmr_rec.description);
			fprintf (pp, "!%10.2f", 	DOLLARS (SR.cost));
			fprintf (pp, "!%-4.4s", 	GetUOM (line_cnt, i));
			fprintf (pp, "!%8.2f", 		GetQty (line_cnt, i));
			fprintf (pp, "|%10.10s", 	DateToString (local_rec.lsystemDate));
			fprintf (pp, "!%-10.10s", 	GetLoc (line_cnt, i));
			fprintf (pp, "!%-16.16s", 	cmhr_rec.wip_glacc);
			fprintf (pp, "!%-16.16s", 	SR.creditAccount);
			fprintf (pp, "!%10.2f!\n", 	DOLLARS (local_rec.qtyIssued * SR.cost));
		}
		else
		{
			fprintf (pp, "!%2.2s", 		" ");
			fprintf (pp, "!%-6.6s" , 	" ");
			fprintf (pp, "!%-6.6s" , 	" ");
			fprintf (pp, "!%9.9s" , 		" ");
			fprintf (pp, "!%16.16s", 	" ");
			fprintf (pp, "!%-19.19s" , 	" ");
			fprintf (pp, "!%-10.10s", 	" ");
			fprintf (pp, "!%-4.4s", 	GetUOM (line_cnt, i));
			fprintf (pp, "!%8.2f", 		GetQty (line_cnt, i));
			fprintf (pp, "!%8.8s", 		" ");
			fprintf (pp, "!%-10.10s", 	GetLoc (line_cnt, i));
			fprintf (pp, "!%-16.16s", 	" ");
			fprintf (pp, "!%-16.16s", 	" ");
			fprintf (pp, "!%-10.10s!\n", " ");
		}
	}

	fflush (pp);
}

/*
 * Routine to close the audit trail output file. 
 */
void
CloseAudit (void)
{
	fprintf (pp, "!--");
	fprintf (pp, "!------");
	fprintf (pp, "!------");
	fprintf (pp, "!---------");
	fprintf (pp, "!----------------");
	fprintf (pp, "!-------------------");
	fprintf (pp, "!----------");
	fprintf (pp, "!----");
	fprintf (pp, "!--------");
	fprintf (pp, "!--------");
	fprintf (pp, "!----------");
	fprintf (pp, "!----------------");
	fprintf (pp, "!----------------");
	fprintf (pp, "!----------!\n");

	fprintf (pp, ".EOF\n");
	pclose (pp);
}

/*
 * To standard unit of measure 
 */
float
ToStdUom (
 float	lclQty)
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
 float	lclQty)
{
    float   cnvQty;

    if (F_HIDE (label ("UOM")))
        return (lclQty);

    if (SR.cnvFct == 0.00 || lclQty == 0.00)
        return (0.00);

    cnvQty = lclQty / SR.cnvFct;

    return (cnvQty);
}

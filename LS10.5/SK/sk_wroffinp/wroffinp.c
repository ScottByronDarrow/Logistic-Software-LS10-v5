/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: wroffinp.c,v 5.9 2002/07/24 08:39:20 scott Exp $
|  Program Name  : (sk_wroffinp.c )                                   |
|  Program Desc  : (Stock Write-Off Input.                      )     |
|---------------------------------------------------------------------|
|  Author        : Scott Darrow.   | Date Written : 10/05/86          |
|---------------------------------------------------------------------|
| $Log: wroffinp.c,v $
| Revision 5.9  2002/07/24 08:39:20  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.8  2002/07/18 07:15:56  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.7  2002/07/16 09:37:10  scott
| S/C 004154 - Locking on search - Oracle.
|
| Revision 5.6  2002/06/20 07:11:18  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.5  2002/01/09 01:16:59  scott
| Updated to change function FindInsfCost () to accept an additional argument of
| the hhbrHash. This allows a serial item to be found using the hhwhHash OR the
| hhbrHash. Used in stock updates in case a serial item has been transfered.
|
| Revision 5.4  2001/08/09 09:20:27  scott
| Updated to add FinishProgram () function
|
| Revision 5.3  2001/08/07 23:31:18  scott
| RELEASE 5.0
|
| Revision 5.2  2001/08/06 23:46:12  scott
| RELEASE 5.0
|
| Revision 5.1  2001/07/25 02:19:45  scott
| Update - LS10.5
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: wroffinp.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_wroffinp/wroffinp.c,v 5.9 2002/07/24 08:39:20 scott Exp $";

#define MAXSCNS		2
#define MAXWIDTH	160
#define MAXLINES	200
#define	TABLINES	10

#include <pslscr.h>
#include <GlUtils.h>
#include <ml_std_mess.h>
#include <ml_sk_mess.h>

extern	int	EnvScreenOK;
#define	SR		store [line_cnt]
#define	PSR		store [line_cnt + 1]
#define	SERIAL		 (inmr_rec.costing_flag [0] == 'S')

	/*----------------------------------------------------------------
	| Special fields and flags  ################################## . |
	----------------------------------------------------------------*/

	FILE	*pp;

	int		printerNumber	=	1;
	char	*serialSpace = "                         ",
			issueAccount [MAXLEVEL + 1];

	long	issueHhmrHash = 0L;

	double	batchTotal = 0;

	float	quantityTotal = 0;

	struct	storeRec {
		char	cost_flag [2];
		char	ser_flag [2];
		long	hhwhHash;
		long	hhbrHash;
		char	serialNo [26];
		char	dbt_acc [MAXLEVEL + 1];
		long	dbt_hash;
		char	crd_acc [MAXLEVEL + 1];
		long	crd_hash;
		double	avgeCost;
		double	lastCost;
		double	stdCost;
		char	_lotControl [2];
		float	_qty;
	} store [MAXLINES];

#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct ccmrRecord	ccmr_rec;
struct exwoRecord	exwo_rec;
struct inccRecord	incc_rec;
struct inwuRecord	inwu_rec;
struct inmrRecord	inmr_rec;
struct inwoRecord	inwo_rec;

/*==========
 Table Names
============*/
static char		*data	= "data";

#include	<MoveRec.h>
#include	<Costing.h>

int		auditOpen = FALSE;

/*===========================
| Local & Screen Structures.|
===========================*/
struct {
	char	dummy [11];
	char	localCurrency [4];
	char	writeOffCode [3];
	char	systemDate [11];
	long	lsystemDate;
	char	previousItem [17];
	char	journalReference [9];
	long	journalDate;
	char	lotControl [2];
	float	workQuantity;
	double	workCost;
	char	item_no [17];
	long	hhwhHash;
	long	receiptDate;
	char	dfltJnlDate [11];
	char	woffDate [11];
	char	previousSerialNo [26];
	char	serialNo [26];
	char	itemDescription [41];
	char	LL [2];
} local_rec;

static	struct	var	vars [] =
{
	{1, LIN, "reason",	 3, 25, CHARTYPE,
		"UU", "          ",
		"", " ", "Reason Code", " ",
		YES, NO, JUSTLEFT, "", "", local_rec.writeOffCode},
	{1, LIN, "res_desc",	 3, 60, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAA", "          ",
		"", "", "Reason Description", " ",
		 NA, NO, JUSTLEFT, "", "", exwo_rec.description},
	{1, LIN, "ref",		 4, 25, CHARTYPE,
		"AAAAAAAA", "          ",
		"0", "0", "Journal Ref.", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.journalReference},
	{1, LIN, "jdate",	 4, 60, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", local_rec.dfltJnlDate, "Journal date    ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.journalDate},
	{2, TAB, "item_no",	MAXLINES, 0, CHARTYPE,
		"UUUUUUUUUUUUUUUU", "          ",
		" ", " ", "  Item number.  ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.item_no},
	{2, TAB, "desc",		 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "            Item Description.           ", " ",
		 ND, NO,  JUSTLEFT, "", "", local_rec.itemDescription},
	{2, TAB, "serialNo",	 0, 0, CHARTYPE,
		"UUUUUUUUUUUUUUUUUUUUUUUUU", "          ",
		" ", local_rec.previousSerialNo, "       Reference.        ", " ",
		YES, NO,  JUSTLEFT, "", "", local_rec.serialNo},
	{2, TAB, "qty",		 0, 0, FLOATTYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", " Quantity ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.workQuantity},
	{2, TAB, "LL", 	 0, 0, CHARTYPE, 
		"U", "          ", 
		" ", "N", "L", "Lot / Location Selection. <return> ", 
		 YES, NO,  JUSTLEFT, "", "", local_rec.LL}, 
	{2, TAB, "cost",		 0, 0, DOUBLETYPE,
		"NNNNNNN.NN", "          ",
		" ", "0", " Est Cost ", " ",
		 NA, NO, JUSTRIGHT, "", "", (char *) &local_rec.workCost},
	{2, TAB, "roffDate",	 0, 0, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", local_rec.woffDate, " Date.  ", " ",
		YES, NO, JUSTRIGHT, "", "", (char *) &local_rec.receiptDate},
	{0, LIN, "",		 0, 0, INTTYPE,
		"A", "          ",
		" ", "", "dummy", " ",
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy},
};

#include	<MoveAdd.h>
#include	<ser_value.h>
#include	<LocHeader.h>

/*=======================
| Function Declarations |
=======================*/
void 	OpenDB 					(void);
void 	CloseDB 				(void);
void 	ReadCcmr 				(void);
int  	heading 				(int);
int  	spec_valid 				(int);
int  	DeleteLine 				(void);
static 	int 	validateDate 	(long);
void 	Update 					(void);
void 	AddGlwk 				(void);
void 	PrintDetails 			(void);
void 	OpenAudit 				(void);
void 	CloseAudit 				(void);
int  	CheckDuplicateSerial 	(char *, long, int);
int  	GetGlAccounts 			(char *, int);
void 	SrchExwo 				(char *);
void 	tab_other 				(int);
void 	PrintCoStuff 			(void);


/*==========================
| Main Processing Routine. |
==========================*/
int
main (
 int argc, 
 char * argv [])
{
	if (argc != 2)
	{
		print_at (0,0,ML (mlSkMess639), argv [0]);
		return (EXIT_FAILURE);
	}

	EnvScreenOK	=	FALSE;

	printerNumber = atoi (argv [1]);

	SETUP_SCR (vars);

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

	FLD ("LL") = ND;
	if (SK_BATCH_CONT || MULT_LOC)
		FLD ("LL") = YES;

	GL_SetMask (GlFormat);

	strcpy (local_rec.dfltJnlDate, DateToString (comm_rec.inv_date));
	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));
	local_rec.lsystemDate = TodaysDate ();

	swide ();

	tab_row = 7;

	while (!prog_exit)
	{
		/*  reset control flags  */
		entry_exit	=	FALSE;
		restart		=	FALSE;
		edit_exit	=	FALSE;
		prog_exit	=	FALSE;
		search_ok	=	TRUE;

		init_vars (1);
		init_vars (2);
		lcount [2] = 0;

		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		if (strcmp (SR.ser_flag, "Y"))
  		strcpy (local_rec.previousSerialNo, "                         "); 
		lcount [2] = 0;
		heading (2);
		entry (2);
		if (restart)
			continue;

		edit_all ();
		if (restart)
			continue;

		if (restart)
			continue;

		Update ();
		if (auditOpen)
			CloseAudit ();
	}

	/*========================
	| Program exit sequence. |
	========================*/
	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

/*========================
| Open data base files . |
========================*/
void
OpenDB (
 void)
{
	MoveOpen	=	TRUE;

	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	open_rec (comr, comr_list, COMR_NO_FIELDS, "comr_co_no");

	strcpy (comr_rec.co_no, comm_rec.co_no);
	cc = find_rec (comr, &comr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, comr, "DBFIND");

	if (!strcmp (comr_rec.base_curr, "   "))
		sprintf (local_rec.localCurrency, "%-3.3s", get_env ("CURR_CODE"));
	else
		sprintf (local_rec.localCurrency, "%-3.3s", comr_rec.base_curr);

	ReadCcmr ();
	open_rec (exwo, exwo_list, EXWO_NO_FIELDS, "exwo_id_no");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (inwo, inwo_list, INWO_NO_FIELDS, "inwo_hhwo_hash");
	open_rec (inwu, inwu_list, INWU_NO_FIELDS, "inwu_id_no");
	open_rec ("move", move_list, MOVE_NO_FIELDS, "move_move_hash");
	OpenLocation (ccmr_rec.hhcc_hash);
	strcpy (StockTake, "Y");

	OpenGlmr ();
	GL_OpenBatch (comm_rec.co_no, comm_rec.est_no);
}

/*========================
| Close data base files. |
========================*/
void
CloseDB (
 void)
{
	abc_fclose (exwo);
	abc_fclose (incc);
	abc_fclose (inmr);
	abc_fclose (inwo);
	abc_fclose (inwu);
	abc_fclose ("move");
	CloseLocation ();
	GL_CloseBatch (printerNumber);
	GL_Close ();
	SearchFindClose ();
	CloseCosting ();
	abc_dbclose (data);
}

void	
ReadCcmr (
 void)
{
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");

	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, comm_rec.est_no);
	strcpy (ccmr_rec.cc_no, comm_rec.cc_no);
	cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
	if (cc)
	{
		errmess (ML (mlStdMess198));
		file_err (cc, (char *)ccmr, "DBFIND");
	}
	abc_fclose (ccmr);
}

int
heading (
 int scn)
{
	clear ();

	rv_pr (ML (mlSkMess127), 58, 0, 1);
	box (0, 2, 130, 2);

	print_at (0,102,ML (mlSkMess089), local_rec.previousItem);
	
	move (0, 1);
	line (130);
	PrintCoStuff ();
	if (scn != 1)
	{
		scn_set (1);
		scn_write (1);
		scn_display (1);
	}
	line_cnt = 0;
	scn_set (scn);
	scn_write (scn);
    return (EXIT_SUCCESS);
}

int
spec_valid (
 int field)
{
	int	i;
	int	TempLine;

	if (LCHECK ("reason"))
	{
		if (SRCH_KEY)
		{
			SrchExwo (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (exwo_rec.co_no, comm_rec.co_no);
		sprintf (exwo_rec.code, "%-2.2s", local_rec.writeOffCode);
		cc = find_rec (exwo, &exwo_rec, EQUAL, "r");
		if (cc)
		{
			errmess (ML (mlStdMess163));
			return (EXIT_FAILURE);
		}

		abc_selfield (glmr, "glmr_hhmr_hash");

		glmrRec.hhmr_hash	=	exwo_rec.hhmr_hash;
		cc = find_rec (glmr, &glmrRec, EQUAL, "r");
		if (cc)
		{
			errmess (ML (mlStdMess242));
			abc_selfield (glmr, "glmr_hhmr_hash");
			return (EXIT_FAILURE);
		}
		abc_selfield (glmr, "glmr_id_no");

		if (glmrRec.glmr_class [2] [0] != 'P')
		{
			errmess (ML (mlStdMess025));
			return (EXIT_FAILURE);
		}

		issueHhmrHash = glmrRec.hhmr_hash;
		sprintf (issueAccount,"%-*.*s", MAXLEVEL,MAXLEVEL,glmrRec.acc_no);

		DSP_FLD ("res_desc");

		return (EXIT_SUCCESS);
	}

	/*---------------
	| Journal Date. |
	---------------*/
	if (LCHECK ("jdate"))
		return (validateDate (local_rec.journalDate));

	/*-----------------------
	| Validate Item Number. |
	-----------------------*/
	if (LCHECK ("item_no"))
	{
		if (dflt_used  || last_char == DELLINE)
			return (DeleteLine ());

		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}
		clear_mess ();

		cc = FindInmr (comm_rec.co_no, local_rec.item_no, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.item_no);
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			errmess (ML (mlStdMess001));
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();

		strcpy (local_rec.item_no, inmr_rec.item_no);
		DSP_FLD ("item_no");

		if (GetGlAccounts (inmr_rec.category, line_cnt))
			return (EXIT_FAILURE);

		/*------------------------------------------
		| Look up to see if item is on Cost Centre |
		------------------------------------------*/
		incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
		incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
		cc = find_rec (incc, &incc_rec, COMPARISON, "r");
		if (cc)
		{
			clear_mess ();
			errmess (ML (mlStdMess027));
			return (EXIT_FAILURE);
		}
		local_rec.hhwhHash = incc_rec.hhwh_hash;

		SR.hhwhHash = incc_rec.hhwh_hash;
		SR.hhbrHash = incc_rec.hhbr_hash;
		strcpy (local_rec.lotControl, inmr_rec.lot_ctrl);
		strcpy (SR._lotControl, inmr_rec.lot_ctrl);
		strcpy (local_rec.itemDescription, inmr_rec.description);

		tab_other (line_cnt);

		strcpy (SR.cost_flag, inmr_rec.costing_flag);
		strcpy (SR.ser_flag,  inmr_rec.serial_item);
		return (EXIT_SUCCESS);
	}

	/*-----------------------------------
	| Check If Serial No already exists |
	-----------------------------------*/
	if (LCHECK ("serialNo"))
	{
		if (last_char == EOI && prog_status == ENTRY)
		{
			skip_entry = -2;
			return (EXIT_SUCCESS);
		}

		if (prog_status == ENTRY)
			strcpy (local_rec.previousSerialNo, local_rec.serialNo);

		if (end_input)
			return (EXIT_SUCCESS);

		if (strcmp (SR.ser_flag, "Y"))
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			SearchInsf (SR.hhwhHash, "F", temp_str);
			return (EXIT_SUCCESS);
		}
		if (dflt_used)
		{
			print_mess (ML (mlStdMess201));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		local_rec.workCost	=	FindInsfCost 
								(
									SR.hhwhHash, 
									0L,
									local_rec.serialNo, 
									"F"
								);
		if (local_rec.workCost == -1.00)
		{
			errmess (ML (mlStdMess201));
			return (EXIT_FAILURE);
		}
		local_rec.workQuantity = 1.00;
		SR._qty	=	local_rec.workQuantity;
		DSP_FLD ("qty");
		DSP_FLD ("cost");
		skip_entry = goto_field (field, label ("roffDate"));
		if (CheckDuplicateSerial (local_rec.serialNo, SR.hhbrHash, line_cnt))
		{
			print_mess (ML (mlStdMess097));
			return (EXIT_FAILURE);
		}
		strcpy (SR.serialNo, local_rec.serialNo);
		return (EXIT_SUCCESS);
	}

	/*------------------------
	| Qty validation lookup. |
	------------------------*/
	if (LCHECK ("qty"))
	{
		if (SERIAL)
		{
			local_rec.workQuantity = 1.00;
			SR._qty	=	local_rec.workQuantity;
			return (EXIT_SUCCESS);
		}

		if (last_char == EOI && prog_status == ENTRY)
		{
			skip_entry = -3;
			return (EXIT_SUCCESS);
		}
		if (prog_status == ENTRY)
		{
			if (incc_rec.closing_stock < local_rec.workQuantity && local_rec.workQuantity > 0.00)
			{

				sprintf (err_str,ML (mlSkMess125), local_rec.workQuantity, 
					incc_rec.closing_stock, clip (local_rec.item_no));
				i = prmptmsg (err_str, "YyNn", 1, 20);
				move (0, 20);
				cl_line ();
				if (i == 'n' || i == 'N')
					return (EXIT_FAILURE);
			}
		}
		SR._qty	=	local_rec.workQuantity;
		return (EXIT_SUCCESS);
	}
	/*------------------------------
	| Validate lots and locations. |
	------------------------------*/
	if (LCHECK ("LL"))
	{
		if (FLD ("LL") == ND)
			return (EXIT_SUCCESS);

		LL_EditLot	=	 (SR._lotControl [0] == 'Y') ? TRUE : FALSE;
		LL_EditSLot	=	 (SR._lotControl [0] == 'Y') ? TRUE : FALSE;
		LL_EditLoc	=	TRUE;
		LL_EditDate	=	TRUE;

		TempLine	=	lcount [2];
		cc = DisplayLL
			 (										/*----------------------*/
				line_cnt,							/*	Line number.		*/
				tab_row + 3 + (line_cnt % TABLINES),/*  Row for window		*/
				tab_col + 22,						/*  Col for window		*/
				4,									/*  length for window	*/
				SR.hhwhHash, 						/*	Warehouse hash.		*/
				inmr_rec.std_uom,				/*	UOM hash			*/
				ccmr_rec.hhcc_hash,				/*	CC hash.			*/
				inmr_rec.sale_unit,				/* UOM					*/
				SR._qty,							/* Quantity.			*/
				1.00,								/* Conversion factor.	*/
				TodaysDate (),						/* Expiry Date.			*/
				FALSE,								/* Silent mode			*/
				 (local_rec.LL [0] == 'Y'),			/* Input Mode.			*/
				SR._lotControl						/* Lot controled item. 	*/
													/*----------------------*/
			);
		/*-----------------
		| Redraw screens. |
		-----------------*/
		strcpy (local_rec.LL, "Y");
		putval (line_cnt);

		lcount [2] = (line_cnt + 1 > lcount [2]) ? line_cnt + 1 : lcount [2];
		scn_write (2);
		scn_display (2);
		lcount [2] = TempLine;
		PrintCoStuff ();
		if (cc)
			return (EXIT_FAILURE);
		
		return (EXIT_SUCCESS);
	}

	/*-------------------------------
	| Cost lookup (if none entered).|
	-------------------------------*/
	if (LCHECK ("cost"))
	{
		if (last_char == EOI && prog_status == ENTRY)
		{
			skip_entry = -4;
			return (EXIT_SUCCESS);
		}

		if (end_input)
			return (EXIT_SUCCESS);

		clear_mess ();

		SR.avgeCost = FindIneiCosts ("A", comm_rec.est_no, inmr_rec.hhbr_hash);
		SR.lastCost = FindIneiCosts ("L", comm_rec.est_no, inmr_rec.hhbr_hash);
		SR.stdCost 	= FindIneiCosts ("T", comm_rec.est_no, inmr_rec.hhbr_hash);

		strcpy (inmr_rec.costing_flag, SR.cost_flag);
		switch (inmr_rec.costing_flag [0])
		{
		case 'A':
		case 'L':
		case 'P':
		case 'T':
			local_rec.workCost =	FindIneiCosts 
									(
										inmr_rec.costing_flag,
										comm_rec.est_no, 
										inmr_rec.hhbr_hash
									);
			break;

		case 'F':
			local_rec.workCost =	FindIncfCost 
									(
										SR.hhwhHash,
					     				incc_rec.closing_stock,
					     				SR._qty, 
										TRUE,
										inmr_rec.dec_pt
									);

			break;

		case 'I':
			local_rec.workCost =	FindIncfCost 
									(
										SR.hhwhHash,
					     				incc_rec.closing_stock,
					     				SR._qty, 
										FALSE,
										inmr_rec.dec_pt
									);
			break;

		case 'S':
			local_rec.workCost =	FindInsfCost 
									(
										SR.hhwhHash, 
										0L,
										local_rec.serialNo, 
										"F"
									);
			if (local_rec.workCost == -1.00)
				local_rec.workCost =	FindInsfValue 
										(
											SR.hhwhHash, 
											TRUE
										);
			break;

		}
		if (local_rec.workCost <= 0.00)
			local_rec.workCost = SR.stdCost;

		DSP_FLD ("cost");
		return (EXIT_SUCCESS);
	}
	/*---------------
	| Journal Date. |
	---------------*/
	if (LCHECK ("roffDate"))
		return (validateDate (local_rec.receiptDate));

	return (EXIT_SUCCESS);
}

int
DeleteLine (
 void)
{
	int	i;
	int	this_page = line_cnt / TABLINES;
	/*-----------------------
	| entry					|
	-----------------------*/
	if (prog_status == ENTRY)
	{
		print_mess (ML (mlStdMess005));
		return (EXIT_FAILURE);
	}
	/*-------------------------------
	| no lines to delete			|
	-------------------------------*/
	if (lcount [2] <= 0)
	{
		print_mess (ML (mlStdMess032));
		return (EXIT_FAILURE);
	}
	/*---------------------------
	| delete lines				|
	---------------------------*/
	for (i = line_cnt;line_cnt < lcount [2] - 1 ;line_cnt++)
	{
		strcpy (SR.cost_flag, PSR.cost_flag);
		strcpy (SR.ser_flag, PSR.ser_flag);
		strcpy (SR.serialNo, PSR.serialNo);
		SR.hhwhHash 	= PSR.hhwhHash;
		SR.hhbrHash 	= PSR.hhbrHash;
		SR.dbt_hash 	= PSR.dbt_hash;
		SR.crd_hash 	= PSR.crd_hash;
		SR._qty 		= PSR._qty;
		SR.avgeCost 	= PSR.avgeCost;
		SR.lastCost 	= PSR.lastCost;
		SR.stdCost 	= PSR.stdCost;
		sprintf (SR.dbt_acc, "%-*.*s",MAXLEVEL,MAXLEVEL,PSR.dbt_acc);
		sprintf (SR.crd_acc, "%-*.*s", MAXLEVEL,MAXLEVEL,PSR.crd_acc);

		getval (line_cnt + 1);
		putval (line_cnt);

		LotMove (line_cnt, line_cnt + 1);

		if (line_cnt / TABLINES == this_page)
			line_display ();
	}

	/*-----------------------------------
	| blank last line - if required		|
	-----------------------------------*/
	if (line_cnt / TABLINES == this_page)
		blank_display ();

	/*-------------------------------
	| zap buffer if deleted all		|
	-------------------------------*/
	if (lcount [2] <= 0)
	{
		init_vars (2);
		putval (i);
	}

	lcount [2] -=1;
	line_cnt = i;
	getval (line_cnt);
	return (EXIT_SUCCESS);
}

static	int	
validateDate (
 long	inputDate)
{
	if (inputDate < MonthStart (comm_rec.inv_date) ||
			inputDate > MonthEnd (comm_rec.inv_date))
	{
		return print_err (ML (mlSkMess344));
	}

	strcpy (local_rec.woffDate, DateToString (inputDate));

	return (EXIT_SUCCESS);
}

/*===============================
| add transaction to incc files |
===============================*/
void
Update (
 void)
{
	char	workReference [11];
	int		tran_type = 0;
	int		NoLots	=	FALSE;
	int		i;

	clear ();
	print_at (0,0,ML (mlStdMess035));
	fflush (stdout);

	/*--------------------------------------------------
	| Update all inventory and general ledger records. |
	--------------------------------------------------*/
	scn_set (2);
	for (line_cnt = 0; line_cnt < lcount [2]; line_cnt++)
	{
		getval (line_cnt);

		/*-------------------------------------------------
		| Find inmr record from item number in structure. |
		-------------------------------------------------*/
		strcpy (inmr_rec.co_no, comm_rec.co_no);
		strcpy (inmr_rec.item_no, local_rec.item_no);
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "u");
		if (cc)
			file_err (cc, (char *)inmr, "DBFIND");

		if (inmr_rec.hhsi_hash != 0L)
			inmr_rec.hhbr_hash = inmr_rec.hhsi_hash;

		/*----------------------------------------------
		| Find warehouse record from master item hash. |
		----------------------------------------------*/
		incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
		incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
		cc = find_rec (incc, &incc_rec, COMPARISON, "u");
		if (cc)
			file_err (cc, incc, "DBFIND");

		local_rec.hhwhHash = incc_rec.hhwh_hash;

		/*---------------------------------------------------
		| Procure the correct costing for the user-defined  |
		| quantity of the current item.		                |
		---------------------------------------------------*/
		switch (inmr_rec.costing_flag [0])
		{
		case 'A':
		case 'L':
		case 'P':
		case 'T':
			local_rec.workCost =	FindIneiCosts 
									(
										inmr_rec.costing_flag,
										comm_rec.est_no, 
										inmr_rec.hhbr_hash
									);
			break;

		case 'F':
			local_rec.workCost =	FindIncfCost 
									(
										SR.hhwhHash,
					     				incc_rec.closing_stock,
					     				SR._qty, 
										TRUE,
										inmr_rec.dec_pt
									);

			break;

		case 'I':
			local_rec.workCost =	FindIncfCost 
									(
										SR.hhwhHash,
					     				incc_rec.closing_stock,
					     				SR._qty, 
										FALSE,
										inmr_rec.dec_pt
									);
			break;

		case 'S':
			local_rec.workCost =	FindInsfCost 
									(
										SR.hhwhHash, 
										0L,
										local_rec.serialNo, 
										"F"
									);
			if (local_rec.workCost == -1.00)
				local_rec.workCost =	FindInsfValue 
										(
											SR.hhwhHash, 
											TRUE
										);
			break;
		}
		if (local_rec.workCost <= 0.00)
			local_rec.workCost = SR.stdCost;

		/*-------------------------------------------------------
		| update inventory cost centre stock record (file incc)	|
		-------------------------------------------------------*/
		incc_rec.issues 	+= SR._qty;
		incc_rec.ytd_issues += SR._qty;

		incc_rec.closing_stock	= 	incc_rec.opening_stock +
									incc_rec.pur +
									incc_rec.receipts +
									incc_rec.adj -
									incc_rec.issues -
									incc_rec.sales;

		/*--------------------------------------
		| Store date when item is out of stock |
        --------------------------------------*/
		if (incc_rec.closing_stock <= 0.00)
		{
			if (incc_rec.os_date == 0L)
				incc_rec.os_date = comm_rec.inv_date;

			if (incc_rec.os_ldate == 0L)
				incc_rec.os_ldate = comm_rec.inv_date;
		}

		/*--------------------------
		| Update warehouse record. |
		--------------------------*/
		cc = abc_update (incc, &incc_rec);
		if (cc)
			file_err (cc, (char *)incc, "DBUPDATE");

		/*--------------------------------------
		| Find Warehouse unit of measure file. |
		--------------------------------------*/
		inwu_rec.hhwh_hash	=	incc_rec.hhwh_hash;
		inwu_rec.hhum_hash	=	inmr_rec.std_uom;
		cc = find_rec (inwu, &inwu_rec, COMPARISON,"u");
		if (cc)
		{
			abc_unlock (inwu);
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
		inwu_rec.issues	+= SR._qty;
		inwu_rec.closing_stock = inwu_rec.opening_stock +
								 inwu_rec.pur +
								 inwu_rec.receipts +
								 inwu_rec.adj -
								 inwu_rec.issues -
								 inwu_rec.sales;

		cc = abc_update (inwu,&inwu_rec);
		if (cc)
			file_err (cc, inwu, "DBUPDATE");

		inwo_rec.hhwo_hash 		= 0L;
		strcpy (inwo_rec.code, 		exwo_rec.code);
		inwo_rec.hhcc_hash 		= 	incc_rec.hhcc_hash;
		inwo_rec.hhbr_hash 		= 	incc_rec.hhbr_hash;
		strcpy (inwo_rec.narrative, 	local_rec.serialNo);
		inwo_rec.date 			= 	local_rec.receiptDate;
		inwo_rec.hhmr_hash 		= 	exwo_rec.hhmr_hash;
		inwo_rec.quantity 		= 	SR._qty;
		inwo_rec.cost_price 	= 	out_cost 
									(
										local_rec.workCost,
										inmr_rec.outer_size
									);
		inwo_rec.sale_price = 0.00;
		strcpy (inwo_rec.stat_flag, "0");
		cc = abc_add (inwo, &inwo_rec);
		if (cc)
			file_err (cc, inwo, "DBADD");

		if (SR._qty != 0.00)
		{
			if (SERIAL)
			{
				cc = UpdateInsf (SR.hhwhHash, 0L,local_rec.serialNo, "F", "S");
				if (cc)
					file_err (cc, insf, "DBUPDATE");
			}
			if (MULT_LOC || SK_BATCH_CONT)
				UpdateLotLocation (line_cnt, TRUE);
		}

		strcpy (inmr_rec.item_no, local_rec.item_no);

		inmr_rec.on_hand -= SR._qty;

		/*----------------------------------
		| Store "o/s" to inmr_ex_code      |
		----------------------------------*/
		if (inmr_rec.on_hand <= 0.00)
			sprintf (inmr_rec.ex_code, "%-3.3s", "o/s");

		/*----------------------------------
		| Update inventory master records. |
		----------------------------------*/
		cc = abc_update (inmr, &inmr_rec);
		if (cc)
			file_err (cc, (char *)inmr, "DBUPDATE");

		sprintf (workReference, "%-10.10s", local_rec.serialNo);

		tran_type = 11;

		NoLots	=	TRUE;
		for (i = 0; i < MAX_LOTS; i++)
		{
			if (!LL_Valid (line_cnt, i))
				break;

			if (GetBaseQty (line_cnt, i) == 0.00)
				continue;

			NoLots	=	FALSE;

			/*--------------------------
			| Log inventory movements. |
			--------------------------*/
			MoveAdd 
			 (
				comm_rec.co_no,
				comm_rec.est_no,
				comm_rec.cc_no,
				incc_rec.hhbr_hash,
				incc_rec.hhcc_hash,
				inmr_rec.std_uom,
				local_rec.receiptDate,
				tran_type,
				GetLotNo (line_cnt, i),
				inmr_rec.inmr_class,
				inmr_rec.category,
				local_rec.journalReference,
				workReference,				/* REF 2 */
				GetBaseQty (line_cnt, i),
				0.00,
				CENTS (local_rec.workCost)
			);
		}
		if (NoLots)
		{
			/*--------------------------
			| Log inventory movements. |
			--------------------------*/
			MoveAdd 
			 (
				comm_rec.co_no,
				comm_rec.est_no,
				comm_rec.cc_no,
				incc_rec.hhbr_hash,
				incc_rec.hhcc_hash,
				inmr_rec.std_uom,
				local_rec.receiptDate,
				tran_type,
				" ",
				inmr_rec.inmr_class,
				inmr_rec.category,
				local_rec.journalReference,
				workReference,				/* REF 2 */
				local_rec.workQuantity,
				0.00,
				CENTS (local_rec.workCost)
			);
		}
		/*---------------------------------
		| Somes details have been printed |
		---------------------------------*/
		PrintDetails (); 

		/*--------------------------------------------
		| Add General Ledger inventory transactions. |
		--------------------------------------------*/
		AddGlwk ();
	}
	strcpy (local_rec.previousItem, inmr_rec.item_no);
	return;
}

/*================================
| Add transactions to glwk file. |
================================*/
void
AddGlwk (
 void)
{
	int		monthNum;

	double	workValue = out_cost (local_rec.workCost, inmr_rec.outer_size);

	strcpy (glwkRec.co_no, comm_rec.co_no);
	strcpy (glwkRec.tran_type, "12");
	glwkRec.post_date	=	StringToDate (local_rec.dfltJnlDate);
	glwkRec.tran_date	=	local_rec.receiptDate;

	DateToDMY (local_rec.receiptDate, NULL, &monthNum, NULL);
	sprintf (glwkRec.period_no, "%02d", monthNum);

	sprintf (glwkRec.sys_ref, "%5.1d", comm_rec.term);
	sprintf (glwkRec.user_ref, "%8.8s", local_rec.journalReference);
	strcpy (glwkRec.stat_flag, "2");
	sprintf (glwkRec.narrative, "Stk Write-off W.H %s", comm_rec.cc_no);
	strcpy (glwkRec.alt_desc1, " ");
	strcpy (glwkRec.alt_desc2, " ");
	strcpy (glwkRec.alt_desc3, " ");
	strcpy (glwkRec.batch_no, " ");
	glwkRec.amount = CENTS ( (SR._qty * workValue));

	sprintf (glwkRec.acc_no, "%-*.*s", MAXLEVEL,MAXLEVEL,SR.dbt_acc);
	glwkRec.hhgl_hash	=	SR.dbt_hash;
	strcpy (glwkRec.jnl_type, "1");
	glwkRec.loc_amount	=	glwkRec.amount;
	glwkRec.exch_rate	=	1.00;
	strcpy (glwkRec.currency, local_rec.localCurrency);
	GL_AddBatch ();

	sprintf (glwkRec.acc_no, "%-*.*s", MAXLEVEL,MAXLEVEL,SR.crd_acc);
	glwkRec.hhgl_hash	=	SR.crd_hash;
	strcpy (glwkRec.jnl_type, "2");
	glwkRec.loc_amount	=	glwkRec.amount;
	glwkRec.exch_rate	=	1.00;
	strcpy (glwkRec.currency, local_rec.localCurrency);
	GL_AddBatch ();
}

/*==============================
| Print details of data input. |
==============================*/
void
PrintDetails (
 void)
{
	int		firstPrintLine	=	TRUE;
	int		i;
	int		NoLots	=	TRUE;
	double	value = out_cost (local_rec.workCost, inmr_rec.outer_size);

	if (!auditOpen)
		OpenAudit ();

	auditOpen = TRUE;

	if (SK_BATCH_CONT && MULT_LOC)
	{
		for (i = 0; i < MAX_LOTS; i++)
		{
			if (!LL_Valid (line_cnt, i))
				break;

			if (GetBaseQty (line_cnt, i) == 0.00)
				continue;

			NoLots	=	FALSE;
			/*---------------------------------
			| Somes details have been printed |
			---------------------------------*/
			if (firstPrintLine)
			{
				fprintf (pp, "|%-16.16s",	inmr_rec.item_no);
				if (inmr_rec.serial_item [0] == 'Y')
					fprintf (pp, "|%-25.25s",	local_rec.serialNo);
				else
					fprintf (pp, "|%-25.25s",	inmr_rec.description);

				fprintf (pp, "|%-7.7s",		GetLotNo (line_cnt, i));
				fprintf (pp, "|%10.2f",		local_rec.workCost);
				fprintf (pp, "|%11.2f",		GetBaseQty (line_cnt , i));
				fprintf (pp, "|%-10.10s",	DateToString (local_rec.receiptDate));
				fprintf (pp, "|%-10.10s",	GetLoc (line_cnt, i));
				fprintf (pp, "|%-16.16s",	SR.dbt_acc);
				fprintf (pp, "|%-16.16s",	SR.crd_acc);
				fprintf (pp, "|%11.2f|\n",	SR._qty * value);
				firstPrintLine	=	FALSE;
			}
			else
			{
				fprintf (pp, "|                ");
				fprintf (pp, "|                         ");
				fprintf (pp, "|       ");
				fprintf (pp, "|          ");
				fprintf (pp, "|%11.2f",		GetBaseQty (line_cnt , i));
				fprintf (pp, "|          ");
				fprintf (pp, "|%-10.10s",	GetLoc (line_cnt, i));
				fprintf (pp, "|                ");
				fprintf (pp, "|                ");
				fprintf (pp, "|           |\n");
			}
		}
	}
	if (NoLots)
	{
		/*---------------------------------
		| Somes details have been printed |
		---------------------------------*/
		fprintf (pp, "|%-16.16s",	inmr_rec.item_no);
		if (inmr_rec.serial_item [0] == 'Y')
		{
			fprintf (pp, "|%-25.25s",	local_rec.serialNo);
		}
		else	
			fprintf (pp, "|%-25.25s",	inmr_rec.description);

		if (SK_BATCH_CONT || MULT_LOC)
			fprintf (pp, "|       ");
		fprintf (pp, "|%10.2f",		local_rec.workCost);
		fprintf (pp, "|%11.2f",		SR._qty);
		fprintf (pp, "|%-10.10s",	DateToString (local_rec.receiptDate));
		if (SK_BATCH_CONT || MULT_LOC)
			fprintf (pp, "|          ");
		fprintf (pp, "|%-16.16s",	SR.dbt_acc);
		fprintf (pp, "|%-16.16s",	SR.crd_acc);
		fprintf (pp, "|%11.2f|\n",	SR._qty * value);
		fflush (pp);
	}

	batchTotal += (double) SR._qty * value;
	quantityTotal   += SR._qty;
}
/*=========================================================================
| Routine to open output pipe to standard print to provide an audit trail |
| of events. This also sends the output straight to the spooler.          |
=========================================================================*/
void
OpenAudit (
 void)
{
	if ( (pp = popen ("pformat", "w")) == 0)
		file_err (cc, "pformat", "POPEN");

	fprintf (pp, ".START%s<%s>\n", DateToString (comm_rec.inv_date), PNAME);
	fprintf (pp, ".SO\n");
	fprintf (pp, ".LP%d\n", printerNumber);
	fprintf (pp, ".13\n");
	fprintf (pp, ".L158\n");
	fprintf (pp, ".ESTOCK WRITE-OFFS\n");

	fprintf (pp, ".B1\n");
	fprintf (pp, ".E%s AS AT %s\n", clip (comm_rec.co_short), SystemTime ());
	fprintf (pp, ".B1\n");

	fprintf (pp, ".EREASON: %-2.2s  %s\n", exwo_rec.code, exwo_rec.description);

	fprintf (pp, ".EBRANCH %s : Warehouse %s \n", clip (comm_rec.est_name), clip (comm_rec.cc_name));

	fprintf (pp, ".R=================");
	fprintf (pp, "==========================");
	if (SK_BATCH_CONT || MULT_LOC)
		fprintf (pp, "========");
	fprintf (pp, "===========");
	fprintf (pp, "============");
	fprintf (pp, "===========");
	if (SK_BATCH_CONT || MULT_LOC)
		fprintf (pp, "===========");
	fprintf (pp, "=================");
	fprintf (pp, "=================");
	fprintf (pp, "=============\n");

	fprintf (pp, "=================");
	fprintf (pp, "==========================");
	if (SK_BATCH_CONT || MULT_LOC)
		fprintf (pp, "========");
	fprintf (pp, "===========");
	fprintf (pp, "============");
	fprintf (pp, "===========");
	if (SK_BATCH_CONT || MULT_LOC)
		fprintf (pp, "===========");
	fprintf (pp, "=================");
	fprintf (pp, "=================");
	fprintf (pp, "=============\n");

	fprintf (pp, "|  ITEM NUMBER   ");
	fprintf (pp, "|        REFERENCE        ");
	if (SK_BATCH_CONT || MULT_LOC)
		fprintf (pp, "|  LOT  ");
	fprintf (pp, "|   @COST  ");
	fprintf (pp, "| QUANTITY  ");
	fprintf (pp, "|   DATE   ");
	if (SK_BATCH_CONT || MULT_LOC)
		fprintf (pp, "| LOCATION ");
	fprintf (pp, "|      DEBIT     ");
	fprintf (pp, "|     CREDIT     ");
	fprintf (pp, "|  EXTENDED |\n");

	fprintf (pp, "|                ");
	fprintf (pp, "|                         ");
	if (SK_BATCH_CONT || MULT_LOC)
		fprintf (pp, "|NUMBER ");
	fprintf (pp, "|          ");
	fprintf (pp, "|  ISSUED   ");
	fprintf (pp, "|  ISSUED  ");
	if (SK_BATCH_CONT || MULT_LOC)
		fprintf (pp, "|          ");
	fprintf (pp, "|     ACCOUNT    ");
	fprintf (pp, "|     ACCOUNT    ");
	fprintf (pp, "|  VALUE.   |\n");

	fprintf (pp, "|----------------");
	fprintf (pp, "|-------------------------");
	if (SK_BATCH_CONT || MULT_LOC)
		fprintf (pp, "|-------");
	fprintf (pp, "|----------");
	fprintf (pp, "|-----------");
	fprintf (pp, "|----------");
	if (SK_BATCH_CONT || MULT_LOC)
		fprintf (pp, "|----------");
	fprintf (pp, "|----------------");
	fprintf (pp, "|----------------");
	fprintf (pp, "|-----------|\n");

	fprintf (pp, ".PI12\n");
}

/*===============================================
| Routine to close the audit trail output file. |
===============================================*/
void
CloseAudit (
 void)
{
	fprintf (pp, "|----------------");
	fprintf (pp, "|-------------------------");
	if (SK_BATCH_CONT || MULT_LOC)
		fprintf (pp, "|-------");
	fprintf (pp, "|----------");
	fprintf (pp, "|-----------");
	fprintf (pp, "|----------");
	if (SK_BATCH_CONT || MULT_LOC)
		fprintf (pp, "|----------");
	fprintf (pp, "|----------------");
	fprintf (pp, "|----------------");
	fprintf (pp, "|-----------|\n");

	fprintf (pp, "|  BATCH TOTAL   ");
	fprintf (pp, "|                         ");
	if (SK_BATCH_CONT || MULT_LOC)
		fprintf (pp, "|       ");
	fprintf (pp, "|          ");
	fprintf (pp, "|%11.2f", quantityTotal);
	fprintf (pp, "|          ");
	if (SK_BATCH_CONT || MULT_LOC)
		fprintf (pp, "|          ");
	fprintf (pp, "|                ");
	fprintf (pp, "|                ");
	fprintf (pp, "|%11.2f|\n", batchTotal);

	fprintf (pp, ".EOF\n");
	pclose (pp);

	quantityTotal = 0.00;
	batchTotal = 0.00;

	auditOpen = FALSE;
}

/*=======================================================
| Check Whether A Serial Number For This Item Number	|
| Has Already Been Used.								|
| Return 1 if duplicate									|
=======================================================*/
int
CheckDuplicateSerial (
 char	*serialNo,
 long	hhbrHash,
 int	lineNumber)
{
	int	i;
	int	no_items = (prog_status == ENTRY) ? line_cnt : lcount [2];

	for (i = 0;i < no_items;i++)
	{
		/*-----------------------
		| Ignore Current Line	|
		-----------------------*/
		if (i == lineNumber)
			continue;

		/*---------------------------------------
		| cannot duplicate item_no/serialNo	|
		| unless serial no was not input	|
		---------------------------------------*/
		if (!strcmp (store [i].serialNo, serialSpace))
			continue;

		/*---------------------------------------
		| Only compare serial numbers for	|
		| the same item number			|
		---------------------------------------*/
		if (store [i].hhbrHash == hhbrHash)
		{
			if (!strcmp (store [i].serialNo, serialNo))
				return (EXIT_FAILURE);
		}
	}
	return (EXIT_SUCCESS);
}

/*===========================
| Process control Accounts. |
===========================*/
int
GetGlAccounts (
 char	*categoryNumber,
 int	linePosition)
{
	GL_GLI 
	(
		comm_rec.co_no,
		comm_rec.est_no,
		comm_rec.cc_no,
		"COSTSALE C",
		" ",
		categoryNumber
	);
	strcpy (store [linePosition].dbt_acc, issueAccount);
	strcpy (store [linePosition].crd_acc, glmrRec.acc_no);
	store [linePosition].dbt_hash = issueHhmrHash;
	store [linePosition].crd_hash = glmrRec.hhmr_hash;
	return (EXIT_SUCCESS);
}

void
SrchExwo (
 char    *key_val)
{
    _work_open (2,0,40);
	save_rec ("#Cd", "#Reason Description");

	strcpy (exwo_rec.co_no, comm_rec.co_no);
	sprintf (exwo_rec.code, "%-2.2s", key_val);
	cc = find_rec (exwo, &exwo_rec, GTEQ, "r");
    while (!cc && !strcmp (exwo_rec.co_no, comm_rec.co_no) &&
	    		  !strncmp (exwo_rec.code, key_val, strlen (key_val)))
	{
		cc = save_rec (exwo_rec.code, exwo_rec.description);
		if (cc)
			break;

		cc = find_rec (exwo, &exwo_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
	        return;

	strcpy (exwo_rec.co_no, comm_rec.co_no);
	sprintf (exwo_rec.code, "%-2.2s", temp_str);
	cc = find_rec (exwo, &exwo_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, exwo, "DBFIND");
}

void
tab_other (
 int line_no)
{
	if (prog_status == ENTRY || line_no < lcount [2])
	    print_at (6, 1,ML (mlSkMess099),line_no + 1, local_rec.itemDescription);
	else
	    print_at (6, 1,ML (mlSkMess099), line_no + 1, " ");
}

void
PrintCoStuff (
 void)
{
	line_at (20,0,130);
	print_at (21,0,ML (mlStdMess038), comm_rec.co_no, clip (comm_rec.co_short));
	print_at (21,45,ML (mlStdMess039), comm_rec.est_no, clip (comm_rec.est_short));
	print_at (22,0,ML (mlStdMess099), comm_rec.cc_no, clip (comm_rec.cc_name));
}

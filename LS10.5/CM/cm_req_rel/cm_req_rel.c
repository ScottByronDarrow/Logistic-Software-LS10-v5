/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: cm_req_rel.c,v 5.6 2002/07/18 06:12:21 scott Exp $
|  Program Name  : (cm_req_rel.c)
|  Program Desc  : (Requisition Forward/Backorder Release)
|---------------------------------------------------------------------|
|  Author        : Campbell Mander | Date Written  : (16/03/93)       |
|---------------------------------------------------------------------|
| $Log: cm_req_rel.c,v $
| Revision 5.6  2002/07/18 06:12:21  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.5  2002/07/03 04:21:41  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.4  2002/01/21 02:10:32  scott
| Updated as first phase of contract management cleanup on it's way to becoming
| useable for 3PL.
|
| Revision 5.3  2002/01/18 04:57:04  scott
| Updated as first phase of contract management cleanup on it's way to becoming
| useable for 3PL.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: cm_req_rel.c,v $", 
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/CM/cm_req_rel/cm_req_rel.c,v 5.6 2002/07/18 06:12:21 scott Exp $";

#define	MAXLINES	100
#include	<pslscr.h>
#include	<dsp_screen.h>
#include	<dsp_process2.h>
#include	<proc_sobg.h>
#include	<ml_std_mess.h>
#include	<ml_cm_mess.h>

#define	MANUAL	0
#define	BRANCH	1
#define	COMPANY	2

#define	FORWARD	0
#define	BACKORD	1

#define	PROC_FWD	(requisitionStatus == FORWARD)

#define	InternalMIN(X, Y)	(X < Y) ? X : Y ;

#define	FWD_LINE	(cmrd_rec.stat_flag [0] == 'F')
#define	PHANTOM		(incc_rec.sort [0] == 'P')

char *	ser_space = "                         ";

#include	"schema"

struct commRecord	comm_rec;
struct cmhrRecord	cmhr_rec;
struct cmrdRecord	cmrd_rec;
struct cmrhRecord	cmrh_rec;
struct inccRecord	incc_rec;
struct inmrRecord	inmr_rec;
struct soktRecord	sokt_rec;

	char	*data = "data";

	int		requisitionStatus	= 0, 
			serialItemRelease 	= FALSE, 
			envCmAutoReq		= 0, 
			envSoFwdRel			= 0, 
			checkAllBackordered = 0;

struct storeRec {
	long	reqNos;
} store [MAXLINES];

	float	closingStock 	= 0.00, 
			errorQuantity	= 0.00;

	char	reqBranchNo [3], 
			errorItem [17];

/*
 * Local & Screen Structures. 
 */
struct {
	char	dummy [11];
	char	systemDate [11];
	long	requisitionNo;
	char	contractNo [7];
	char	requisitionBy [21];
	long	requiredDate;
	long	hhrqHash;
} local_rec;

static	struct	var	vars [] =
{
	{1, TAB, "requisitionNo", MAXLINES, 4, LONGTYPE, 
		"NNNNNN", "          ", 
		"0", " ", "Requisition No", " ", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.requisitionNo}, 
	{1, TAB, "contractNo", 	 0, 4, CHARTYPE, 
		"AAAAAA", "          ", 
		" ", "", "Contract Number", " ", 
		 NA, NO, JUSTLEFT, "", "", local_rec.contractNo}, 
	{1, TAB, "requisitionBy", 	 0, 0, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Requested By    ", " ", 
		 NA, NO, JUSTLEFT, "", "", local_rec.requisitionBy}, 
	{1, TAB, "requiredDate", 	 0, 2, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", "", "Required Date", " ", 
		 NA, NO, JUSTLEFT, "", "", (char *)&local_rec.requiredDate}, 
	{1, TAB, "hhrqHash", 	 0, 0, LONGTYPE, 
		"NNNNNNNNNN", "          ", 
		" ", "", "", " ", 
		 ND, NO, JUSTLEFT, "", "", (char *)&local_rec.hhrqHash}, 

	{0, LIN, "", 	 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}, 
};

/*
 * Local function prototypes 
 */
float	ProcessPhantom		(long);
int		CheckBoLines		(long);
int		CheckSerial			(void);
int		DeleteLine			(void);
int		heading				(int);
int		ProcessCmrd			(long, int);
int		Process				(void);
int		SupplyOK			(long);
void	BackOrder			(void);
void	CloseDB				(void);
void	NormalRelease		(void);
void	OpenDB				(void);
void	shutdown_prog		(void);
void	SrchCmrh			(char *);
void	UpdateCmrh			(long, char *);

/*
 * Main Processing Routine.
 */
int
main (
	int		argc, 
	char	*argv [])
{
	int		i;
	char	*sptr;

	if (argc < 2)
	{
		print_at (0, 0, ML (mlCmMess738), argv [0]);
		return (EXIT_FAILURE);
	}

	/*
	 * Check level of req. no generation.
	 */
	sptr = chk_env ("CM_AUTO_REQ");
	envCmAutoReq = (sptr == (char *)0) ? COMPANY : atoi (sptr);

	/*
	 * Check forward release flag. 
	 */
	sptr = chk_env ("SO_FWD_REL");
	envSoFwdRel = (sptr == (char *)0) ? FALSE : atoi (sptr);

	/*
	 * Determine whether we are doing a Forward or Backorder release.
	 */
	switch (argv [1][0])
	{
	case 'F':
		requisitionStatus = FORWARD;
		break;

	case 'B':
	default:
		requisitionStatus = BACKORD;
		break;
	}

	SETUP_SCR (vars);
	tab_col = 5;

	strcpy (local_rec.systemDate, DateToString (TodaysDate ()));

	init_scr 	();
	set_tty 	();
	set_masks 	();
	init_vars 	(1);

	OpenDB ();
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	strcpy (reqBranchNo, (envCmAutoReq == COMPANY) ? " 0" : comm_rec.est_no);

	/*
	 * Initialise reqNos array. 
	 */
	for (i = 0; i < MAXLINES; i++)
		store [i].reqNos = 0L;

	/*
	 * Reset control flags 
	 */
	while (prog_exit == 0)
	{
		entry_exit 	= FALSE;
		prog_exit 	= FALSE;
		restart 	= FALSE;
		search_ok 	= TRUE;
		init_ok 	= TRUE;
		init_vars (1);
		lcount [1]	= 0;

		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
			continue;

		Process ();
		prog_exit = TRUE;
	}

	shutdown_prog ();
	return (EXIT_SUCCESS);
}

/*
 * Program exit sequence. 
 */
void
shutdown_prog (void)
{
	recalc_sobg ();
	CloseDB (); 
	FinishProgram ();
}

void
OpenDB (void)
{
	abc_dbopen (data);

	open_rec (cmhr, cmhr_list, CMHR_NO_FIELDS, "cmhr_hhhr_hash");
	open_rec (cmrd, cmrd_list, CMRD_NO_FIELDS, "cmrd_id_no");
	open_rec (cmrh, cmrh_list, CMRH_NO_FIELDS, "cmrh_id_no");

	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_hhbr_hash");
}

void
CloseDB (void)
{
	abc_fclose (cmhr);
	abc_fclose (cmrd);
	abc_fclose (cmrh);

	abc_fclose (incc);
	abc_fclose (inmr);
	abc_dbclose (data);
}

int
spec_valid (
	int		field)
{
	int		i			= 0,
			checkLines	= 0;

	if (LCHECK ("requisitionNo")) 
	{
		if (dflt_used)
			return (DeleteLine ());

		if (SRCH_KEY)
		{
			SrchCmrh (temp_str);
			return (EXIT_SUCCESS);
		}

		/*
		 * Lookup requisition 
		 */
		strcpy (cmrh_rec.co_no, comm_rec.co_no);
		strcpy (cmrh_rec.br_no, reqBranchNo);
		cmrh_rec.req_no = local_rec.requisitionNo;
		cc = find_rec (cmrh, &cmrh_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlCmMess015));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (PROC_FWD)
		{
			if (cmrh_rec.stat_flag [0] != 'F')
			{
				print_mess (ML (mlCmMess032));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}
		else
		{
			if (!CheckBoLines (cmrh_rec.hhrq_hash))
			{
				print_mess (ML (mlStdMess092));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}

		/*
		 * Lookup contract. 
		 */
		cmhr_rec.hhhr_hash	= cmrh_rec.hhhr_hash;
		cc = find_rec (cmhr, &cmhr_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess075));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
	
		/*
		 * Check table for requisitionNo.
		 */
		checkLines = (prog_status == ENTRY) ? line_cnt : lcount [1];
		for (i = 0; i < checkLines; i++)
		{
			if (prog_status != ENTRY && i == line_cnt)
				continue;

			if (store [i].reqNos == local_rec.requisitionNo)
			{
				print_mess (ML (mlCmMess034));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}

		sprintf (local_rec.contractNo, "%-6.6s", cmhr_rec.cont_no);
		sprintf (local_rec.requisitionBy, "%-20.20s", cmrh_rec.req_by);
		local_rec.requiredDate = cmrh_rec.rqrd_date;
		DSP_FLD ("contractNo");
		DSP_FLD ("requisitionBy");
		DSP_FLD ("requiredDate");

		store [line_cnt].reqNos = local_rec.requisitionNo;
		local_rec.hhrqHash = cmrh_rec.hhrq_hash;

		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*
 * Check that at least one line has a non-zero qty_bord field. 
 */
int
CheckBoLines (
	long	hhrqHash)
{
	cmrd_rec.hhrq_hash 	= hhrqHash;
	cmrd_rec.line_no 	= 0;
	cc = find_rec (cmrd, &cmrd_rec, GTEQ, "r");
	while (!cc && cmrd_rec.hhrq_hash == hhrqHash)
	{
		if (cmrd_rec.qty_border != 0.00)
			return (TRUE);

		cc = find_rec (cmrd, &cmrd_rec, NEXT, "u");
	}
	
	return (FALSE);
}

void
SrchCmrh (
	char	 *keyValue)
{
	char	requisitionNo [7];
	char	desc [41];

	_work_open (6, 0, 40);
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

		/*
		 * Search on Forward reqs only. 
		 */
		if (PROC_FWD && cmrh_rec.stat_flag [0] != 'F')
		{
			cc = find_rec (cmrh, &cmrh_rec, NEXT, "r");
			continue;
		}

		/*
		 * Search on reqs with BO qtys only. 
		 */
		if (!PROC_FWD && !CheckBoLines (cmrh_rec.hhrq_hash))
		{
			cc = find_rec (cmrh, &cmrh_rec, NEXT, "r");
			continue;
		}

		cmhr_rec.hhhr_hash = cmrh_rec.hhhr_hash;
		cc = find_rec (cmhr, &cmhr_rec, COMPARISON, "r");
		if (!cc)
		{
			sprintf (requisitionNo, "%06ld", cmrh_rec.req_no);
			sprintf (desc, 
				"%-6.6s | %-20.20s", 
				cmhr_rec.cont_no, 
				cmrh_rec.req_by);
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

int
DeleteLine (void)
{
	int	i;
	int	thisPage;

	if (prog_status == ENTRY)
	{
		print_mess (ML (mlStdMess005));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}

	lcount [1]--;

	thisPage = line_cnt / TABLINES;

	for (i = line_cnt;line_cnt < lcount [1];line_cnt++)
	{
		getval (line_cnt + 1);
		putval (line_cnt);

		store [line_cnt].reqNos = store [line_cnt + 1].reqNos;

		if (thisPage == line_cnt / TABLINES)
			line_display ();
	}

	putval (line_cnt);

	if (thisPage == line_cnt / TABLINES)
		blank_display ();
	
	line_cnt = i;
	getval (line_cnt);
	return (EXIT_SUCCESS);
}

int
Process (void)
{
	int	all_bo;

	abc_selfield (cmrh, "cmrh_hhrq_hash");

	sprintf (err_str, 
			"Forward Order Release - Order %06ld", 
			cmrh_rec.req_no);
	dsp_screen (err_str, comm_rec.co_no, comm_rec.co_name);

	/*
	 * Process all entered requisitions. 
	 */
	for (line_cnt = 0; line_cnt < lcount [1]; line_cnt++)
	{
		getval (line_cnt);
		cmrh_rec.hhrq_hash = local_rec.hhrqHash;
		cc = find_rec (cmrh, &cmrh_rec, COMPARISON, "r");
		if (cc)	
			continue;

		/*
		 * Check full supply etc. 
		 */
		if (!SupplyOK (local_rec.hhrqHash))
			continue;

		/*
		 * Process requisition lines. 
		 */
		checkAllBackordered = FALSE;
		ProcessCmrd (local_rec.hhrqHash, TRUE);

		/*
		 * If we have fully backordered at least 
		 * one line then check if ALL lines are  
		 * backordered.  If so then update header
		 * to status B.                          
		 */
		all_bo = FALSE;
		if (checkAllBackordered)
		{	
			all_bo = TRUE;
			cmrd_rec.hhrq_hash = local_rec.hhrqHash;
			cmrd_rec.line_no = 0;
			cc = find_rec (cmrd, &cmrd_rec, GTEQ, "r");
			while (!cc && 
			       cmrd_rec.hhrq_hash == local_rec.hhrqHash)
			{
				if (cmrd_rec.qty_order != 0.00)
				{
					all_bo = FALSE;
					break;
				}
				cc = find_rec (cmrd, &cmrd_rec, NEXT, "r");
			}
		}
		if (all_bo)
			UpdateCmrh (local_rec.hhrqHash, "B");
	}

	return (EXIT_SUCCESS);
}

/*
 * Check whether supply of current requisition is OK.  
 * If requisition is not FULL SUPPLY then return TRUE. 
 * If requisition IS FULL SUPPLY then check if ALL     
 * lines on the requisition can be fully supplied. If  
 * so then return TRUE otherwise ask if user wishes to 
 * confirm partial supply.                             
 */
int
SupplyOK (
	long	hhrqHash)
{
	int	i;

	/*
	 * Not full supply so OK to supply. 
	 */
	if (cmrh_rec.full_supply [0] != 'Y')
		return (TRUE);

	/*
	 * Check if we can supply all lines on the requisition in full.   
	 */
	strcpy (errorItem, "");
	errorQuantity = 0.00;
	if (!ProcessCmrd (hhrqHash, FALSE))
	{
		sprintf (err_str, 
			ML (mlCmMess084), 
			cmrh_rec.req_no);
		rv_pr (err_str, 22, 15, 1);

		sprintf (err_str, 
			ML (mlCmMess086), 
			errorQuantity, 
			clip (errorItem));
		i = prmptmsg (err_str, "YyNn", (78 - strlen (err_str)) / 2, 16);
		/*
		 * Clear messages. 
		 */
		print_at (15, 1, "%-70.70s", " ");
		print_at (16, 1, "%-70.70s", " ");
		if (i == 'N' || i == 'n')
			return (FALSE);
	}

	return (TRUE);
}

/*
 * Process cmrd lines for selective forward Order. 
 */
int
ProcessCmrd (
	long	hhrqHash, 
	int		doUpdate)
{
	float	tot_qty;
	char	req_str [7];

	cmrd_rec.hhrq_hash = hhrqHash;
	cmrd_rec.line_no = 0;
	cc = find_rec (cmrd, &cmrd_rec, GTEQ, "u");
	while (!cc && cmrd_rec.hhrq_hash == hhrqHash)
	{
		/*
		 * Find inmr record. 
		 */
		inmr_rec.hhbr_hash = cmrd_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
		if (cc)
			file_err (cc, inmr, "DBFIND");
			
		/*
		 * Display message on screen. 
		 */
		sprintf (err_str, " : Line %4d", cmrd_rec.line_no);
		sprintf (req_str, "%06ld", cmrh_rec.req_no);
		dsp_process (req_str, err_str);

		if (PHANTOM)
			closingStock = ProcessPhantom (cmrd_rec.hhbr_hash);
		else
		{
			incc_rec.hhcc_hash = cmrd_rec.hhcc_hash;
			incc_rec.hhbr_hash = alt_hash (inmr_rec.hhbr_hash, 
							 inmr_rec.hhsi_hash);
			cc = find_rec (incc, &incc_rec, COMPARISON, "r");
			if (cc)
			{
				abc_unlock (cmrd);
				cc = find_rec (cmrd, &cmrd_rec, NEXT, "u");
				continue;
			}
			if (PROC_FWD)
			{
				closingStock = incc_rec.closing_stock -
			 	 	 incc_rec.committed -
			 	 	 incc_rec.backorder;
			}
			else
			{
				closingStock = incc_rec.closing_stock -
			 	 	 incc_rec.committed;
				if (envSoFwdRel)
					closingStock -= incc_rec.forward;
			}
		}
	
		/*
		 * Check whether item is a serial and line has a serial number.  
		 */
		serialItemRelease = CheckSerial ();

		/*
		 * Release if closing stock > 0 or 'Z' or 'N' Class items. 
		 */
		tot_qty = cmrd_rec.qty_order + cmrd_rec.qty_border;
		if (closingStock > 0.00 || serialItemRelease)
		{
			if (doUpdate)
				NormalRelease ();
			else
			{
				/*
				 * Line can be fully supplied 
				 */
				if (closingStock < tot_qty)
				{
					errorQuantity = tot_qty;
					sprintf (errorItem, "%-.16s", inmr_rec.item_no);
					return (FALSE);
				}
			}
		}
		else
		{
			if (doUpdate)
				BackOrder ();
			else
			{
				/*
				 * Line cannot be fully supplied and we 
				 * are checking for SupplyOK ().      
				 */
				errorQuantity = tot_qty;
				sprintf (errorItem, "%-.16s", inmr_rec.item_no);
				return (FALSE);
			}
		}

		abc_unlock (cmrd);
		cc = find_rec (cmrd, &cmrd_rec, NEXT, "u");
	}
	abc_unlock (cmrd);
	return (TRUE);
}

/*
 * Release forward order. 
 */
void
NormalRelease (void)
{
	float	qtyOrder 	= 0.0,
			qtySupplied = 0.0,
			qtyLeft 	= 0.0;

	qtyOrder = cmrd_rec.qty_order + cmrd_rec.qty_border;
	qtySupplied  = cmrd_rec.qty_order + cmrd_rec.qty_border;

	if (serialItemRelease)
		closingStock = cmrd_rec.qty_order + cmrd_rec.qty_border;

	/*
	 * closing stock need to be held & reduced as each valid 
	 * cmrd record is processed.
	 */
	if (closingStock > 0.00 || serialItemRelease)
	{
		qtyLeft = closingStock;
		qtySupplied = qtyOrder;

		if (closingStock <= qtyOrder)
			qtySupplied = InternalMIN (qtyLeft, closingStock);

		if (qtyOrder < qtySupplied)
			qtySupplied = InternalMIN (qtyLeft, qtyOrder);

		cmrd_rec.qty_order = qtySupplied;
		cmrd_rec.qty_border  = qtyOrder - qtySupplied;
	
		closingStock -= qtySupplied;

		strcpy (cmrd_rec.stat_flag, "R");
		cc = abc_update (cmrd, &cmrd_rec);
		if (cc)
			file_err (cc, cmrd, "DBUPDATE");

		/*
		 * Add sobg record for recalc of stock. 
		 */
		add_hash 
		(
			comm_rec.co_no, 
			comm_rec.est_no, 
			"RC", 
			0, 
		  	cmrd_rec.hhbr_hash, 
			cmrd_rec.hhcc_hash, 
			0L, 
			(double) 0.00
		);

		UpdateCmrh (cmrd_rec.hhrq_hash, "R");
	}
	else
		BackOrder ();
}
/*
 * Not enough stock so release as a Backorder. 
 */
void
BackOrder (void)
{
	cmrd_rec.qty_border += cmrd_rec.qty_order;
	cmrd_rec.qty_order = 0.00;

	strcpy (cmrd_rec.stat_flag, "B");
	cc = abc_update (cmrd, &cmrd_rec);
	if (cc)
		file_err (cc, cmrd, "DBUPDATE");
		
	/*
	 * Add record for later updating of record. 
	 */
	add_hash (comm_rec.co_no, comm_rec.est_no, "RC", 0, 
	  	  cmrd_rec.hhbr_hash, cmrd_rec.hhcc_hash, 0L, 
		(double) 0.00);

	checkAllBackordered = TRUE;
}
/*
 * Update Header for Lines Processed. 
 */
void
UpdateCmrh (
	long	hhrqHash, 
	char 	*stat_flag)
{
	cmrh_rec.hhrq_hash	=	hhrqHash;
	cc = find_rec (cmrh, &cmrh_rec, COMPARISON, "u");
	if (!cc)	
	{
		strcpy (cmrh_rec.stat_flag, stat_flag);
		cc = abc_update (cmrh, &cmrh_rec);
		if (cc)
			file_err (cc, cmrh, "DBUPDATE");
	}
}

/*
 * Check if item is a serial item and a serial number exists. 
 */
int
CheckSerial (void)
{
	if (inmr_rec.serial_item [0] == 'Y' && 
	    strcmp (cmrd_rec.serial_no, ser_space))
	{
		return (TRUE);
	}
	
	return (FALSE);
}

/*
 * Specific code to handle single level Bills. 
 */
float
ProcessPhantom (
	long	hhbrHash)
{
	float	min_qty = 0.00, 
			onHand = 0.00;

	int	first_time = TRUE;

	open_rec (sokt, sokt_list, SOKT_NO_FIELDS, "sokt_hhbr_hash");

	sokt_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec (sokt, &sokt_rec, GTEQ, "r");
	while (!cc && sokt_rec.hhbr_hash == hhbrHash)
	{
		incc_rec.hhcc_hash = cmrd_rec.hhcc_hash;
		incc_rec.hhbr_hash = sokt_rec.mabr_hash;
		cc = find_rec (incc, &incc_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (sokt, &sokt_rec, NEXT, "r");
			continue;
		}
	
		onHand 	= incc_rec.closing_stock -
				  incc_rec.committed -
				  incc_rec.backorder;
		
		onHand /= sokt_rec.matl_qty;
		if (first_time)
			min_qty = onHand;

		if (min_qty > onHand)
			min_qty = onHand;

		first_time = FALSE;

		cc = find_rec (sokt, &sokt_rec, NEXT, "r");
	}
	abc_fclose (sokt);

	return (min_qty);
}

int
heading (
 int scn)
{
	if (restart) 
		return (EXIT_SUCCESS);

	clear ();

	if (PROC_FWD) 
		rv_pr (ML (mlStdMess093), (80 - strlen (ML (mlStdMess093))) / 2, 0, 1);
	else
		rv_pr (ML (mlStdMess094), (80 - strlen (ML (mlStdMess094))) / 2, 0, 1);

	line_at (1, 0, 80);

	box (5, 5, 67, 12);

	line_at (20, 0, 80);

	print_at (21, 0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
	print_at (22, 0, ML (mlStdMess039), comm_rec.est_no, comm_rec.est_name);

	/*  reset this variable for new screen NOT page	*/
	line_cnt = 0;
	scn_write (scn);
	return (EXIT_SUCCESS);
}

/*=====================================================================
|  Copyright (C) 1998 - 2001 LogisticSoftware                         |
|=====================================================================|
| $Id: so_usr_rel.c,v 5.12 2002/07/24 08:39:29 scott Exp $
|  Program Name  : (so_usr_rel.c)
|  Program Desc  : (User Release For Selected Items)
|---------------------------------------------------------------------|
|  Date Written  : (20/10/88)      | Author       : Scott Darrow.     |
|---------------------------------------------------------------------|
| $Log: so_usr_rel.c,v $
| Revision 5.12  2002/07/24 08:39:29  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.11  2002/07/18 07:18:26  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.10  2002/07/08 10:35:33  kaarlo
| S/C 4022. Added delay to Item Number.
|
| Revision 5.9  2002/06/26 06:04:10  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.8  2002/04/30 07:56:50  scott
| Update for new Archive modifications;
|
| Revision 5.7  2002/04/16 02:40:49  robert
| SC00916 - Updated to fixed memory dump occurence FreeCcList
|
| Revision 5.6  2002/04/10 10:47:35  robert
| SC00864/SC00914 - Update for display and default value problem on LS10-GUI
|
| Revision 5.5  2002/03/04 08:47:27  scott
| S/C 00824 - SKTR12-Selective B/O Release; the default value for all in field Transfer No is '00ALL'.
|
| Revision 5.4  2001/11/20 03:45:42  scott
| Updated to convert to app.schema and perform a general code clean.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: so_usr_rel.c,v $", 
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SO/so_usr_rel/so_usr_rel.c,v 5.12 2002/07/24 08:39:29 scott Exp $";

#define MAXSCN 		2
#define MAXLINES	1000
#define TABLINES	8
#define MAXITEMS	1000
#define	SR		store [line_cnt]

#include	<pslscr.h>
#include	<twodec.h>
#include	<proc_sobg.h>
#include	<ml_std_mess.h>
#include	<ml_so_mess.h>

#define	COMMENT_LINE	 	(incc_rec.sort [0] == 'Z' && envVarSoCommHold)
#define	NON_STOCK	 		(incc_rec.sort [0] == 'N')
#define	PHANTOM		 		(inmr_rec.inmr_class [0] == 'P')

#define	SOLN_BACKORDER	 	(soln_rec.status [0] == 'B')

#define	ITLN_BACKORDER	 	(itln_rec.status [0] == 'B' || \
			 			 	(itln_rec.status [0] == 'T' && \
			    		  	 itln_rec.qty_border != 0.00))

#define	VALID_ITLN	 		(local_rec.ord_sel [0] == 'B' || \
			  				 local_rec.ord_sel [0] == itln_rec.stock [0])

struct	RCV_CC_LIST
{
	long	hhcc_hash;
	long	hhit_hash;
	int	line_no;
	struct	RCV_CC_LIST	*_next;
};

#define	RCV_CC_NULL	 ((struct RCV_CC_LIST *) NULL)

struct	RCV_CC_LIST	*rcv_cc_head = RCV_CC_NULL;
struct	RCV_CC_LIST	*rcv_cc_curr = RCV_CC_NULL;

	int		envVarConOrders 	= 0, 
			envVarSoCommHold 	= 0,
			envVarSoFwdRel		= 0,
			transEnable 		= FALSE;

	float	origionalAvailable = 0.00;

	char	*ithr2	=	"ithr2";

#include	"schema"

struct commRecord	comm_rec;
struct sohrRecord	sohr_rec;
struct solnRecord	soln_rec;
struct colnRecord	coln_rec;
struct inmrRecord	inmr_rec;
struct cumrRecord	cumr_rec;
struct ccmrRecord	ccmr_rec;
struct ccmrRecord	ccmr2_rec;
struct inccRecord	incc_rec;
struct ithrRecord	ithr_rec;
struct itlnRecord	itln_rec;
struct soktRecord	sokt_rec;

	int	pid;

	char	*currentUser;

	struct	storeRec {
		float	_release;
	} store [MAXLINES];

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	order_no [9];
	char	acro [10];
	char	ord_no [9];
	char	ord_sel [2];
	long	ord_date;
	long	reqd_date;
	float	order_bal;
	float	release;
	float	backorder;
	char	rel_flg [2];
	long	hash;
	float	qty_bord;
	char	bo_flag [2];
	char	fs_flag [2];
	long	hhum_hash;
} local_rec;

	char	ord_trn_1 [16];
	char	ord_trn_2 [12];
	char	ord_trn_3 [12];

static	struct	var	vars [] =
{
	{1, LIN, "item_no", 	 4, 22, CHARTYPE, 
		"UUUUUUUUUUUUUUUU", "          ", 
		" ", "", "Item Number          ", " ", 
		 NE, NO,  JUSTLEFT, "", "", inmr_rec.item_no}, 
	{1, LIN, "desc", 	 5, 22, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "Description          ", " ", 
		 NA, NO,  JUSTLEFT, "", "", inmr_rec.description}, 
	{1, LIN, "ord_no", 	 7, 22, CHARTYPE, 
		"AAAAAAAA", "          ", 
		"", "ALL", ord_trn_1, "Default is ALL ", 
		YES, NO, JUSTLEFT, "", "", local_rec.order_no}, 
	{1, LIN, "ord_sel", 	 8, 22, CHARTYPE, 
		"U", "          ", 
		" ", "B", "Customer/Stock/Both ", "Enter C (ustomer trans) S (tock trans) B)oth) <Default = Both>", 
		 ND, NO,  JUSTLEFT, "", "", local_rec.ord_sel}, 
	{2, TAB, "acro", 	MAXLINES, 0, CHARTYPE, 
		"UUUUUUUUU", "          ", 
		" ", " ", ord_trn_3, " ", 
		 NA, NO,  JUSTLEFT, "", "", local_rec.acro}, 
	{2, TAB, "order", 	 0, 0, CHARTYPE, 
		"AAAAAAAA", "          ", 
		" ", " ", ord_trn_2, " ", 
		 NA, NO,  JUSTLEFT, "", "", local_rec.ord_no}, 
	{2, TAB, "ord_date", 	 0, 0, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", " ", "Ord Date", " ", 
		 NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.ord_date}, 
	{2, TAB, "reqd_date", 	 0, 0, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", " ", "Req Date", " ", 
		 NA, NO, JUSTRIGHT, "", "", (char *)&local_rec.reqd_date}, 
	{2, TAB, "ord_bal", 	 0, 1, FLOATTYPE, 
		"NNNNNN.NN", "          ", 
		" ", "0", " Order Bal", " ", 
		 NA, NO, JUSTRIGHT, "0", "999999.99", (char *)&local_rec.order_bal}, 
	{2, TAB, "rel_flg", 	 0, 4, CHARTYPE, 
		"U", "          ", 
		" ", "", " Release. ", " ", 
		YES, NO,  JUSTLEFT, "YN", "", local_rec.rel_flg}, 
	{2, TAB, "release", 	 0, 0, FLOATTYPE, 
		"NNNNNN.NN", "          ", 
		" ", "0", " Release. ", " ", 
		YES, NO, JUSTRIGHT, "0", "999999.99", (char *)&local_rec.release}, 
	{2, TAB, "backorder", 	 0, 1, FLOATTYPE, 
		"NNNNNN.NN", "          ", 
		" ", "0", " Backorder.", " ", 
		YES, NO, JUSTRIGHT, "0", "999999.99", (char *)&local_rec.backorder}, 
	{2, TAB, "ord_desp", 	 0, 0, FLOATTYPE, 
		"NNNNN.NN", "          ", 
		" ", "0", "qty_bord", " ", 
		 ND, NO, JUSTRIGHT, "", "", (char *)&local_rec.qty_bord}, 
	{2, TAB, "hash", 	 0, 0, LONGTYPE, 
		"NNNNNNNNNN", "          ", 
		" ", " ", "hash", " ", 
		 ND, NO, JUSTRIGHT, "", "", (char *)&local_rec.hash}, 
	{2, TAB, "bo_flag", 	 0, 0, CHARTYPE, 
		"U", "          ", 
		" ", " ", "cumr_bo_flag", " ", 
		 ND, NO, JUSTRIGHT, "", "", local_rec.bo_flag}, 
	{2, TAB, "fs_flag", 	 0, 0, CHARTYPE, 
		"U", "          ", 
		" ", " ", "itln_full_supply", " ", 
		 ND, NO, JUSTRIGHT, "", "", local_rec.fs_flag}, 
	{2, TAB, "hhum_hash", 	 0, 0, LONGTYPE, 
		"NNNNNN", "          ", 
		" ", " ", "itln_hhum_hash", " ", 
		 ND, NO, JUSTRIGHT, "", "", (char *)&local_rec.hhum_hash}, 
	{0, LIN, "", 	 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}, 
};

int		envVarQcApply = FALSE, 
		envVarSkQcAvl = FALSE;

#include <RealCommit.h>

/*======================= 
| Function Declarations |
=======================*/
struct 	RCV_CC_LIST *FindIthr 	(void);
float 	CalcAvailable 			(void);
float 	ProcessPhantom 			(long);
int  	CheckAvailable 			(float);
int  	CheckLines 				(long);
int  	CheckSohr 				(long);
int  	GetData 				(void);
int  	heading 				(int);
int  	LoadAllInfo 			(void);
int  	spec_valid 				(int);
long 	CreateIthr 				(void);
void 	BackorderSoln 			(void);
void 	CloseDB 				(void);
void 	DeleteRecords 			(void);
void 	DeleteColn 				(long);
void 	FreeCcList 				(void);
void 	ItlnUpdate 				(void);
void 	LoadOrders 				(void);
void 	LoadTransfers 			(void);
void 	OpenDB 					(void);
void 	PrintRelease 			(void);
void 	ReadMisc 				(void);
void 	shutdown_prog 			(void);
void 	SolnUpdate 				(void);
void 	SrchIthr 				(char *);
void 	SrchSohr 				(char *);
void 	UpdateIthr 				(long);
void 	UpdateSolnRelease 		(long);
void 	UpdateSoln 				(void);

/*
 * Main Processing Routine.
 */
int
main (
 int argc, 
 char * argv [])
{
	int	i;
	char	*sptr;

	SETUP_SCR (vars);


	pid = getpid ();

	sptr = strrchr (argv [0], '/');
	if (sptr)
		sptr++;
	else
		sptr = argv [0];

	currentUser = getenv ("LOGNAME");

	if (strcmp (sptr, "so_usr_trel"))
	{
		strcpy (ord_trn_1, "Order Number   ");
		strcpy (ord_trn_2, "Order No");
		strcpy (ord_trn_3, " Acronym   ");
	}
	else
	{
		transEnable = TRUE;
		strcpy (ord_trn_1, "Transfer No    ");
		strcpy (ord_trn_2, "Transfer");
		strcpy (ord_trn_3, "Cust/Stock.");
		FLD ("ord_sel") = YES;
	}

	/*
	 * check forward release flag	
	 */
	envVarSoFwdRel = (sptr = chk_env ("SO_FWD_REL")) ? atoi (sptr) : FALSE;

	/*
	 * check forward for order consolidation
	 */
	envVarConOrders = (sptr = chk_env ("CON_ORDERS")) ? atoi (sptr) : 0;

	/*
	 * Check if Comment lines are held.
	 */
	envVarSoCommHold = (sptr = chk_env ("SO_COMM_HOLD")) ? atoi (sptr) : 0;

	/* 
	 * QC module is active or not. 
	 */
	envVarQcApply = (sptr = chk_env ("QC_APPLY")) ? atoi (sptr) : 0;

	/* 
	 * Whether to include QC qty in available stock. 
	 */
	envVarSkQcAvl = (sptr = chk_env ("SK_QC_AVL")) ? atoi (sptr) : 0;

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

	OpenDB ();

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
		eoi_ok 		= FALSE;
		init_vars (1);
		init_vars (2);
		lcount [2] = 0;

		for (i = 0; i < MAXLINES; i++)
			store [i]._release = 0.00;

		heading (1);
		entry (1);

		if (prog_exit || restart)
			continue;

		LoadAllInfo ();
		if (lcount [2] == 0)
		{
			if (transEnable)
				sprintf (err_str, ML (mlSoMess191), inmr_rec.item_no);
			else
				sprintf (err_str, ML (mlSoMess192), inmr_rec.item_no);
			print_mess (err_str);
			sleep (sleepTime);
			fflush (stdout);
			continue;
		}

		init_ok = FALSE;
		eoi_ok 	= FALSE;
		heading (2);
		scn_display (2);
		entry (2);
		eoi_ok = TRUE;
		if (restart)
			continue;

		prog_status = ! (ENTRY);

		edit_all ();
		if (restart)
			continue;

		if (!transEnable)
			SolnUpdate ();
		else
		{
			ItlnUpdate ();
			FreeCcList ();
		}
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
	abc_dbopen ("data");

	ReadMisc ();

	if (transEnable)
	{
		abc_alias (ithr2, ithr);
		open_rec (ithr, ithr_list, ITHR_NO_FIELDS, "ithr_id_no");
		open_rec (ithr2, ithr_list, ITHR_NO_FIELDS, "ithr_hhit_hash");
		open_rec (itln, itln_list, ITLN_NO_FIELDS, "itln_id_no");
	}
	else
	{
		open_rec (sohr, sohr_list, SOHR_NO_FIELDS, "sohr_hhso_hash");
		open_rec (soln, soln_list, SOLN_NO_FIELDS, "soln_hhbr_hash");
		open_rec (coln, coln_list, COLN_NO_FIELDS, "coln_hhsl_hash");
	}
	open_rec (cumr, cumr_list, CUMR_NO_FIELDS, "cumr_hhcu_hash");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (soic, soic_list, soic_no_fields, "soic_id_no2");
}

void
CloseDB (void)
{
	abc_fclose (ccmr);
	if (transEnable)
	{
		abc_fclose (ithr);
		abc_fclose (ithr2);
		abc_fclose (itln);
	}
	else
	{
		abc_fclose (sohr);
		abc_fclose (soln);
		abc_fclose (coln);
	}
	abc_fclose (ccmr);
	abc_fclose (cumr);
	abc_fclose (inmr);
	abc_fclose (incc);
	abc_fclose (soic);
	SearchFindClose ();
	abc_dbclose ("data");
}

/*============================================ 
| Get common info from commom database file. |
============================================*/
void
ReadMisc (void)
{
	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");

	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, comm_rec.est_no);
	strcpy (ccmr_rec.cc_no, comm_rec.cc_no);

	cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
	if (cc)
 	        file_err (cc, ccmr, "DBFIND");

	abc_selfield (ccmr, "ccmr_hhcc_hash");
}

int
spec_valid (
 int field)
{
	float	realCommitted;

	/*-----------------------
	| Validate Item Number. |
	-----------------------*/ 
	if (LCHECK ("item_no"))
	{
		origionalAvailable = 0.00;

		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}
		clear_mess ();

		cc = FindInmr (comm_rec.co_no, inmr_rec.item_no, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, inmr_rec.item_no);
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			errmess (ML (mlStdMess001));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		
		SuperSynonymError ();

		DSP_FLD ("item_no");
		DSP_FLD ("desc");

		/*------------------------------------------------------------
		| Items does not allow backorders so remove backorder field. |
		------------------------------------------------------------*/
		FLD ("backorder") = YES;
		FLD ("release") = YES;
		FLD ("rel_flg") = NO;
		if (inmr_rec.bo_flag [0] == 'F')
		{
			FLD ("release") = ND;
			FLD ("backorder") = ND;
		}
		else
		{
			FLD ("rel_flg") = ND;
			if (inmr_rec.bo_flag [0] == 'N')
				FLD ("backorder") = ND;
		}

		if (PHANTOM)
			origionalAvailable = ProcessPhantom (inmr_rec.hhbr_hash);
		else
		{
			incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
			incc_rec.hhbr_hash = alt_hash (inmr_rec.hhbr_hash, 
							 inmr_rec.hhsi_hash);
			cc = find_rec (incc, &incc_rec, COMPARISON, "r");
			if (cc)
				origionalAvailable = 0.00;
			else
			{
				/*---------------------------------
				| Calculate Actual Qty Committed. |
				---------------------------------*/
				realCommitted = RealTimeCommitted (incc_rec.hhbr_hash, 
													incc_rec.hhcc_hash);
				if (envVarSoFwdRel)
				{
					origionalAvailable = incc_rec.closing_stock -
					     	     incc_rec.committed -
								 realCommitted -
					     	     incc_rec.forward;
				}
				else
				{
					origionalAvailable = incc_rec.closing_stock -
					     	     incc_rec.committed -
								 realCommitted;
				}
				if (envVarQcApply && envVarSkQcAvl)
					origionalAvailable -= incc_rec.qc_qty;
			}
		}
		return (EXIT_SUCCESS);
	}

	/*------------------------------
	| Validate order number input. |
	------------------------------*/
	if (LCHECK ("ord_no") && transEnable == FALSE)
	{
		if (SRCH_KEY)
		{
			SrchSohr (temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used || !strncmp (local_rec.order_no, "ALL",3))
		{
			abc_selfield (sohr, "sohr_hhso_hash");
			strcpy (local_rec.order_no, "ALL     ");
			DSP_FLD ("ord_no");
			return (EXIT_SUCCESS);
		}

		abc_selfield (sohr, "sohr_id_no2");
		strcpy (sohr_rec.co_no, comm_rec.co_no);
		strcpy (sohr_rec.br_no, comm_rec.est_no);
		strcpy (sohr_rec.order_no, zero_pad (local_rec.order_no, 8));
		cc = find_rec (sohr, &sohr_rec, COMPARISON, "w");
		if (cc)
		{
			print_mess (ML (mlStdMess122));
			sleep (sleepTime);
			fflush (stdout);
			return (EXIT_FAILURE);
		}
		abc_selfield (sohr, "sohr_hhso_hash");

		LoadAllInfo ();
		if (lcount [2] == 0)
		{
			sprintf (err_str, ML (mlSoMess192), inmr_rec.item_no);
			print_mess (err_str);
			sleep (sleepTime);
			fflush (stdout);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	/*------------------------------
	| Validate Trans number input. |
	------------------------------*/
	if (LCHECK ("ord_no") && transEnable)
	{
		abc_selfield (itln, "itln_id_no");

		if (SRCH_KEY)
		{
			SrchIthr (temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used || !strncmp (local_rec.order_no, "ALL",3))
		{
			strcpy (local_rec.order_no, "ALL   ");
			strcpy (local_rec.ord_sel, "B");
			DSP_FLD ("ord_no");
			DSP_FLD ("ord_sel");
			FLD ("ord_sel") = YES;
			return (EXIT_SUCCESS);
		}

		FLD ("ord_sel") = NA;
		strcpy (local_rec.ord_sel, "B");
		strcpy (ithr_rec.co_no, comm_rec.co_no);
		ithr_rec.del_no = atol (local_rec.order_no);
		strcpy (ithr_rec.type, "B");
		cc = find_rec (ithr, &ithr_rec, COMPARISON, "u");
		if (cc)
		{
			abc_unlock (ithr);
			strcpy (ithr_rec.type, "T");
			cc = find_rec (ithr, &ithr_rec, COMPARISON, "u");
		}
		if (cc)
		{
			abc_unlock (ithr);
			print_mess (ML (mlSoMess309));
			sleep (sleepTime);
			fflush (stdout);
			return (EXIT_FAILURE);
		}

		LoadAllInfo ();
		if (lcount [2] == 0)
		{
			sprintf (err_str, ML (mlSoMess191), inmr_rec.item_no);
			print_mess (err_str);
			sleep (sleepTime);
			fflush (stdout);
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	/*--------------------------------
	| Validate Acronym number input. |
	--------------------------------*/
	if (LCHECK ("acro"))
	{
		if (prog_status == ENTRY)
			getval (line_cnt);

		return (EXIT_SUCCESS);
	}

	/*-----------------------------
	| Validate Quantity Released. |
	-----------------------------*/
	if (LCHECK ("release"))
	{
		if (dflt_used)
		{
			local_rec.release   = local_rec.order_bal;
			local_rec.backorder = 0.00;

			DSP_FLD ("release");
			DSP_FLD ("backorder");

			if (CheckAvailable (local_rec.release))
				return (EXIT_FAILURE);
			
			SR._release = local_rec.release;
			PrintRelease ();
			return (EXIT_SUCCESS);
		}

		if ((local_rec.release > local_rec.order_bal) && 
			local_rec.release > 0.00)
		{
			if (transEnable)
				sprintf (err_str, ML (mlSoMess189), local_rec.release);
			else
				sprintf (err_str, ML (mlSoMess190), local_rec.release);
			print_mess (err_str);
			fflush (stdout);
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		if (local_rec.fs_flag [0] == 'Y' && transEnable)
		{
			if (local_rec.release != 0.00 && local_rec.release != local_rec.order_bal)
			{
				print_mess (ML (mlSoMess198));
				fflush (stdout);
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
		}

		if (local_rec.fs_flag [0] == 'L' && transEnable)
		{
			if (local_rec.release != 0.00 && local_rec.release != local_rec.order_bal)
			{
				print_mess (ML (mlSoMess199));
				fflush (stdout);
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
		}

		local_rec.backorder = local_rec.order_bal - local_rec.release;
		DSP_FLD ("backorder");

		if (CheckAvailable (local_rec.release))
			return (EXIT_FAILURE);

		SR._release = local_rec.release;
		PrintRelease ();

		return (EXIT_SUCCESS);
	}

	/*-------------------------------
	| Validate Released Flag	|
	-------------------------------*/
	if (LCHECK ("rel_flg") && FLD ("rel_flg") != ND)
	{
		if (dflt_used || local_rec.rel_flg [0] == 'Y')
		{
			strcpy (local_rec.rel_flg, "Y");
			DSP_FLD ("rel_flg");
			local_rec.release   = local_rec.order_bal;
			local_rec.backorder = 0.00;

			if (CheckAvailable (local_rec.release))
				return (EXIT_FAILURE);

			SR._release = local_rec.release;
			PrintRelease ();
			return (EXIT_SUCCESS);
		}

		if (local_rec.rel_flg [0] == 'N')
		{
			DSP_FLD ("rel_flg");
			local_rec.release   = 0.00;
			local_rec.backorder = local_rec.order_bal;

			SR._release = local_rec.release;
			PrintRelease ();
			return (EXIT_SUCCESS);
		}

		return (EXIT_SUCCESS);
	}

	/*-------------------------
	| Validate Backorder Qty. |
	-------------------------*/
	if (LCHECK ("backorder"))
	{
		/*----------------------------------
		| Items does not allow backorders. |
		----------------------------------*/
		if (FLD ("backorder") == ND)
		{
			local_rec.backorder = 0.00;
			return (EXIT_SUCCESS);
		}

		/*-------------------------------------
		| Customer does not allow backorders. |
		-------------------------------------*/
	   	if (local_rec.bo_flag [0] == 'N' &&
		    local_rec.backorder != 0.00 &&
		    !transEnable)
		{
			sprintf (err_str, ML (mlSoMess193), local_rec.acro);
			print_mess (err_str);
			sleep (sleepTime);
			local_rec.backorder = 0.00;
			DSP_FLD ("backorder");
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			local_rec.backorder = local_rec.order_bal - 
					      local_rec.release;

			DSP_FLD ("backorder");

			return (EXIT_SUCCESS);
		}

		if (local_rec.backorder > local_rec.order_bal)
		{
			if (transEnable)
				sprintf (err_str, ML (mlSoMess194), local_rec.backorder);
			else
				sprintf (err_str, ML (mlSoMess195), local_rec.backorder);
			print_mess (err_str);
			fflush (stdout);
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		local_rec.release = local_rec.order_bal - local_rec.backorder;
		DSP_FLD ("release");

		SR._release = local_rec.release;
		PrintRelease ();

		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*========================
| Check available stock. |
========================*/
int
CheckAvailable (
	float	release)
{
	int	i;

	if (release == 0.00)
		return (EXIT_SUCCESS);

	if (release > CalcAvailable ()) 
	{
		print_mess (ML (mlSoMess196));
		i = prmptmsg (ML (mlSoMess310), "YyNn", 1, 4);
		move (1, 4);
		cl_line ();
		if (i != 'Y' && i != 'y')
			return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}

/*============================
| Calculate available stock. |
============================*/
float	
CalcAvailable (void)
{
	float	curr_avail = 0.00;
	int	i;

	curr_avail = origionalAvailable;
	
	for (i = 0; i <= lcount [2]; i++)
		curr_avail -= store [i]._release;
	
	return (curr_avail);
}

void
PrintRelease (
 void)
{
	print_at (2, 0, ML (mlSoMess197), clip (inmr_rec.item_no), CalcAvailable ());
	fflush (stdout);
}

/*=====================
| Search for Order no |
=====================*/
void
SrchSohr (
	char *key_val)
{
	_work_open (8,0,40);
	save_rec ("#Ord No", "#Date ");                       
	abc_selfield (sohr, "sohr_id_no2");
	strcpy (sohr_rec.co_no, comm_rec.co_no);
	strcpy (sohr_rec.br_no, comm_rec.est_no);
	sprintf (sohr_rec.order_no, "%-8.8s", key_val);
	cc = find_rec (sohr, &sohr_rec, GTEQ, "r");
	while (!cc && !strncmp (sohr_rec.order_no, key_val, strlen (key_val)) &&
				  !strcmp (sohr_rec.co_no, comm_rec.co_no) &&
				  !strcmp (sohr_rec.br_no, comm_rec.est_no))
	{ 
		if (sohr_rec.status [0] != 'P' && sohr_rec.status [0] != 'S')
		{
			strcpy (err_str, DateToString (sohr_rec.dt_raised));      
			cc = save_rec (sohr_rec.order_no, err_str);
			if (cc)
				break;
		}
		cc = find_rec (sohr, &sohr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (sohr_rec.co_no, comm_rec.co_no);
	strcpy (sohr_rec.br_no, comm_rec.est_no);
	sprintf (sohr_rec.order_no, "%-8.8s", temp_str);
	cc = find_rec (sohr, &sohr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, sohr, "DBFIND");
}

/*======================
| Search for Docket no |
======================*/
void
SrchIthr (
	char *key_val)
{
	_work_open (10,6,40);
	save_rec ("#Docket", "#Date ");                       
	strcpy (ithr_rec.co_no, comm_rec.co_no);
	strcpy (ithr_rec.type, "B");
	if (atol (key_val) != 0L)
	{
		ithr_rec.del_no = atol (key_val);
		cc = find_rec (ithr, &ithr_rec, COMPARISON, "r");
		if (!cc)
		{
			if (CheckLines (ithr_rec.hhit_hash))
			{
				sprintf (err_str, "%06ld [B]", ithr_rec.del_no);
				save_rec (err_str, ithr_rec.tran_ref);
			}
		}
	}
	else
	{
		ithr_rec.del_no = 0L;
		cc = find_rec (ithr, &ithr_rec, GTEQ, "r");
        	while (!cc &&
		       !strcmp (ithr_rec.co_no, comm_rec.co_no) &&
		       !strcmp (ithr_rec.type, "B"))
    		{ 
			if (CheckLines (ithr_rec.hhit_hash))
			{
				sprintf (err_str, "%06ld [B]", ithr_rec.del_no);
				cc = save_rec (err_str, ithr_rec.tran_ref);
				if (cc)
					break;
			}
			cc = find_rec (ithr, &ithr_rec, NEXT, "r");
		}
	}
	strcpy (ithr_rec.co_no, comm_rec.co_no);
	strcpy (ithr_rec.type, "T");
	if (atol (key_val) != 0L)
	{
		ithr_rec.del_no = atol (key_val);
		cc = find_rec (ithr, &ithr_rec, COMPARISON, "r");
		if (!cc)
		{
			if (CheckLines (ithr_rec.hhit_hash))
			{
				sprintf (err_str, "%06ld [T]", ithr_rec.del_no);
				save_rec (err_str, ithr_rec.tran_ref);
			}
		}
	}
	else
	{
		ithr_rec.del_no = 0L;
		cc = find_rec (ithr, &ithr_rec, GTEQ, "r");
        	while (!cc &&
		       !strcmp (ithr_rec.co_no, comm_rec.co_no) &&
		       !strcmp (ithr_rec.type, "T"))
    		{ 
			if (CheckLines (ithr_rec.hhit_hash))
			{
				sprintf (err_str, "%06ld [T]", ithr_rec.del_no);
				cc = save_rec (err_str, ithr_rec.tran_ref);
				if (cc)
					break;
			}
			cc = find_rec (ithr, &ithr_rec, NEXT, "r");
		}
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (ithr_rec.co_no, comm_rec.co_no);
	ithr_rec.del_no = atol (temp_str);
	ithr_rec.type [0] = temp_str [8];
	cc = find_rec (ithr, &ithr_rec, COMPARISON, "u");
	if (cc)
		file_err (cc, ithr, "DBFIND");

	sprintf (temp_str, "%06ld", ithr_rec.del_no);
}

int
CheckLines (
	long	hhitHash)
{
	itln_rec.hhit_hash 	= hhitHash;
	itln_rec.line_no 	= 0;
	cc = find_rec (itln, &itln_rec, GTEQ, "r");
	while (!cc && itln_rec.hhit_hash == hhitHash)
	{
		if (itln_rec.i_hhcc_hash == ccmr_rec.hhcc_hash &&
		     itln_rec.hhbr_hash == inmr_rec.hhbr_hash &&
		   (ITLN_BACKORDER))
			return (EXIT_FAILURE);

		cc = find_rec (itln, &itln_rec, NEXT, "r");
	}
	return (EXIT_SUCCESS);
}

int
LoadAllInfo (void)
{
	scn_set (2);
	lcount [2] = 0;

	if (transEnable)
		LoadTransfers ();
	else
		LoadOrders ();

	/*
	 * set max no. of tabular entries. 
	 */
	vars [scn_start].row = lcount [2];

	scn_set (1);
	if (lcount [2] == 0)
		return (EXIT_FAILURE);

	return (EXIT_SUCCESS);
}

void
LoadOrders (void)
{
	/*
	 * All orders selected.
	 */
	if (!strncmp (local_rec.order_no, "ALL", 3))
	{
		abc_selfield (soln, "soln_hhbr_hash");
		abc_selfield (sohr, "sohr_hhso_hash");

		soln_rec.hhbr_hash	=	inmr_rec.hhbr_hash;
		cc = find_rec (soln, &soln_rec, GTEQ, "r");
		while (!cc && soln_rec.hhbr_hash == inmr_rec.hhbr_hash)
		{
			if (SOLN_BACKORDER && 
			    soln_rec.hhcc_hash == ccmr_rec.hhcc_hash)
			{
				/*
				 * Check Sales Order Header to ensure no 
				 * packing slip outstanding.            
				 */
				if (!CheckSohr (soln_rec.hhso_hash))
					GetData ();
			}
			cc = find_rec (soln, &soln_rec, NEXT, "r");
		}
	}
	else
	{
		abc_selfield (soln, "soln_id_no");
		abc_selfield (sohr, "sohr_hhso_hash");
		soln_rec.hhso_hash = sohr_rec.hhso_hash;
		soln_rec.line_no = 0;
		cc = find_rec (soln, &soln_rec, GTEQ, "r");
		while (!cc && soln_rec.hhso_hash == sohr_rec.hhso_hash)
		{
			if (soln_rec.hhcc_hash != ccmr_rec.hhcc_hash)
			{
				print_mess ("Invalid Warehouse!!");
				break;
			}
			if (soln_rec.hhbr_hash == inmr_rec.hhbr_hash &&
			     SOLN_BACKORDER)
				GetData ();
			
			cc = find_rec (soln, &soln_rec, NEXT, "r");
		}
		abc_selfield (soln, "soln_hhbr_hash");
	}
}

void
LoadTransfers (void)
{
	/*
	 * All orders selected.
	 */
	if (!strncmp (local_rec.order_no, "ALL", 3))
	{
		abc_selfield (itln, "itln_hhbr_hash");
		itln_rec.hhbr_hash = inmr_rec.hhbr_hash;
		cc = find_rec (itln, &itln_rec, GTEQ, "r");
		while (!cc && itln_rec.hhbr_hash == inmr_rec.hhbr_hash)
		{
			if (itln_rec.i_hhcc_hash == ccmr_rec.hhcc_hash &&
			  (ITLN_BACKORDER) &&
			  (VALID_ITLN))
				GetData ();

			cc = find_rec (itln, &itln_rec, NEXT, "r");
		}
		abc_selfield (itln, "itln_id_no");
	}
	else
	{
		itln_rec.hhit_hash = ithr_rec.hhit_hash;
		itln_rec.line_no = 0;
		cc = find_rec (itln, &itln_rec, GTEQ, "r");
		while (!cc && itln_rec.hhit_hash == ithr_rec.hhit_hash)
		{
		        if (itln_rec.hhbr_hash == inmr_rec.hhbr_hash &&
			     itln_rec.i_hhcc_hash == ccmr_rec.hhcc_hash &&
			   (ITLN_BACKORDER) &&
			   (VALID_ITLN))
				GetData ();
			
			cc = find_rec (itln, &itln_rec, NEXT, "r");
		}
	}
}

/*
 * Process Sales order header and check if packing slip outstanding.
 */
int
CheckSohr (
	long	hhsoHash)
{
	sohr_rec.hhso_hash	=	hhsoHash;
	return (find_rec (sohr, &sohr_rec, COMPARISON, "u"));
}

int
GetData (void)
{
	if (transEnable)
	{
		ithr_rec.hhit_hash = itln_rec.hhit_hash;
		cc = find_rec (ithr2, &ithr_rec, COMPARISON, "r");
		if (cc)
 	        	file_err (cc, ithr2, "DBFIND");

		strcpy (local_rec.acro, 
			 (itln_rec.stock [0] == 'C') ? " Customer" : " Stock");

		sprintf (local_rec.ord_no, "%06ld", ithr_rec.del_no);
		strcpy (local_rec.bo_flag, "Y");
		local_rec.ord_date 		= ithr_rec.iss_date;
		local_rec.reqd_date 	= itln_rec.due_date;
		local_rec.order_bal 	= itln_rec.qty_border;
		local_rec.hhum_hash 	= itln_rec.hhum_hash;
		local_rec.release 		= 0.00;
		local_rec.backorder 	= itln_rec.qty_border;
		local_rec.hash 			= itln_rec.itff_hash;
		local_rec.qty_bord 		= itln_rec.qty_border;
		strcpy (local_rec.fs_flag, itln_rec.full_supply);
	}
	else
	{
		cumr_rec.hhcu_hash	=	sohr_rec.hhcu_hash;
		if (find_rec (cumr, &cumr_rec, COMPARISON, "r"))
			return (EXIT_FAILURE); 

		strcpy (local_rec.acro, cumr_rec.dbt_acronym);
		strcpy (local_rec.ord_no, sohr_rec.order_no);
		strcpy (local_rec.bo_flag, cumr_rec.bo_flag);
		local_rec.ord_date = sohr_rec.dt_raised;
		local_rec.reqd_date = soln_rec.due_date;
		local_rec.order_bal = soln_rec.qty_order + soln_rec.qty_bord;
		local_rec.release = soln_rec.qty_order;
		local_rec.backorder = soln_rec.qty_bord;
		local_rec.hash = soln_rec.hhsl_hash;
		local_rec.qty_bord = soln_rec.qty_bord;
	}
	store [lcount [2]]._release = 0.00;

	putval (lcount [2]++);
    return (EXIT_SUCCESS);
}

/*
 * Update soln Lines.
 */
void
SolnUpdate (void)
{
	int		released_all	= TRUE;
	long	hhsoHash		=	0L;

	clear ();
	print_at (0, 0, ML (mlStdMess035));
	fflush (stdout);

	abc_selfield (soln, "soln_hhsl_hash");

	scn_set (2);
	for (line_cnt = 0; line_cnt < lcount [2]; line_cnt++)
	{
		getval (line_cnt);

		soln_rec.hhsl_hash	=	local_rec.hash;
		if (find_rec (soln, &soln_rec, EQUAL, "u"))
		{
			abc_unlock (soln);
			continue;
		}
		if (local_rec.release > 0.00)
		{
			UpdateSoln ();
			sohr_rec.hhso_hash	=	soln_rec.hhso_hash;
			cc = find_rec (sohr, &sohr_rec, EQUAL, "u");
			if (cc)
				continue; 

			strcpy (sohr_rec.sohr_new, "N");
			strcpy (sohr_rec.status, "C");
			strcpy (sohr_rec.stat_flag, (envVarConOrders) ? "M" : "R");
			hhsoHash = sohr_rec.hhso_hash;

			cc = abc_update (sohr, &sohr_rec);
			if (cc) 
				file_err (cc, sohr, "DBUPDATE");
		}
		else
		{
			if (local_rec.release == 0.00 && local_rec.backorder == 0.00)
				DeleteRecords ();
			else
			{
				BackorderSoln ();
				released_all = FALSE;
			}
		}
	}
	if (released_all)
		UpdateSolnRelease (hhsoHash);

	abc_selfield (soln, "soln_hhbr_hash");
	abc_unlock (soln);
	abc_unlock (sohr);
}

void
UpdateSolnRelease (
	long	hhsoHash)
{
 	abc_selfield (soln, "soln_id_no");
	soln_rec.hhso_hash	= hhsoHash;
	soln_rec.line_no 	= 0;
	cc = find_rec (soln, &soln_rec, GTEQ, "u");
	while (!cc && soln_rec.hhso_hash == hhsoHash)
	{
		if (soln_rec.status [0] == 'H')
		{
			strcpy (soln_rec.stat_flag, (envVarConOrders) ? "M" : "R");
			cc = abc_update (soln, &soln_rec);
			if (cc)
				file_err (cc, soln, "DBUPDATE");
		}
		cc = find_rec (soln, &soln_rec, NEXT, "u");
  	}
	abc_selfield (soln, "soln_hhsl_hash");
}

void
UpdateSoln (void)
{
	strcpy (soln_rec.status, "C");
	strcpy (soln_rec.stat_flag, (envVarConOrders) ? "M" : "R");

	soln_rec.qty_bord 	= local_rec.backorder;
	soln_rec.qty_order 	= local_rec.release;
	DeleteColn (soln_rec.hhsl_hash);
	cc = abc_update (soln, &soln_rec);
	if (cc) 
		file_err (cc, soln, "DBUPDATE");

	add_hash 
	(
		comm_rec.co_no, 
		comm_rec.est_no, 
		"RC", 
		0, 
		soln_rec.hhbr_hash, 
		soln_rec.hhcc_hash, 
		0L, 
		(double) 0.00
	);
}

void
BackorderSoln (void)
{
	soln_rec.qty_order 	= local_rec.backorder;
	soln_rec.qty_bord 	= 0.0;
	DeleteColn (soln_rec.hhsl_hash);
	cc = abc_update (soln, &soln_rec);
	if (cc) 
		file_err (cc, soln, "DBUPDATE");

	add_hash 
	(
		comm_rec.co_no, 
		comm_rec.est_no, 
		"RC", 
		0, 
		soln_rec.hhbr_hash, 
		soln_rec.hhcc_hash, 
		0L, 
		(double) 0.00
	);
}

void
DeleteRecords (void)
{
	int		line_no = 0;
	long	hhsoHash = 0L;

	/*
	 * Delete appropriate line
	 */
	cc = find_rec (soln, &soln_rec, COMPARISON, "u");
	if (!cc)
	{	
		DeleteColn (soln_rec.hhsl_hash);

		hhsoHash 	= soln_rec.hhso_hash;
		line_no 	= soln_rec.line_no;
		abc_unlock (soln);
		abc_delete (soln);
	}

	/*
	 * Delete COMMENT Items related to the item deleted
	 */
	abc_selfield (soln, "soln_id_no");
	abc_selfield (inmr, "inmr_hhbr_hash");
	soln_rec.hhso_hash 	= hhsoHash;
	soln_rec.line_no 	= line_no;
	cc = find_rec (soln, &soln_rec, GTEQ, "u");
	while (!cc && soln_rec.hhso_hash == hhsoHash)
	{
		inmr_rec.hhbr_hash	=	soln_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");

		/*
		 * Only delete if NON_STOCK
		 */
		if (!cc && COMMENT_LINE)
		{
			DeleteColn (soln_rec.hhsl_hash);
			abc_unlock (soln);
			cc = abc_delete (soln);
			if (cc) 
				file_err (cc, soln, "DBDELETE");
		}
		else
			break;

		soln_rec.hhso_hash 	= hhsoHash;
		soln_rec.line_no 	= line_no;
		cc = find_rec (soln, &soln_rec, GTEQ, "u");
	}
	abc_unlock (soln);

	/*
	 * After Deleting the Line, Find the Header and
	 * then see if there is any Lines Exists, if
	 * delete the header as well.			
	 */
	sohr_rec.hhso_hash	=	hhsoHash;
	cc = find_rec (sohr, &sohr_rec, COMPARISON, "u");
	if (cc)
		file_err (cc, sohr, "DBFIND");

	soln_rec.hhso_hash 	= sohr_rec.hhso_hash;
	soln_rec.line_no 	= 0;
	cc = find_rec (soln, &soln_rec, GTEQ, "r");
	/*
	 * Not sales order lines
	 */
	if (cc || soln_rec.hhso_hash != sohr_rec.hhso_hash)
	{
		abc_unlock (sohr);
		cc = abc_delete (sohr);
		if (cc) 
			file_err (cc, sohr, "DBDELETE");
	}
	abc_unlock (sohr);
	abc_selfield (soln, "soln_hhsl_hash");
	abc_selfield (inmr, "inmr_id_no");
}

/*
 * Delete line from existing packing slip so new backorder can be released.
 */
void
DeleteColn (
	long	hhslHash)
{
	coln_rec.hhsl_hash	=	hhslHash;
	cc = find_rec (coln, &coln_rec, COMPARISON, "u");
	if (cc)
	{
		abc_unlock (coln);
		return;
	}
	if (soln_rec.status [0] == 'P')
		abc_delete (coln);
	else
		abc_unlock (coln);
	
	return;
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
		rv_pr (ML (mlSoMess188), 20, 0, 1);
		line_at (1, 0, 80);

		switch (scn)
		{
		case	1:			
			line_at (3, 0, 80);
			line_at (6, 0, 80);
			line_at (9, 0, 80);			
			break;

		case	2:
		line_at (1, 0, 80);
			PrintRelease ();
			break;
		}

		line_at (20, 0, 80);
		print_at (21, 0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
		print_at (22, 0, ML (mlStdMess039), comm_rec.est_no, comm_rec.est_name);

		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}
/*
 * Update itln Lines.
 */
void
ItlnUpdate (void)
{
	struct	RCV_CC_LIST	*tmp_list;

	clear ();
	print_at (0, 0, ML (mlStdMess035));
	fflush (stdout);

	abc_selfield (itln, "itln_itff_hash");

	scn_set (2);
	for (line_cnt = 0; line_cnt < lcount [2]; line_cnt++)
	{
		getval (line_cnt);

		if (local_rec.release == 0.00)
			continue;

		itln_rec.itff_hash	=	local_rec.hash;
		if (find_rec (itln, &itln_rec, EQUAL, "u"))
		{
			abc_unlock (itln);
			continue;
		}

		if (itln_rec.full_supply [0] == 'Y')
		{
			itln_rec.qty_order  = local_rec.release;
			itln_rec.qty_border = local_rec.order_bal - 
						 local_rec.release;

			if (itln_rec.qty_border != 0.00)
			{
				abc_unlock (itln);
				continue;
			}

			strcpy (itln_rec.status, "U");

			cc = abc_update (itln, &itln_rec);
			if (cc)
 	        		file_err (cc, itln, "DBUPDATE");

			UpdateIthr (itln_rec.hhit_hash);
		}
		else
		{
			if (itln_rec.full_supply [0] == 'L' && 
                             itln_rec.qty_border != local_rec.release)
			{
				abc_unlock (itln);
				continue;
			}
			itln_rec.qty_border = 0.00;
			strcpy (itln_rec.status, 
				 (itln_rec.qty_order == 0.00) ? "D" : "T");

			cc = abc_update (itln, &itln_rec);
			if (cc)
 	        		file_err (cc, itln, "DBUPDATE");

			tmp_list = FindIthr ();
			itln_rec.hhit_hash = tmp_list->hhit_hash;
			itln_rec.line_no   = tmp_list->line_no++;
			itln_rec.qty_order = local_rec.release;
			itln_rec.qty_border = local_rec.order_bal - 
					         local_rec.release;
			itln_rec.hhum_hash = local_rec.hhum_hash;

			strcpy (itln_rec.status, "U");
			cc = abc_add (itln, &itln_rec);
			if (cc)
 	        		file_err (cc, itln, "DBADD");

			abc_unlock (itln);
		}

		add_hash 
		(
			comm_rec.co_no, 
			comm_rec.est_no, 
			"RC", 
			0, 
			itln_rec.hhbr_hash, 
			itln_rec.i_hhcc_hash, 
			0L, 
			(double) 0.00
		);

		if (itln_rec.r_hhbr_hash != 0L)
		{
			add_hash 
			(
				comm_rec.co_no, 
				comm_rec.est_no, 
				"RC", 
				0, 
				itln_rec.r_hhbr_hash, 
				itln_rec.r_hhcc_hash, 
				0L, 
				(double) 0.00
			);
		}
		else
		{
			add_hash 
			(
				comm_rec.co_no, 
				comm_rec.est_no, 
				"RC", 
				0, 
				itln_rec.hhbr_hash, 
				itln_rec.r_hhcc_hash, 
				0L, 
				(double) 0.00
			);
		}
	}
	abc_unlock (itln);
	abc_selfield (itln, "itln_id_no");
}

/*
 * Update Header to same status of lines.
 */
void
UpdateIthr (
	long	hhitHash)
{
	ithr_rec.hhit_hash	=	hhitHash;
	cc = find_rec (ithr2, &ithr_rec, COMPARISON, "u");
	if (!cc)
	{
		strcpy (ithr_rec.type, "U");
		if (ithr_rec.full_supply [0] != 'Y')
			ithr_rec.del_no = 0L;
		cc = abc_update (ithr2, &ithr_rec);
		if (cc)
			file_err (cc, ithr2, "DBUPDATE");
	}
	else
		abc_unlock (ithr2);
}

void
FreeCcList (void)
{
	struct	RCV_CC_LIST	*tmp_list = RCV_CC_NULL;

	rcv_cc_curr = rcv_cc_head;
	while (rcv_cc_curr != NULL)
	{
		tmp_list = rcv_cc_curr;
		rcv_cc_curr = rcv_cc_curr->_next;
		free (tmp_list);
	}
}

struct	RCV_CC_LIST	*
FindIthr (void)
{
	struct	RCV_CC_LIST	*tmp_list = (struct RCV_CC_LIST *) 0;

	if (rcv_cc_head == RCV_CC_NULL)
	{
		rcv_cc_head = (struct RCV_CC_LIST *) malloc (sizeof (struct RCV_CC_LIST));
		if (rcv_cc_head == RCV_CC_NULL)
 	        	file_err (errno, "FindIthr", "MALLOC");

		rcv_cc_head->hhcc_hash = itln_rec.r_hhcc_hash;
		rcv_cc_head->hhit_hash = CreateIthr ();
		rcv_cc_head->line_no = 1;
		rcv_cc_head->_next = RCV_CC_NULL;

		return (rcv_cc_head);
	}

	for (rcv_cc_curr = rcv_cc_head; rcv_cc_curr != RCV_CC_NULL; rcv_cc_curr = rcv_cc_curr->_next)
	{
		if (rcv_cc_curr->hhcc_hash == itln_rec.r_hhcc_hash)
			return (rcv_cc_curr);

		tmp_list = rcv_cc_curr;
	}

	rcv_cc_curr = tmp_list;
	tmp_list = (struct RCV_CC_LIST *) malloc (sizeof (struct RCV_CC_LIST));
	if (tmp_list == RCV_CC_NULL)
 	        file_err (errno, "FindIthr", "MALLOC");
		
	rcv_cc_curr->_next = tmp_list;
	tmp_list->hhcc_hash = itln_rec.r_hhcc_hash;
	tmp_list->hhit_hash = CreateIthr ();
	tmp_list->line_no = 1;
	tmp_list->_next = RCV_CC_NULL;

	return (tmp_list);
}

long	
CreateIthr (void)
{
	char	tmp_date [11], 
			iss_co [3], 
			rec_co [3];

	ccmr2_rec.hhcc_hash = itln_rec.r_hhcc_hash;
	cc = find_rec (ccmr, &ccmr2_rec, EQUAL, "r");
	if (cc)
		file_err (cc, ccmr, "DBFIND");

	strcpy (rec_co, ccmr2_rec.co_no);

	ccmr2_rec.hhcc_hash = itln_rec.i_hhcc_hash;
	cc = find_rec (ccmr, &ccmr2_rec, EQUAL, "r");
	if (cc)
 	        file_err (cc, ccmr, "DBFIND");

	strcpy (iss_co, ccmr2_rec.co_no);

	strcpy (tmp_date, DateToString (TodaysDate ()));

	if (strcmp (iss_co, rec_co))
		strcpy (ithr_rec.co_no, "  ");
	else
		strcpy (ithr_rec.co_no, iss_co);

	strcpy (ithr_rec.type, "U");
	ithr_rec.del_no 	= 900000L + (long) pid;
	ithr_rec.iss_sdate 	= StringToDate (tmp_date);
	ithr_rec.iss_date 	= StringToDate (tmp_date);
	ithr_rec.rec_date 	= 0L;
	strcpy (ithr_rec.tran_ref, "Released Tran BO");
	strcpy (ithr_rec.printed, "N");
	strcpy (ithr_rec.stat_flag, "0");
	sprintf (ithr_rec.op_id, "%-14.14s", currentUser);
	ithr_rec.date_create = TodaysDate ();
	strcpy (ithr_rec.time_create, TimeHHMM ());

	cc = abc_add (ithr, &ithr_rec);
	if (cc)
 	        file_err (cc, ithr, "DBADD");

	cc = find_rec (ithr, &ithr_rec, EQUAL, "u");
	if (cc)
 	        file_err (cc, ithr, "DBFIND");

	ithr_rec.del_no = 0L;
	cc = abc_update (ithr, &ithr_rec);
	if (cc)
 	        file_err (cc, ithr, "DBUPDATE");

	return (ithr_rec.hhit_hash);
}

/*
 * Specific code to handle single level Bills.
 */
float	
ProcessPhantom (
	long	hhbrHash)
{
	float	min_qty = 0.00, 
			on_hand = 0.00;

	float	realCommitted;

	int		first_time = TRUE;

	open_rec (sokt, sokt_list, SOKT_NO_FIELDS, "sokt_hhbr_hash");

	sokt_rec.hhbr_hash	=	hhbrHash;
	cc = find_rec (sokt, &sokt_rec, GTEQ, "r");
	while (!cc && sokt_rec.hhbr_hash == hhbrHash)
	{
		incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
		incc_rec.hhbr_hash = sokt_rec.mabr_hash;
		cc = find_rec (incc, &incc_rec, COMPARISON, "r");
		if (cc)
		{
			cc = find_rec (sokt, &sokt_rec, NEXT, "r");
			continue;
		}
		/*
		 * Calculate Actual Qty Committed.
		 */
		realCommitted = RealTimeCommitted (incc_rec.hhbr_hash, 
											incc_rec.hhcc_hash);
		if (envVarSoFwdRel)
		{
			on_hand = incc_rec.closing_stock -
		   	  	  	  incc_rec.committed -
					  realCommitted -
		   	          incc_rec.forward;
		}
		else
		{
			on_hand = incc_rec.closing_stock -
					  incc_rec.committed -
					  realCommitted;
		}
		if (envVarQcApply && envVarSkQcAvl)
			on_hand -= incc_rec.qc_qty;

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

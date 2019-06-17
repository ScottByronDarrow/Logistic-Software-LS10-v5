/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_nst_fix.c,v 5.8 2002/07/24 08:39:15 scott Exp $
|  Program Name  : ( sk_nst_fix.c   )                                 |
|  Program Desc  : ( Stock Take Input.                            )   |
|---------------------------------------------------------------------|
|  Author        : Roger Gibbison  | Date Written  : 28/03/89         |
|---------------------------------------------------------------------|
| $Log: sk_nst_fix.c,v $
| Revision 5.8  2002/07/24 08:39:15  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.7  2002/07/18 07:15:54  scott
| Updated to make sure lcount [] is being set to zero at top of while.
|
| Revision 5.6  2002/07/17 09:57:57  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.5  2002/07/16 02:43:43  scott
| Updated from service calls and general maintenance.
|
| Revision 5.4  2002/06/26 05:48:50  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.3  2002/06/20 07:11:06  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.2  2001/08/09 09:19:22  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:45:26  scott
| RELEASE 5.0
|
| Revision 5.0  2001/06/19 08:16:49  robert
| LS10-5.0 New Release as of 19 JUNE 2001
|
| Revision 4.0  2001/03/09 02:38:10  scott
| LS10-4.0 New Release as at 10th March 2001
|
| Revision 3.1  2000/11/20 07:40:18  scott
| New features related to 3PL environment
| New features related to Number Plates
| All covered in release 3 notes
|
| Revision 3.0  2000/10/10 12:20:45  gerry
| Revision No. 3 Start
| <after Rel-10102000>
|
| Revision 2.1  2000/08/15 07:56:06  scott
| Updated to add app.schema and perform general cleanup.
| Program will form base for sk_st_rfinp.c (RF stock take input)
|
| Revision 2.0  2000/07/15 09:11:27  gerry
| Forced Revision No Start 2.0 Rel-15072000
|
| Revision 1.24  2000/06/13 05:03:11  scott
| New Search routine that allow multi level searches using
| Item number, description, Alpha code, alternate no, maker number, selling group,
| buying group using AND or OR searches.
| New search beings routines into the library to speed up GVision.
| See seperate release notes.
|
| Revision 1.23  2000/03/15 05:40:30  vij
| inserted sleep () after displaying error message
|
| Revision 1.22  2000/01/13 20:37:31  cam
| Changes for GVision compatibility.  Separated description for Audit Required.
|
| Revision 1.22  2000/01/13 20:37:31  cam
| Changes for GVision compatibility.  Separated description for Audit Required.
|
| Revision 1.21  1999/12/06 01:31:03  scott
| Updated to change CTime to SystemTime due to conflict with VisualC++.
|
| Revision 1.20  1999/11/11 05:59:58  scott
| Updated to remove display of program name as ^P can be used.
|
| Revision 1.19  1999/11/03 07:32:15  scott
| Updated to fix small compile errors due to -Wall flag being set.
|
| Revision 1.18  1999/10/08 05:32:38  scott
| First Pass checkin by Scott.
|
| Revision 1.17  1999/06/20 05:20:23  scott
| Updated to add log for cvs + remove old read_comm + fixed all warnings.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_nst_fix.c,v $";
char	*PROG_VERSION = "@ (#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_nst_fix/sk_nst_fix.c,v 5.8 2002/07/24 08:39:15 scott Exp $";

#define	MAXWIDTH	150
#define	MAXLINES	500

#include 	<pslscr.h>
#include 	<getnum.h>
#include	<get_lpno.h>
#include	<dsp_screen.h>
#include	<dsp_process2.h>
#include	<twodec.h>
#include	<ml_std_mess.h>
#include	<ml_sk_mess.h>

#define	AUDIT		 (local_rec.audit [0] == 'Y')
#define	SK_HHBR_HASH	store [line_cnt]._hhbr_hash
#define	SK_HHUM_HASH	store [line_cnt]._hhum_hash
#define	SK_HHWH_HASH	store [line_cnt]._hhwh_hash
#define	SK_QUANTITY		store [line_cnt]._qty
#define	SK_CNV_FCT		store [line_cnt]._cnv_fct
#define	SK_UOM			store [line_cnt]._uom
#define	SR				store [line_cnt]
#define	LL_SPECIAL		 (MULT_LOC && !SK_BATCH_CONT)
#define	LOT_ITEM		 (SR._lot_ctrl [0] == 'Y')

#include	"schema"

struct commRecord	comm_rec;
struct inscRecord	insc_rec;
struct ccmrRecord	ccmr_rec;
struct inmrRecord	inmr_rec;
struct inccRecord	incc_rec;
struct sttfRecord	sttf_rec;
struct inumRecord	inum_rec;
struct inumRecord	inum2_rec;

	FILE	*fout;

	char	*data 	= "data",
			*inum2 	= "inum2";

	struct	storeRec {
		long	_hhbr_hash;
		long	_hhwh_hash;
		long	_hhcc_hash;
		long	_hhum_hash;
		float	_qty;
		char	_uom [5];
		char	_uom_group [21];
		float	_cnv_fct;
		float	_StdCnvFct;
		char	_lot_ctrl [2];
	} store [MAXLINES];

	double		TotalLineQty 	=	0.00;

	int		envVarSkStPfrz = 0, 
			envVarStExpDate = 0;

	char	systemDate [11];
	char	lot_ctrl_dflt [2];
	long	lsystemDate;

	char	*envVarSkIvalClass;
 	char 	*result;

/*============================ 
| Local & Screen Structures. |
============================*/
struct {
	char	dummy [11];
	char	audit [2];
	char	auditDesc [6];
	int		lpno;
	long	page_no;
	long	lst_page;
	double	qty;
	double	disp_tqty;
	long	exp_date;
	char	dflt_qty [15];
	char	rep_qty [10];
	char	uom [5];
	int		dec_pt;
	long	hhwh_hash;
	char	LL [2];
	char	Location [11];
} local_rec;

#define	DUMM	0
#define	HEAD	1
#define	ITEM	2

static	struct	var	vars [] =
{
	{HEAD, LIN, "stake_code", 	 4, 30, CHARTYPE, 
		"U", "          ", 
		" ", "", "Stock Take Selection Code.", "", 
		 NE, NO,  JUSTLEFT, "", "", insc_rec.stake_code}, 
	{HEAD, LIN, "stake_desc", 	 5, 30, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Stock Selection Desc", "", 
		 NA, NO,  JUSTLEFT, "", "", insc_rec.description}, 
	{HEAD, LIN, "stake_date", 	 6, 30, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", "", "Start Date", "", 
		 NA, NO,  JUSTLEFT, "", "", (char *)&insc_rec.start_date}, 
	{HEAD, LIN, "stake_time", 	 7, 30, CHARTYPE, 
		"AAAAA", "          ", 
		" ", "", "Start Time", "", 
		 NA, NO,  JUSTLEFT, "", "", insc_rec.start_time}, 
	{HEAD, LIN, "audit", 	 9, 30, CHARTYPE, 
		"U", "          ", 
		" ", "Y", "Audit Required", "", 
		YES, NO,  JUSTLEFT, "YN", "", local_rec.audit}, 
	{HEAD, LIN, "auditDesc", 	 9, 33, CHARTYPE, 
		"UUU", "          ", 
		" ", " ", "", "", 
		NA, NO,  JUSTLEFT, "", "", local_rec.auditDesc}, 
	{HEAD, LIN, "lpno", 	10, 30, INTTYPE, 
		"NN", "          ", 
		" ", "1", "Printer", "", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.lpno}, 

	{ITEM, TAB, "item_no", 	MAXLINES, 0, CHARTYPE, 
		"UUUUUUUUUUUUUUUU", "          ", 
		" ", " ", "  Item  Number  ", "", 
		YES, NO,  JUSTLEFT, "", "", inmr_rec.item_no}, 
	{ITEM, TAB, "desc", 	 0, 0, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "          D e s c r i p t i o n         ", "", 
		 NA, NO,  JUSTLEFT, "", "", inmr_rec.description}, 
	{ITEM, TAB, "UOM", 	 0, 1, CHARTYPE, 
		"AAAA", "          ", 
		" ", " ", " UOM. ", "Enter Input Unit of Measure [SEARCH]",
		YES, NO,  JUSTLEFT, "", "", local_rec.uom}, 
	{ITEM, TAB, "hhwh_hash",	 0,  0, LONGTYPE,
		"NNNNNNNNN", "          ",
		" ", " ", "", "",
		ND, NO, JUSTRIGHT, "", "", (char *) &local_rec.hhwh_hash},
	{ITEM, TAB, "qty", 	 0, 0, DOUBLETYPE, 
		local_rec.dflt_qty, "          ", 
		" ", "0", "  Quantity  ", "", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.qty}, 
	{ITEM, TAB, "location", 	 0, 0, CHARTYPE, 
		"UUUUUUUUUU", "          ", 
		" ", " ", " Location ", "", 
		YES, NO,  JUSTLEFT, "", "", local_rec.Location}, 
	{ITEM, TAB, "LL", 	 0, 0, CHARTYPE, 
		"U", "          ", 
		" ", " ", "L", "Lot / Location Selection. <return> ", 
		 YES, NO,  JUSTLEFT, "", "", local_rec.LL}, 
	{ITEM, TAB, "page_no", 	 0, 3, LONGTYPE, 
		"NNNNNN", "          ", 
		" ", "0", "Page # Ref.", "", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.page_no}, 
	{ITEM, TAB, "exp_date", 	 0, 1, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", " ", "Expiry Date", "", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.exp_date}, 
	{ITEM, TAB, "dec_pt",	 0, 0, INTTYPE,
		"N", "          ",
		"", "", "", "",
		 ND, NO,  JUSTRIGHT, "", "", (char *) &local_rec.dec_pt},
	{DUMM, LIN, "", 	 0, 0, CHARTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}, 
};

#include	<LocHeader.h>

/*======================= 
| Function Declarations |
========================*/
int 	heading 		(int);
int  	DeleteLine 		(int);
int  	ValidQuantity 	(double, int);
int  	spec_valid 		(int);
void 	CloseDB 		(void);
void 	DrawBox 		(void);
void 	DrawQuantity 	(int);
void 	HeadAudit 		(void);
void 	OpenDB 			(void);
void 	PrintCoStuff 	(void);
void 	ReadCcmr 		(void);
void 	SrchInsc 		(void);
void 	SrchInum 		(char *, int);
void 	TailAudit 		(double);
void 	UpdateData 		(void);
void 	UpdateStake 	(int);
void 	UpdateStdStake 	(int, int);
void 	tab_other 		(int);

/*=========================== 
| Main Processing Routine . |
===========================*/
int
main (
 int argc,
 char * argv [])
{
	char	*sptr;
	int		after, 
			before;

	strcpy (lot_ctrl_dflt, "N");

	sptr = chk_env ("SK_QTY_MASK");
	if (sptr == (char *)0)
		strcpy (local_rec.dflt_qty, "NNNNNNN.NNNNNN");
	else
		strcpy (local_rec.dflt_qty, sptr);

	before = strlen (local_rec.dflt_qty);
	sptr = strrchr (local_rec.dflt_qty, '.');
	if (sptr)
		after = (int) ( (sptr + strlen (sptr) - 1) - sptr);
	else
		after = 0;

	if (after == 0)
		sprintf (local_rec.rep_qty, "%%%df", before);
	else
		sprintf (local_rec.rep_qty, "%%%d.%df", before, after);

	strcpy (systemDate, DateToString (TodaysDate ()));
	lsystemDate = TodaysDate ();

	SETUP_SCR (vars);


	sptr = chk_env ("SK_ST_PFRZ");
	if (sptr != (char *)0)
		envVarSkStPfrz = atoi (sptr);

	sptr = get_env ("ST_EXP_DATE");
	if (sptr != (char *)0)
		envVarStExpDate = atoi (sptr);
	else
		envVarStExpDate = 0;

	if (!envVarStExpDate)
		FLD ("exp_date") = ND;

	sptr = chk_env ("SK_IVAL_CLASS");
	if (sptr)
		envVarSkIvalClass = strdup (sptr);
	else
		envVarSkIvalClass = strdup ("ZKPN");

	upshift (envVarSkIvalClass); 

	tab_row	= 3;
	init_scr ();
	set_tty ();
	set_masks ();
/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (ITEM, store, sizeof (struct storeRec));
#endif
	init_vars (1);

	OpenDB ();

	FLD ("LL") 		 = ND;
	FLD ("location") = ND;

	if (SK_BATCH_CONT)
		FLD ("LL") = YES;

	if (MULT_LOC && !SK_BATCH_CONT)
		FLD ("location") = YES;

	swide ();

	while (prog_exit == 0)
	{
		entry_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		search_ok	= TRUE;

		init_vars (HEAD);
		init_vars (ITEM);
		lcount [ITEM] = 0;

		heading (HEAD);
		entry (HEAD);
		if (restart || prog_exit)
			continue;

		heading (ITEM);
		entry (ITEM);
		if (restart)
			continue;

		edit_all ();
		if (restart)
			continue;

		UpdateData ();
	}
	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

int
spec_valid (
 int field)
{
	int		TempLine;
	char	workLocationType [2];
	long	workInloHash	=	0;

	if (LCHECK ("stake_code"))
	{
		if (SRCH_KEY)
		{
			SrchInsc ();
			return (EXIT_SUCCESS);
		}
		insc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
		cc = find_rec (insc, &insc_rec, COMPARISON, "r");
		if (cc)
		{
			sprintf (err_str, ML (mlSkMess047) ,insc_rec.stake_code);
			print_mess (err_str);
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		DSP_FLD ("stake_desc");
		DSP_FLD ("stake_date");
		DSP_FLD ("stake_time");
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("audit"))
	{
		strcpy (local_rec.auditDesc, (AUDIT) ? "Yes" : "No ");
		DSP_FLD ("auditDesc");
		FLD ("lpno") = (AUDIT) ? YES : NA;
	}

	if (LCHECK ("lpno"))
	{
		if (F_NOKEY (label ("lpno")))
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			local_rec.lpno = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if (!valid_lp (local_rec.lpno))
		{
			print_mess (ML (mlStdMess020));
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("item_no"))
	{
		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
			return (DeleteLine (2));

		cc = FindInmr (comm_rec.co_no, inmr_rec.item_no, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, inmr_rec.item_no);
			cc = find_rec ("inmr", &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			errmess (ML (mlStdMess001));
			sleep (sleepTime);
			return (EXIT_FAILURE); 
		}
		SuperSynonymError ();

		result = strrchr (envVarSkIvalClass, inmr_rec.inmr_class [0]);
		if (result)
		{

			sprintf (err_str, ML (mlSkMess211),inmr_rec.item_no);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
		incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
		cc = find_rec (incc, &incc_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess192));
			return (EXIT_FAILURE); 
		}
		if (incc_rec.stat_flag [0] != insc_rec.stake_code [0] &&
		   (incc_rec.stat_flag [0] != '0' || !envVarSkStPfrz))
		{
			sprintf (err_str, ML (mlSkMess212), incc_rec.stat_flag);
			print_mess (err_str);
			return (EXIT_FAILURE); 
		}

		inum_rec.hhum_hash = inmr_rec.std_uom;
		cc = find_rec (inum, &inum_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess028));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		if (inum_rec.cnv_fct == 0.00)
		{
			print_mess (ML (mlSkMess553));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		DSP_FLD ("item_no");
		DSP_FLD ("desc");

		strcpy (SR._uom, inum_rec.uom);
		strcpy (SR._uom_group, inum_rec.uom_group);
		SR._StdCnvFct 		= inum_rec.cnv_fct;
		SK_HHBR_HASH 		= inmr_rec.hhbr_hash;
		SK_HHWH_HASH 		= incc_rec.hhwh_hash;
		SR._hhcc_hash 		= incc_rec.hhcc_hash;
		local_rec.dec_pt	= inmr_rec.dec_pt;
		local_rec.hhwh_hash = incc_rec.hhwh_hash;
		strcpy (SR._lot_ctrl, inmr_rec.lot_ctrl);

		if (prog_status == ENTRY)
			SK_QUANTITY = 0.00;

		DrawQuantity (line_cnt);
		tab_other (line_cnt);
		return (EXIT_SUCCESS);
	}

	/*--------------------------
	| Validate Unit of Measure |
	--------------------------*/
	if (LCHECK ("UOM"))
	{
		if (F_NOKEY (label ("UOM")) || dflt_used)
			strcpy (local_rec.uom, SR._uom);

		if (SRCH_KEY)
		{
			SrchInum (temp_str, line_cnt);
			return (EXIT_SUCCESS);
		}

		strcpy (inum2_rec.uom_group, SR._uom_group);
		strcpy (inum2_rec.uom, local_rec.uom);
		cc = find_rec (inum2, &inum2_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess028));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (ValidItemUom (SR._hhbr_hash, inum2_rec.hhum_hash))
		{
			print_mess (ML (mlStdMess028));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.uom, inum2_rec.uom);
		SR._hhum_hash = inum2_rec.hhum_hash;
		strcpy (SR._uom, inum2_rec.uom);
        if ( inum2_rec.cnv_fct == 0.00)
             inum2_rec.cnv_fct = 1.00;

        SR._cnv_fct = inum2_rec.cnv_fct / SR._StdCnvFct;
		strcpy (SR._uom, inum2_rec.uom);
		strcpy (SR._uom_group, inum2_rec.uom_group);

		DSP_FLD ("UOM");
		return (EXIT_SUCCESS);
	}

	/*------------------------
	| Qty validation lookup. |
	------------------------*/
	if (LCHECK ("qty"))
	{
		strcpy (inmr_rec.co_no, comm_rec.co_no);
		cc = find_rec (inmr, &inmr_rec, EQUAL, "r");
		if (cc)
			file_err (cc, "inmr", "DBFIND");

		local_rec.qty = n_dec (atof (temp_str), inmr_rec.dec_pt);
		DSP_FLD ("qty");

		if (!ValidQuantity (local_rec.qty, inmr_rec.dec_pt))
		{
			errmess (ML (mlStdMess190));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		SR._qty = (float) local_rec.qty;

		DSP_FLD ("qty");

		tab_other (line_cnt);
		return (EXIT_SUCCESS);
	}

    /*--------------------                                                 
    | Validate Location. |                                                 
    --------------------*/                                                
    if (LCHECK ("location"))                                                     
    {                                                                           
		if (FLD ("location") == ND)
			return (EXIT_SUCCESS);

        if (SRCH_KEY)
        {                                                                       
			if (search_key	!=	SEARCH)
				SearchLomr (SR._hhcc_hash, temp_str);
			else
            	SearchLOC (TRUE, SR._hhwh_hash, temp_str);
            return (EXIT_SUCCESS);                                                          
        }                                                                       
		cc = 	FindLocation 
				 (
					SR._hhwh_hash,
					SR._hhum_hash,
					local_rec.Location,
					ValidLocations,
					&workInloHash
				);
		if (cc)
		{
			cc	=	CheckLocation 
					(
						SR._hhcc_hash, 
						local_rec.Location,
						workLocationType
					);
			if (cc)
			{
            	print_mess (ML (mlStdMess209));
            	sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
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

		LL_EditLoc	=	 (MULT_LOC) ? TRUE : FALSE; 
		LL_EditLot	=	 (LOT_ITEM) ? TRUE : FALSE;
		LL_EditDate	=	 (LOT_ITEM) ? TRUE : FALSE;
		LL_EditSLot	=	 (LOT_ITEM) ? TRUE : FALSE;

		TempLine	=	lcount [2];
		cc = DisplayLL
			 (										/*----------------------*/
				line_cnt,							/*	Line number.		*/
				tab_row + 3 + (line_cnt % TABLINES),/*  Row for window		*/
				tab_col + 22,						/*  Col for window		*/
				4,									/*  length for window	*/
				SR._hhwh_hash, 						/*	Warehouse hash.		*/
				SR._hhum_hash,						/*	UOM hash			*/
				SR._hhcc_hash,						/*	CC hash.			*/
				SR._uom,							/* UOM					*/
				SR._qty,							/* Quantity.			*/
				SR._cnv_fct,						/* Conversion factor.	*/
				TodaysDate (),					    /* Expiry Date.			*/
				FALSE,								/* Silent mode			*/
				FALSE,								/* Input Mode.			*/
				SR._lot_ctrl						/* Lot controled item. 	*/
													/*----------------------*/
			);
		/*-----------------
		| Redraw screens. |
		-----------------*/
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
	if (LCHECK ("page_no"))
	{
		if (dflt_used)
			local_rec.page_no = local_rec.lst_page;

		local_rec.lst_page = local_rec.page_no;
	
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("exp_date"))
	{
		if (F_NOKEY (field))
			return (EXIT_SUCCESS);

		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

int
DeleteLine (
 int scn)
{
	int		i;
	int		this_page = line_cnt / TABLINES;

	if (prog_status == ENTRY)
	{
		print_mess (ML (mlStdMess005));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}

	if (lcount [scn] == 0)
	{
		print_mess (ML (mlStdMess032));
		sleep (sleepTime);
		return (EXIT_FAILURE);
	}
	print_at (2,0,ML (mlStdMess035));
	fflush (stdout);

	lcount [scn]--;

	for (i = line_cnt;line_cnt < lcount [scn];line_cnt++)
	{
		SR._hhbr_hash 			= store [line_cnt + 1]._hhbr_hash;
		SR._hhwh_hash 			= store [line_cnt + 1]._hhwh_hash;
		SR._hhcc_hash 			= store [line_cnt + 1]._hhcc_hash;
		SR._qty 				= store [line_cnt + 1]._qty;
		SR._cnv_fct				= store [line_cnt + 1]._cnv_fct;
		strcpy (SR._uom			, store [line_cnt + 1]._uom);
		strcpy (SR._uom_group	, store [line_cnt + 1]._uom_group);
		strcpy (SR._lot_ctrl	, store [line_cnt + 1]._lot_ctrl);
		getval (line_cnt + 1);
		putval (line_cnt);
	
		if (this_page == line_cnt / TABLINES)
			line_display ();
	}

	if (this_page == line_cnt / TABLINES)
		blank_display ();
	
	line_cnt = i;
	getval (line_cnt);
	print_at (2,0,"                   ");
	fflush (stdout);
	return (EXIT_SUCCESS);
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
 int _dec_pt)
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
		sprintf (err_str, ML (mlSkMess238), _qty, compare [_dec_pt]);
		print_mess (err_str);
		sleep (sleepTime);
		clear_mess ();
		return (FALSE);
	}

	return (TRUE);
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
	ReadCcmr ();

	abc_alias (inum2, inum);
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (insc, insc_list, INSC_NO_FIELDS, "insc_id_no");
	open_rec (inum, inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	open_rec (inum2,inum_list, INUM_NO_FIELDS, "inum_id_no2");
	open_rec (sttf, sttf_list, STTF_NO_FIELDS, 
							(envVarStExpDate) ? "sttf_id_no4" : "sttf_id_no3");
	OpenLocation (ccmr_rec.hhcc_hash);

	LL_EditLoc	=	TRUE;
	LL_EditLot	=	TRUE;
	LL_EditDate	=	TRUE;
	LL_EditSLot	=	TRUE;
	IgnoreAvailChk	=	TRUE;
	strcpy (StockTake, "Y");
	strcpy (llctAutoAll, 	"N");
	strcpy (llctAllLocs, 	"Y");
	strcpy (llctExpItems, "Y");
	strcpy (llctAltUom, 	"Y");
}

/*=========================
| Close data base files . |
=========================*/
void
CloseDB (
 void)
{
	abc_fclose (inmr);
	abc_fclose (incc);
	abc_fclose (insc);
	abc_fclose (sttf);
	abc_fclose (inum);
	abc_fclose (inum2);
	CloseLocation ();
	SearchFindClose ();
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
		file_err (cc, "ccmr", "DBFIND");

	abc_fclose (ccmr);
}

/*===============
| Search on UOM |
===============*/
void
SrchInum (
 char 	*key_val,
 int	line_cnt)
{
	work_open ();
	save_rec ("#UOM","#Description");

	strcpy (inum2_rec.uom_group, SR._uom_group);
	strcpy (inum2_rec.uom, key_val);
	cc = find_rec (inum2, &inum2_rec, GTEQ, "r");
	while (!cc &&
		   !strcmp (inum2_rec.uom_group, SR._uom_group))
	{
		if (strncmp (inum2_rec.uom, key_val, strlen (key_val)))
		{
			cc = find_rec (inum2, &inum2_rec, NEXT, "r");
			continue;
		}

		if (!ValidItemUom (SR._hhbr_hash, inum2_rec.hhum_hash))
		{
			cc = save_rec (inum2_rec.uom, inum2_rec.desc);
			if (cc)
				break;
		}

		cc = find_rec (inum2, &inum2_rec, NEXT, "r");
	}

	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (inum2_rec.uom_group, SR._uom_group);
	strcpy (inum2_rec.uom, key_val);
	cc = find_rec (inum2, &inum2_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "inum2", "DBFIND");
}

void
SrchInsc (
 void)
{
	work_open ();
	save_rec ("# ", "#Stock Take Selection Description");
	insc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	strcpy (insc_rec.stake_code, " ");
	cc = find_rec (insc, &insc_rec, GTEQ, "r");
	while (!cc && insc_rec.hhcc_hash == ccmr_rec.hhcc_hash)
	{
		cc = save_rec (insc_rec.stake_code, insc_rec.description);
		if (cc)
			break;

		cc = find_rec (insc, &insc_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;
	insc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	strcpy (insc_rec.stake_code, temp_str);
	cc = find_rec (insc, &insc_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "insc", "DBFIND");
}

void
UpdateData (
 void)
{
	float	TotalQty = 0.00;

	if (AUDIT)
		HeadAudit ();
	else
	{
		clear ();
		print_at (0,0 ,ML (mlStdMess035)); 
		fflush (stdout);
	}

	scn_set (ITEM);

	for (line_cnt = 0;line_cnt < lcount [ITEM];line_cnt++)
	{
		getval (line_cnt);

		if (FLD ("LL") == ND)
			UpdateStdStake (line_cnt, LL_SPECIAL);
		else
			UpdateStake (line_cnt);
	
		TotalQty += ( (float) (TotalLineQty));
	}
	if (AUDIT)
		TailAudit (TotalQty);
}

void
HeadAudit (
 void)
{
	dsp_screen (" Printing Stock Take Input Audit ", 
									comm_rec.co_no, comm_rec.co_name);

	if ( (fout = popen ("pformat", "w")) == 0)
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);
		
	fprintf (fout, ".START%s<%s>\n", DateToString (comm_rec.inv_date), PNAME);
	fprintf (fout, ".SO\n");
	fprintf (fout, ".LP%d\n", local_rec.lpno);
	fprintf (fout, ".PI10\n");
	fprintf (fout, ".10\n");
	fprintf (fout, ".L156\n");
	fprintf (fout, ".ESTOCK TAKE INPUT AUDIT\n");
	fprintf (fout, ".E%s %s\n", comm_rec.co_no, clip (comm_rec.co_name));
	fprintf (fout, ".E%s %s\n", comm_rec.est_no, clip (comm_rec.est_name));
	fprintf (fout, ".E%s %s\n", comm_rec.cc_no, clip (comm_rec.cc_name));
	fprintf (fout, ".EAS AT %s\n", SystemTime ());

	fprintf (fout, ".R   =================");
	fprintf (fout, "=========================================");
	fprintf (fout, "=========");
	if (MULT_LOC)
		fprintf (fout, "===========");
	fprintf (fout, "=====");
	if (SK_BATCH_CONT)
		fprintf (fout, "=========");
	fprintf (fout, "===============");
	if (envVarStExpDate)
		fprintf (fout, "============");
	fprintf (fout, "=\n");

	fprintf (fout, "   =================");
	fprintf (fout, "=========================================");
	fprintf (fout, "=========");
	if (MULT_LOC)
		fprintf (fout, "===========");
	fprintf (fout, "=====");
	if (SK_BATCH_CONT)
		fprintf (fout, "=========");
	fprintf (fout, "===============");
	if (envVarStExpDate)
		fprintf (fout, "============");
	fprintf (fout, "=\n");

	fprintf (fout, "   |  ITEM  NUMBER  ");
	fprintf (fout, "|          D E S C R I P T I O N         ");
	fprintf (fout, "| PAGE # ");
	if (MULT_LOC)
		fprintf (fout, "| LOCATION ");
	fprintf (fout, "| UOM");
	if (SK_BATCH_CONT)
		fprintf (fout, "| LOT NO.");
	fprintf (fout, "|   COUNTED    ");
	if (envVarStExpDate)
		fprintf (fout, "|EXPIRY DATE");
	fprintf (fout, "|\n");

	fprintf (fout, "   |----------------");
	fprintf (fout, "|----------------------------------------");
	fprintf (fout, "|--------");
	if (MULT_LOC)
		fprintf (fout, "|----------");
	fprintf (fout, "|----");
	if (SK_BATCH_CONT)
		fprintf (fout, "|--------");
	fprintf (fout, "|--------------");
	if (envVarStExpDate)
		fprintf (fout, "|-----------");
	fprintf (fout, "|\n");
	fflush (fout);
}

void
TailAudit (
 double	tot_qty)
{
	fprintf (fout, "   |----------------");
	fprintf (fout, "|----------------------------------------");
	fprintf (fout, "|--------");
	if (MULT_LOC)
		fprintf (fout, "|----------");
	fprintf (fout, "|----");
	if (SK_BATCH_CONT)
		fprintf (fout, "|--------");
	fprintf (fout, "|--------------");
	if (envVarStExpDate)
		fprintf (fout, "|-----------");
	fprintf (fout, "|\n");

	fprintf (fout, "   |                ");
	fprintf (fout, "                                TOTAL    ");
	fprintf (fout, "         ");
	fprintf (fout, "     ");
	if (SK_BATCH_CONT)
		fprintf (fout, "         ");
	if (MULT_LOC)
		fprintf (fout, "           ");
	sprintf (err_str, local_rec.rep_qty, tot_qty);
	fprintf (fout, " %14s", err_str);
	if (envVarStExpDate)
		fprintf (fout, "|           ");
	fprintf (fout, "|\n");

	fprintf (fout, ".EOF\n");
	pclose (fout);
}

void
tab_other (
 int line_no)
{
	DrawQuantity (line_no);
}

void
DrawBox (
 void)
{
	box (58, 18, 65, 3);
	move (59, 20);
	line (64);
	print_at (19,59,ML (mlSkMess213));
	fflush (stdout);
}

void
DrawQuantity (
 int cur_line)
{
	register	int	line_no;
	int		high_val = (line_cnt > lcount [ITEM])  ? line_cnt : lcount [ITEM];
	double	tot_qty = 0.00;

	/*-----------------------------------------------
	| Calculate the total for the tabular screen	|
	-----------------------------------------------*/
	for (line_no = 0;line_no < high_val;line_no++)
	{
		if (store [line_no]._hhbr_hash == store [cur_line]._hhbr_hash)
			tot_qty += (double) store [line_no]._qty * store [line_no]._cnv_fct;
	}
	tot_qty += (double) store [cur_line]._qty * store [line_no]._cnv_fct;

	sttf_rec.hhwh_hash = store [cur_line]._hhwh_hash;
	sttf_rec.hhum_hash = 0L;
	strcpy (sttf_rec.location, "          ");
	strcpy (sttf_rec.lot_no, "       ");
	sttf_rec.exp_date = 0L;
	cc = find_rec (sttf, &sttf_rec, GTEQ, "r");
	while (!cc && sttf_rec.hhwh_hash == store [cur_line]._hhwh_hash)
	{
		tot_qty += n_dec (sttf_rec.qty, inmr_rec.dec_pt);
		cc = find_rec (sttf, &sttf_rec, NEXT, "r");
	}
	
	sprintf (err_str, local_rec.rep_qty, n_dec (tot_qty, inmr_rec.dec_pt));
	print_at (21,59," %-16.16s |  %4.4s  |  %14s ", inmr_rec.item_no, 
											inmr_rec.sale_unit, err_str);
	fflush (stdout);
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

		rv_pr (ML (mlSkMess214), 45, 0, 1);
		move (0, 1);
		line (132);

		if (scn == 1)
		{
			box (0, 3, 132, 7);
			move (1, 8);
			line (131);
			move (0, 19);
			line (132);

			print_at (20,0,ML (mlStdMess038),comm_rec.co_no,comm_rec.co_name);

			print_at (21,0,ML (mlStdMess039),comm_rec.est_no,comm_rec.est_name);

			print_at (22,0,ML (mlStdMess099),comm_rec.cc_no,comm_rec.cc_name);
		}
		else
		{
			print_at (2,0,ML (mlSkMess239), 
							insc_rec.stake_code, insc_rec.description);

			PrintCoStuff ();
		}
		/*  reset this variable for new screen NOT page	*/
		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}

void
PrintCoStuff (
 void)
{
	move (0, 19);
	line (60);
	print_at (20,0,ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
	print_at (21,0,ML (mlStdMess039), comm_rec.est_no, comm_rec.est_name);
	print_at (22,0,ML (mlStdMess099), comm_rec.cc_no, comm_rec.cc_name);
	DrawBox ();
}

void
UpdateStake (
 int	lineCnt)
{
	int		i;

	TotalLineQty	=	0.00;

	abc_selfield ("sttf", "sttf_id_no3");

	for (i = 0; i < MAX_LOTS; i++)
	{
		if (!LL_Valid (lineCnt, i))
			break;

		if (GetQty (lineCnt, i) ==	0.00)
			continue;

		sttf_rec.hhwh_hash = GetHHWH (lineCnt, i);
		sttf_rec.hhum_hash = GetHHUM (lineCnt, i);
		strcpy (sttf_rec.location, 	GetLoc 	(lineCnt, i));
		strcpy (sttf_rec.lot_no, 	GetLotNo (lineCnt, i));
		cc = find_rec (sttf, &sttf_rec, EQUAL, "u");
		if (cc)
		{
			putchar ('A');
			fflush (stdout);
			sttf_rec.hhwh_hash 	= 		GetHHWH 	(lineCnt, i);
			sttf_rec.hhum_hash 	= 		GetHHUM 	(lineCnt, i);
			strcpy (sttf_rec.location, 	GetLoc 		(lineCnt, i));
			strcpy (sttf_rec.lot_no,   	GetLotNo 	(lineCnt, i));
			strcpy (sttf_rec.slot_no,  	GetSLotNo 	(lineCnt, i));

			sttf_rec.exp_date 	= (envVarStExpDate) ? local_rec.exp_date: 0L;
			if (sttf_rec.exp_date	== 0L)
				sttf_rec.exp_date	= StringToDate (GetExpiry (lineCnt,i));
			sttf_rec.qty 			= (float) GetBaseQty (lineCnt, i);
			sttf_rec.page_no 		= local_rec.page_no;
			strcpy (sttf_rec.stat_flag, insc_rec.stake_code);
			cc = abc_add (sttf, &sttf_rec);
			if (cc)
				file_err (cc, "sttf", "DBADD");
		}
		else
		{
			putchar ('U');
			fflush (stdout);
			sttf_rec.exp_date 	= (envVarStExpDate) ? local_rec.exp_date: 0L;
			if (sttf_rec.exp_date	== 0L)
				sttf_rec.exp_date	= StringToDate (GetExpiry (lineCnt,i));
			strcpy (sttf_rec.stat_flag, insc_rec.stake_code);
			sttf_rec.hhum_hash 	= 		GetHHUM 	(lineCnt, i);
			sttf_rec.qty 		+= (float) GetBaseQty (lineCnt, i);
			cc = abc_update (sttf, &sttf_rec);
			if (cc)
				file_err (cc, "sttf", "DBUPDATE");
		}
		if (AUDIT)
		{
			dsp_process (" Item : ", inmr_rec.item_no);

			fprintf (fout, "   |%-16.16s", inmr_rec.item_no);
			fprintf (fout, "|%-40.40s", inmr_rec.description);
			fprintf (fout, "| %6ld ", local_rec.page_no);
			if (MULT_LOC)
				fprintf (fout, "|%-10.10s", GetLoc (lineCnt, i));
			fprintf (fout, "|%-4.4s", GetLUOM (lineCnt, i));
			if (SK_BATCH_CONT)
				fprintf (fout, "|%7.7s ", GetLotNo (lineCnt, i));
			sprintf (err_str, local_rec.rep_qty, GetBaseQty (lineCnt, i));
			fprintf (fout, "|%14s", err_str);
			if (envVarStExpDate)
				fprintf (fout, "|%10.10s ", DateToString (local_rec.exp_date));
			fprintf (fout, "|\n");

			TotalLineQty += GetBaseQty (lineCnt, i);
		}
	}
}

void
UpdateStdStake (
 int	line_cnt,
 int	LLSpecial)
{
	TotalLineQty	=	0.00;

	abc_selfield ("sttf", "sttf_id_no3");

	sttf_rec.hhwh_hash = SK_HHWH_HASH;
	sttf_rec.hhum_hash = SK_HHUM_HASH;
	if (LLSpecial)
	{
		strcpy (sttf_rec.location, 	local_rec.Location);
		strcpy (sttf_rec.lot_no,	"  N/A  ");
		strcpy (sttf_rec.slot_no,	"  N/A  ");
	}
	else
	{
		strcpy (sttf_rec.location, 	"          ");
		strcpy (sttf_rec.lot_no,	"       ");
		strcpy (sttf_rec.slot_no,	"       ");
	}
	cc = find_rec (sttf, &sttf_rec, EQUAL, "u");
	if (cc)
	{
		putchar ('A');
		fflush (stdout);
		sttf_rec.hhwh_hash 	=	SK_HHWH_HASH;
		sttf_rec.hhum_hash 	=	SK_HHUM_HASH;
		if (LLSpecial)
		{
			strcpy (sttf_rec.location, 	local_rec.Location);
			strcpy (sttf_rec.lot_no,	"  N/A  ");
			strcpy (sttf_rec.slot_no,	"  N/A  ");
		}
		else
		{
			strcpy (sttf_rec.location, 	"          ");
			strcpy (sttf_rec.lot_no,	"       ");
			strcpy (sttf_rec.slot_no,	"       ");
		}
		sttf_rec.exp_date 	= (envVarStExpDate) ? local_rec.exp_date: 0L;
		sttf_rec.qty 		= (float) SK_QUANTITY * SK_CNV_FCT;
		sttf_rec.page_no 	= local_rec.page_no;
		strcpy (sttf_rec.stat_flag, insc_rec.stake_code);
		cc = abc_add (sttf, &sttf_rec);
		if (cc)
			file_err (cc, "sttf", "DBADD");
	}
	else
	{
		putchar ('U');
		fflush (stdout);
		sttf_rec.exp_date 	= (envVarStExpDate) ? local_rec.exp_date: 0L;
		strcpy (sttf_rec.stat_flag, insc_rec.stake_code);
		sttf_rec.hhum_hash 	= 	SK_HHUM_HASH;
		sttf_rec.qty += (float)	SK_QUANTITY * SK_CNV_FCT;
		cc = abc_update (sttf, &sttf_rec);
		if (cc)
			file_err (cc, "sttf", "DBUPDATE");
	}
	if (AUDIT)
	{
		dsp_process (" Item : ", inmr_rec.item_no);

		fprintf (fout, "   |%-16.16s", inmr_rec.item_no);
		fprintf (fout, "|%-40.40s", inmr_rec.description);
		fprintf (fout, "| %6ld ", local_rec.page_no);
		if (LLSpecial)
			fprintf (fout, "|%-10.10s", local_rec.Location);
		fprintf (fout, "|%-4.4s", SK_UOM);
		sprintf (err_str, local_rec.rep_qty, SK_QUANTITY * SK_CNV_FCT);
		fprintf (fout, "|%14s", err_str);
		if (envVarStExpDate)
			fprintf (fout, "|%10.10s ", DateToString (local_rec.exp_date));
		fprintf (fout, "|\n");

		TotalLineQty += SK_QUANTITY * SK_CNV_FCT;
	}
}

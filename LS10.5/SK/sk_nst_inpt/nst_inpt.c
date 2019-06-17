/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: nst_inpt.c,v 5.9 2002/11/26 03:24:34 keytan Exp $
|  Program Name  : (sk_nst_inpt.c)                                    |
|  Program Desc  : (Stock Take Input.                        )        |
|---------------------------------------------------------------------|
|  Author        : Roger Gibbison  | Date Written  : 28/03/89         |
|---------------------------------------------------------------------|
| $Log: nst_inpt.c,v $
| Revision 5.9  2002/11/26 03:24:34  keytan
| ditto.
|
| Revision 5.8  2002/11/25 05:46:55  lsl
| Updated to fix issue regarding the screen 2. the file is not set back to its BOF
|
| Revision 5.7  2002/07/24 08:39:16  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.6  2002/07/17 09:57:57  scott
| Updated to change argument to get_lpno from (1) to (0)
|
| Revision 5.5  2002/07/16 02:43:45  scott
| Updated from service calls and general maintenance.
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: nst_inpt.c,v $";
char	*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_nst_inpt/nst_inpt.c,v 5.9 2002/11/26 03:24:34 keytan Exp $";

#define	MAXWIDTH	100
#define	MAXLINES	800
#include 	<ml_std_mess.h>
#include 	<ml_sk_mess.h>
#include 	<pslscr.h>
#include	<get_lpno.h>
#include	<getnum.h>
#include	<wild_search.h>
#include	<twodec.h>
#include	<dsp_screen.h>
#include	<dsp_process.h>

#define	AUDIT			 (local_rec.audit [0] == 'Y')
#define	BY_LOCN			 (inputType [0] == 'L')
#define	BY_BOTH			 (inputType [0] == 'B')
#define	SR				store [line_cnt]
#define	LSR				store [lcount [2]]
#define	SK_HHBR_HASH	store [line_cnt].hhbrHash
#define	SK_HHUM_HASH	store [line_cnt].hhumHash
#define	SK_HHWH_HASH	store [line_cnt].hhwhHash
#define	SK_QUANTITY		store [line_cnt].quantity
#define	SK_CNV_FCT		store [line_cnt].cnvFct
#define	SK_UOM			store [line_cnt].uom
#define	LOT_ITEM		 (SK_BATCH_CONT && SR.lotControl [0] == 'Y')
#define	LL_SPECIAL		 (MULT_LOC && BY_BOTH && !SK_BATCH_CONT)

	FILE	*fout;

	char	inputType [2];

	int		workNo, 
			PID; 

	int		SK_ST_EXP_UOM;

	char	*INVAL_CLS;
 	char 	*result;

	double	tot_dsp = 0.00;

	struct	{
		long	hhwhHash;
		char	location [11];
	} wkRec;

	struct	storeRec {
		long	hhbrHash;
		long	hhwhHash;
		long	hhccHash;
		long	hhumHash;
		float	quantity;
		char	uom [5];
		char	uomGroup [21];
		float	stdCnvFct;
		float	cnvFct;
		char	lotControl [2];
		int		decPt;
	} store [MAXLINES];

#include	"schema"

struct commRecord	comm_rec;
struct inscRecord	insc_rec;
struct ccmrRecord	ccmr_rec;
struct inmrRecord	inmr_rec;
struct inccRecord	incc_rec;
struct inwuRecord	inwu_rec;
struct sttfRecord	sttf_rec;
struct inumRecord	inum_rec;
struct inumRecord	inum2_rec;
struct inloRecord	inlo_rec;

	char	*data  = "data", 
			*inum2 = "inum2";
	int		SK_ST_PFRZ = 0;

/*
 * Local & Screen Structures. 
 */
struct {
	char	dummy [11];
	char	audit [2];
	char	auditDesc [10];
	int		printerNo;
	double	qty;
	char	uom [5];
	char	location [11];
	char	dflt_qty [15];
	char	rep_qty [30];
	char	LL [2];
} local_rec, local_rec2;

extern int	TruePosition;

static	struct	var	vars [] =
{
	{
	1, LIN, "stake_code", 	 4, 2, CHARTYPE, 
		"U", "          ", 
		" ", "", "Stock Take Selection Code  ", "", 
		 NA, NO,  JUSTLEFT, "", "", insc_rec.stake_code 
	},
	{
	1, LIN, "stake_desc", 	 5, 2, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "Stock Selection Desc.      ", "", 
		 NA, NO,  JUSTLEFT, "", "", insc_rec.description
	},
	{
	1, LIN, "stake_date", 	 6, 2, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", "", "Start Date                 ", "", 
		 NA, NO,  JUSTLEFT, "", "", (char *)&insc_rec.start_date
	},
	{
	1, LIN, "stake_time", 	 7, 2, CHARTYPE, 
		"AAAAA", "          ", 
		" ", "", "Start Time                 ", "", 
		 NA, NO,  JUSTLEFT, "", "", insc_rec.start_time
	},
	{
	1, LIN, "audit", 	 9, 2, CHARTYPE, 
		"U", "          ", 
		" ", "Y","Audit Required             ", "", 
		YES, NO,  JUSTLEFT, "YN", "", local_rec.audit
	},
	{
	1, LIN, "auditDesc", 	 9, 35, CHARTYPE, 
		"AAAAAAAAAA", "          ", 
		" ", "","", "", 
		NA, NO,  JUSTLEFT, "", "", local_rec.auditDesc
	},
	{
	1, LIN, "printerNo", 	10, 2, INTTYPE, 
		"NN", "          ", 
		" ", "1","Printer Number             ", "", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.printerNo
	},

	{
	2, TAB, "hhwhHash", 	MAXLINES, 0, LONGTYPE, 
		"NNNNNNN", "          ", 
		" ", "", " ", "", 
		 ND, NO, JUSTRIGHT, "", "", (char *)&incc_rec.hhwh_hash
	},
	{
	2, TAB, "location", 	 0, 0, CHARTYPE, 
		"UUUUUUUUUU", "          ", 
		" ", " ", " Location ", "", 
		YES, NO,  JUSTLEFT, "", "", local_rec.location 
	},
	{
	2, TAB, "item_no", 	 0, 0, CHARTYPE, 
		"UUUUUUUUUUUUUUUU", "          ", 
		" ", " ", "  Item  Number  ", "", 
		 NA, NO,  JUSTLEFT, "", "", inmr_rec.item_no
	},
	{
	2, TAB, "item_desc", 	 0, 1, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "           D e s c r i p t i o n          ", "", 
		 NA, NO,  JUSTLEFT, "", "", inmr_rec.description
	},
	{
	2, TAB, "UOM", 	 0, 1, CHARTYPE, 
		"AAAA", "          ", 
		" ", " ", " UOM. ", "Enter Input Unit of Measure [SEARCH]",
		YES, NO,  JUSTLEFT, "", "", local_rec.uom
	},
	{
	2, TAB, "qty", 	 0, 0, DOUBLETYPE, 
		local_rec.dflt_qty, "          ", 
		" ", "0.00", "  Quantity  ", "", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.qty
	},
	{
	2, TAB, "LL", 	 0, 0, CHARTYPE, 
		"U", "          ", 
		" ", " ", "L", "Select lot <return>  A(utomatic Mode) O(ne Key Mode)", 
		 YES, NO,  JUSTLEFT, "", "", local_rec.LL
	},
	{
	0, LIN, "", 	 0, 0, CHARTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy
	},
};

int		AutomaticMode	=	FALSE;
double	TotalLineQty	=	0.00;
int		lastPosition	=	0;
int     envVarStockTakeUpdate = 0;


#define     LEAVE       0
#define     ZERO_COUNT  1
#define     ZERO_ALL    2

#include	<LocHeader.h>
/*
 * Function Declarations 
 */
void 	shutdown_prog 		(void);
int  	spec_valid 			(int);
int  	LoadData 			(void);
int  	ValidQuantity 		(double, int);
int  	ValidItem 			(void);
int  	validUom 			(int);
int	 	OpenDB 				(void);
void 	CloseDB 			(void);
void 	ReadCcmr 			(void);
void 	SrchUom 			(char *, int);
void 	UpdateData 			(void);
void 	HeadAudit 			(void);
void 	TailAudit 			(double);
int  	heading 			(int);
void 	PrintCoStuff 		(void);
void 	UpdateStake 		(int);
void 	UpdateStdStake 		(int, int);

/*
 * Main Processing Routine . 
 */
int
main (
 int argc, 
 char * argv [])
{
	int		i;
	char	*sptr;
			
	int		after,
			before;

	TruePosition	=	TRUE;

	if (argc != 2)
	{
		print_at (0,0,mlStdMess046, argv [0]);
		return (EXIT_FAILURE);
	}
	PID = atoi (argv [1]);

	sptr = chk_env ("SK_QTY_MASK");
	if (sptr == (char *)0)
		strcpy (local_rec.dflt_qty, "NNNNNNN.NNNNNN");
	else
		strcpy (local_rec.dflt_qty, sptr);
	before = strlen (local_rec.dflt_qty);
	sptr = strrchr (local_rec.dflt_qty, '.');
	if (sptr)
		after = (int) ((sptr + strlen (sptr) - 1) - sptr);
	else
		after = 0;
	if (after == 0)
		sprintf (local_rec.rep_qty, "%%%df", before);
	else
		sprintf (local_rec.rep_qty, "%%%d.%df", before, after);

	sptr = chk_env ("SK_IVAL_CLASS");
	if (sptr)
		INVAL_CLS = strdup (sptr);
	else
		INVAL_CLS = "ZKPN";

	sptr = chk_env ("SK_ST_EXP_UOM");
	SK_ST_EXP_UOM	= (sptr != (char *)0) ? atoi (sptr) : FALSE;

    envVarStockTakeUpdate       = atoi (get_env ("ST_UPZERO"));

	upshift (INVAL_CLS); 

	SETUP_SCR (vars);


	tab_row = 3;

	tab_col = 10;

	sptr = chk_env ("SK_ST_PFRZ");
	if (sptr != (char *)0)
		SK_ST_PFRZ = atoi (sptr);

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

	if (OpenDB ())
        return (EXIT_SUCCESS);

	FLD ("LL") = ND;
	if (SK_BATCH_CONT || MULT_LOC)
		FLD ("LL") = YES;

	cc = RF_READ (workNo, (char *) &wkRec);
	if (cc)
		sys_err ("Error in sksort During (WKREAD)", cc, PNAME);

	/*---------------------------------------
	| Validate Stock Take Control Record	|
	---------------------------------------*/
	sprintf (inputType, "%-1.1s", wkRec.location);
	sprintf (insc_rec.stake_code, "%-1.1s", wkRec.location + 1);
	insc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	cc = find_rec (insc, &insc_rec, COMPARISON, "r");
	if (cc)
	{
		sprintf (err_str, "Error in insc %s during (DBFIND) ", 
							insc_rec.stake_code);
		sys_err (err_str, cc, PNAME);
	}
	if (LL_SPECIAL)
		FLD ("LL") = ND;

	/*-----------------------
	| Locations within item	|
	-----------------------*/
	if (BY_LOCN || BY_BOTH)
		FLD ("location") = NA;
	else
		FLD ("location") = ND;
	
	if (FLD ("LL") == ND)
	{
		FLD ("UOM")		 = YES;
		FLD ("qty")		 = YES;
	}
	else
	{
		FLD ("UOM")		 = ND;
		FLD ("qty")		 = ND;
	}
	if (SK_ST_EXP_UOM)
		FLD ("UOM")	=	NA;

	lcount [2] = 0;
	search_ok = 1;
	init_ok = 0;

	heading (1);
	scn_display (1);
	entry (1);
    if (restart || prog_exit) {
		shutdown_prog ();
        return (EXIT_SUCCESS);
    }

	heading (1);
	scn_display (1);
	edit (1);
    if (restart) {
		shutdown_prog ();
        return (EXIT_SUCCESS);
    }


	while (prog_exit == 0)
	{
		incc_rec.hhwh_hash	=	0L;
		strcpy (local_rec.location, "          ");
		strcpy (inmr_rec.item_no, "                ");
		strcpy (inmr_rec.description, " ");
		strcpy (local_rec.uom, " ");
		strcpy (local_rec.LL, " ");
		local_rec.qty	=	0.00;
		for (i = 0; i < MAXLINES; i++)
			memset ((char *) &SR, '\0', sizeof (struct storeRec));
		
		lastPosition	=	0;
		cc = LoadData ();
		if (cc && lcount [2] == 0)
			break;

		heading (2);
		scn_display (2);
		entry (2);
		if (restart)
			break;

		FLD ("LL") = ND;
		if ((SK_BATCH_CONT || MULT_LOC) && !LL_SPECIAL)
			FLD ("LL") = YES;

		AutomaticMode	=	FALSE;

		heading (2);
		scn_display (2);
		edit (2);

		if (restart)
			break;

		UpdateData ();

		prog_exit = 0;
	}
	shutdown_prog ();
	return (EXIT_SUCCESS);
}

void
shutdown_prog (
 void)
{
	CloseDB (); 
	FinishProgram ();
}

int
spec_valid (
 int field)
{
	int		TempLine;

	if (LCHECK ("audit")) 
	{
		strcpy (local_rec.auditDesc, (AUDIT) ? ML ("Y(es") : ML ("N(o "));
		DSP_FLD ("auditDesc");
		FLD ("printerNo") = (AUDIT) ? YES : NA;
	}

	if (LCHECK ("printerNo")) 
	{
		if (F_NOKEY (label ("printerNo")))
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			local_rec.printerNo = get_lpno (0);
			return (EXIT_SUCCESS);
		}

		if (!valid_lp (local_rec.printerNo))
		{
			print_mess (ML (mlStdMess020));
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("hhwhHash"))
	{
		if (AutomaticMode)
			FLD ("LL") = NA;
		getval (line_cnt);
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("item_no"))
	{
		int	item_ok;
		if (FIELD.required == ND || FIELD.required == NA)
			return (EXIT_SUCCESS);

		item_ok = ValidItem ();

		return (item_ok);
	}

	/*
	 * Validate Unit of Measure 
	 */
	if (LCHECK ("UOM"))
	{
		if (F_NOKEY (label ("UOM")) || dflt_used)
			strcpy (local_rec.uom, SR.uom);

		if (SRCH_KEY)
		{
			SrchUom (temp_str, line_cnt);
			return (EXIT_SUCCESS);
		}

		strcpy (inum2_rec.uom_group, SR.uomGroup);
		strcpy (inum2_rec.uom, local_rec.uom);
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
			if (SR.hhumHash == inum2_rec.hhum_hash)
				AddINUV (SR.hhbrHash, SR.hhumHash);	

			if (ValidItemUom (SR.hhbrHash, inum2_rec.hhum_hash))
			{
				print_mess (ML (mlStdMess028));
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}

		strcpy (local_rec.uom, inum2_rec.uom);
		SR.hhumHash = inum2_rec.hhum_hash;
		strcpy (SR.uom, inum2_rec.uom);
        if (inum2_rec.cnv_fct == 0.00)
             inum2_rec.cnv_fct = 1.00;

        SR.cnvFct = inum2_rec.cnv_fct / SR.stdCnvFct;
		strcpy (SR.uom, inum2_rec.uom);
		strcpy (SR.uomGroup, inum2_rec.uom_group);

		if (LL_SPECIAL)
		{
			local_rec.qty 	=	SR.quantity;
			DSP_FLD ("qty");
		}
		DSP_FLD ("UOM");
		return (EXIT_SUCCESS);
	}

	/*------------------------
	| Qty validation lookup. |
	------------------------*/
	if (LCHECK ("qty"))
	{
		if (dflt_used)
			local_rec.qty 	=	SR.quantity;
		DSP_FLD ("qty");

		if (!ValidQuantity (local_rec.qty, SR.decPt))
		{
			errmess (ML (mlStdMess190));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		if (local_rec.qty < 0.00)
		{
			errmess (ML (mlStdMess190));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		SR.quantity = (float) local_rec.qty;

		DSP_FLD ("qty");

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

		if (local_rec.LL [0] == 'O')
			FLD ("LL") = NI;

		rv_pr 
		 (
			"->", 
			tab_col - 2, 
			tab_row + 2 + (line_cnt % TABLINES),
			TRUE
		);
		lastPosition	=	tab_row + 2 + (line_cnt % TABLINES),

		TempLine	=	lcount [2];
		cc = DspLLStake
			 (										/*----------------------*/
				line_cnt,							/*	Line number.		*/
				tab_row + 3 + (line_cnt % TABLINES),/*  Row for window		*/
				tab_col + 1,						/*  Col for window		*/
				4,									/*  length for window	*/
				SR.hhwhHash, 						/*	Warehouse hash.		*/
				 (!SK_ST_EXP_UOM)
				? 0L : SR.hhumHash,				/*	UOM hash			*/
				SR.hhccHash,						/*	CC hash.			*/
				SR.cnvFct,
				SR.uom,							/* UOM					*/
				TodaysDate (),						/* Expiry Date.			*/
				 (BY_LOCN || BY_BOTH)
				? local_rec.location : NULL,		/* Input Mode.			*/
				SR.lotControl,						/* Lot controled item. 	*/
				AutomaticMode						/* Set to Silent Mode.	*/
													/*----------------------*/
			);
		/*-----------------
		| Redraw screens. |
		-----------------*/
		if (AutomaticMode)
			strcpy (local_rec.LL, "A");

		putval (line_cnt);

		if (!AutomaticMode)
		{
			lcount [2] = (line_cnt + 1 > lcount [2]) ? line_cnt + 1 : lcount [2];
			scn_write (2);
			scn_display (2);
			lcount [2] = TempLine;
			PrintCoStuff ();
		}
		if (cc)
		{
			FLD ("LL") = YES;
			return (EXIT_FAILURE);
		}
		if (local_rec.LL [0] == 'A' && prog_status == ENTRY)
			AutomaticMode	=	TRUE;

		rv_pr 
		 (
			"  ", 
			tab_col - 2, 
			lastPosition,
			FALSE
		);
		DSP_FLD ("LL");
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*
 * Load next n records from work file	
 */
int
LoadData (void)
{
	print_at (2,0,ML (mlStdMess035));

	lcount [2] = 0;

	abc_selfield (incc, "incc_hhwh_hash");
	abc_selfield (inmr, "inmr_hhbr_hash");

	scn_set (2);

	cc = RF_READ (workNo, (char *) &wkRec);
	while (!cc)
	{
		incc_rec.hhwh_hash	=	wkRec.hhwhHash;
		cc = find_rec (incc, &incc_rec, COMPARISON, "r");
		if (cc)
		{
			cc = RF_READ (workNo, (char *) &wkRec);
			continue;
		}
		inmr_rec.hhbr_hash	=	incc_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		if (cc)
		{
			cc = RF_READ (workNo, (char *) &wkRec);
			continue;
		}
		sprintf (local_rec.location, "%-10.10s", wkRec.location);

		inum_rec.hhum_hash = inmr_rec.std_uom;
		cc = find_rec (inum, &inum_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess028));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		if (inum_rec.cnv_fct == 0.00)
			inum_rec.cnv_fct	=	1.00;
		
		LSR.hhbrHash 	= inmr_rec.hhbr_hash;
		LSR.hhwhHash 	= incc_rec.hhwh_hash;
		LSR.hhccHash 	= incc_rec.hhcc_hash;
		LSR.stdCnvFct 	= inum_rec.cnv_fct;
        LSR.cnvFct 	= 1.00;
        LSR.decPt 	= inmr_rec.dec_pt;
		strcpy (LSR.lotControl, inmr_rec.lot_ctrl);
		strcpy (LSR.uom, inum_rec.uom);
		strcpy (local_rec.uom, inum_rec.uom);
		strcpy (LSR.uomGroup, inum_rec.uom_group);
		local_rec.qty = 0.00;
		LSR.quantity		=	0.00;
		if (LL_SPECIAL)
		{
			inlo_rec.qty = 0.00;

			cc =	FindLocation
					(
						LSR.hhwhHash,
						inum_rec.hhum_hash,
						local_rec.location,
						ValidLocations,
						&inlo_rec.inlo_hash
					);
			if (!cc)
			{
				cc = find_rec (inlo, &inlo_rec, EQUAL, "r");
				if (cc)
					file_err (cc, (char *)inlo, "DBFIND");
			}
			LSR.quantity = (float) ((inlo_rec.qty < 0.00) ? 0.00 : inlo_rec.qty);
			local_rec.qty = (float) ((inlo_rec.qty < 0.00) ? 0.00 : inlo_rec.qty);
		}

		/*--------------------------------------------------
		| This will create one tabular entry for each UOM. |
		--------------------------------------------------*/
		if (SK_ST_EXP_UOM)
		{
			inwu_rec.hhwh_hash	=	incc_rec.hhwh_hash;
			inwu_rec.hhum_hash	=	0L;
			cc = find_rec (inwu, &inwu_rec, GTEQ, "r");
			while (!cc && inwu_rec.hhwh_hash ==	incc_rec.hhwh_hash)
			{
				inum_rec.hhum_hash = inwu_rec.hhum_hash;
				cc = find_rec (inum, &inum_rec, EQUAL, "r");
				if (cc)
				{
					cc = find_rec (inwu, &inwu_rec, NEXT, "r");
					continue;
				}
				strcpy (LSR.uom, inum_rec.uom);
				strcpy (local_rec.uom, inum_rec.uom);
				strcpy (LSR.uomGroup, inum_rec.uom_group);
				LSR.hhumHash = inum_rec.hhum_hash;
        		if (inum_rec.cnv_fct == 0.00)
					inum_rec.cnv_fct = 1.00;

        		LSR.cnvFct = inum_rec.cnv_fct / LSR.stdCnvFct;

				if (LL_SPECIAL)
				{
					inlo_rec.qty	=	0.00;
					cc =	FindLocation
							(
								LSR.hhwhHash,
								inum_rec.hhum_hash,
								local_rec.location,
								ValidLocations,
								&inlo_rec.inlo_hash
							);
					if (!cc)
					{
						cc = find_rec (inlo, &inlo_rec, EQUAL, "r");
						if (cc)
							file_err (cc, (char *)inlo, "DBFIND");
					}
					if (inlo_rec.qty < 0.00)
					{
						LSR.quantity		=	 (float) 0.00;
						local_rec.qty	=	 (float) 0.00;
					}
					else
					{
						LSR.quantity		=	inlo_rec.qty;
						local_rec.qty	=	inlo_rec.qty;
					}
				}

				putval (lcount [2]++);

				LSR.hhbrHash = store [lcount [2] - 1].hhbrHash;
				LSR.hhwhHash = store [lcount [2] - 1].hhwhHash;
				LSR.hhccHash = store [lcount [2] - 1].hhccHash;
				LSR.stdCnvFct = store [lcount [2] - 1].stdCnvFct;
				strcpy (LSR.lotControl, store [lcount [2] - 1].lotControl);
				putval (lcount [2]);

				cc = find_rec (inwu, &inwu_rec, NEXT, "r");
			}
		}
		else
		{
			if (LL_SPECIAL)
			{
				cc =	FindLocation
						(
							LSR.hhwhHash,
							inum_rec.hhum_hash,
							local_rec.location,
							ValidLocations,
							&inlo_rec.inlo_hash
							);
				if (!cc)
				{
					cc = find_rec (inlo, &inlo_rec, EQUAL, "r");
					if (cc)
						file_err (cc, (char *)inlo, "DBFIND");
				}
				LSR.quantity		=	 (cc) ? (float) 0.00 : inlo_rec.qty;
				local_rec.qty	=	 (cc) ? (float) 0.00 : inlo_rec.qty;
			}
			putval (lcount [2]++);
		}
		
		if (lcount [2] % 10 == 0)
		{
			putchar ('R');
			fflush (stdout);
		}
		if ((lcount [2] + 10) >= MAXLINES)
			break;

		cc = RF_READ (workNo, (char *) &wkRec);
	}

	abc_selfield (incc, "incc_id_no");
	abc_selfield (inmr, "inmr_id_no");


	vars [scn_start].row = lcount [2];
	move (0, 2);
	cl_line ();
	fflush (stdout);
	return (cc);
}
/*--------------------------------------------
| Checks if the quantity entered by the user |
| valid quantity that can be saved to a      |
| float variable without any problems of     |
| losing figures after the decimal point.    |
| eg. if decPt is 2 then the greatest      |
| quantity the user can enter is 99999.99    |
--------------------------------------------*/
int
ValidQuantity (
 double _qty, 
 int decPt)
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

	if (_qty > compare [decPt])
	{
		sprintf (err_str, ML (mlSkMess238), _qty, compare [decPt]);
		print_mess (err_str);
		sleep (sleepTime);
		clear_mess ();
		return (FALSE);
	}

	return (TRUE);
}

/*
 * Validate Item Number	
 */
int
ValidItem (void)
{
	if (SRCH_KEY)
	{
		InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
		return (EXIT_SUCCESS);
	}
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
		return (EXIT_FAILURE);
	}
	SuperSynonymError ();
	
	result = strstr (INVAL_CLS, inmr_rec.inmr_class);
	if (result != (char*) NULL) 
	{
		sprintf (err_str, ML (mlSkMess211), inmr_rec.item_no);
		print_mess (err_str);
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
	   (incc_rec.stat_flag [0] != '0' || !SK_ST_PFRZ))
	{
		sprintf (err_str, ML (mlSkMess454), incc_rec.stat_flag);
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
	LSR.hhbrHash = inmr_rec.hhbr_hash;
	LSR.hhwhHash = incc_rec.hhwh_hash;
	LSR.hhccHash = incc_rec.hhcc_hash;
	LSR.stdCnvFct = inum_rec.cnv_fct;
	strcpy (LSR.uom, inum_rec.uom);
	strcpy (LSR.uomGroup, inum_rec.uom_group);
	strcpy (LSR.lotControl, inmr_rec.lot_ctrl);

	return (EXIT_SUCCESS);
}

/*
 * Validate Unit of Measure	
 */
int
validUom (
	int	line_cnt)
{
	if (SRCH_KEY)
	{
		SrchUom (temp_str, line_cnt);
		return (EXIT_SUCCESS);
	}
	sprintf (local_rec.uom, "%-4.4s", temp_str);

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
	LSR.stdCnvFct = inum_rec.cnv_fct;
	strcpy (LSR.uom, inum_rec.uom);
	strcpy (local_rec.uom, inum_rec.uom);
	strcpy (LSR.uomGroup, inum_rec.uom_group);
	return (EXIT_SUCCESS);
}

/*
 * Open data base files 
 */
int
OpenDB (void)
{
	char	*sptr = getenv ("PROG_PATH");
	char	filename [101];

	sprintf (filename, "%s/WORK/sksort%05d", (sptr == (char *)0) ? "/usr/LS10.5" : sptr, PID);
	cc = RF_OPEN (filename, sizeof (wkRec), "r", &workNo);
	if (cc)
		return (EXIT_FAILURE);

	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	ReadCcmr ();

	abc_alias (inum2,inum);
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (inwu, inwu_list, INWU_NO_FIELDS, "inwu_id_no");
	open_rec (insc, insc_list, INSC_NO_FIELDS, "insc_id_no");
	open_rec (inum, inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	open_rec (inum2, inum_list, INUM_NO_FIELDS, "inum_id_no2");
	open_rec (sttf, sttf_list, STTF_NO_FIELDS, "sttf_id_no3");
	open_rec (inlo, inlo_list, INLO_NO_FIELDS, "inlo_inlo_hash");


	OpenLocation (ccmr_rec.hhcc_hash);

	LL_EditLoc	=	 (MULT_LOC) ? TRUE : FALSE; 
	LL_EditLot	=	 (SK_BATCH_CONT) ? TRUE : FALSE;
	LL_EditDate	=	 (SK_BATCH_CONT) ? TRUE : FALSE;
	LL_EditSLot	=	 (SK_BATCH_CONT) ? TRUE : FALSE;

	IgnoreAvailChk	=	TRUE;
	strcpy (llctAutoAll, "N");
	return (EXIT_SUCCESS);
}

/*
 * Close data base files 
 */
void
CloseDB (void)
{
	cc = RF_DELETE (workNo);
	if (cc)
		sys_err ("Error in sksort During (WKDEL)", cc, PNAME);

	abc_fclose (inmr);
	abc_fclose (incc);
	abc_fclose (insc);
	abc_fclose (inum);
	abc_fclose (inum2);
	abc_fclose (inlo);
	abc_fclose (sttf);
	CloseLocation ();
	SearchFindClose ();
	abc_dbclose (data);
}

void
ReadCcmr (void)
{
	open_rec (ccmr, ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, comm_rec.est_no);
	strcpy (ccmr_rec.cc_no, comm_rec.cc_no);
	cc = find_rec (ccmr, &ccmr_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, (char *)ccmr, "DBFIND");

	abc_fclose (ccmr);
}

/*
 * Search on UOM 
 */
void
SrchUom (
	char 	*key_val,
	int		line_cnt)
{
	_work_open (4,0,40);
	save_rec ("#UOM","#Description");

	strcpy (inum2_rec.uom_group, SR.uomGroup);
	strcpy (inum2_rec.uom, key_val);
	cc = find_rec (inum2, &inum2_rec, GTEQ, "r");
	while (!cc &&
		   !strcmp (inum2_rec.uom_group, SR.uomGroup))
	{
		if (strncmp (inum2_rec.uom, key_val, strlen (key_val)))
		{
			cc = find_rec (inum2, &inum2_rec, NEXT, "r");
			continue;
		}

		if (!ValidItemUom (SR.hhbrHash, inum2_rec.hhum_hash))
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

	strcpy (inum2_rec.uom_group, SR.uomGroup);
	strcpy (inum2_rec.uom, key_val);
	cc = find_rec (inum2, &inum2_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, inum2, "DBFIND");
}
	
/*
 * Update Stock Take Transaction File	
 */
void
UpdateData (void)
{
	float	TotalQty	=	0.00;

	if (AUDIT)
		HeadAudit ();
	else
	{
		clear ();
		print_at (0,0, ML (mlSkMess496));
		fflush (stdout);
	}
	scn_set (2);

	for (line_cnt = 0;line_cnt < lcount [2];line_cnt++)
	{
		getval (line_cnt);
	
		if (FLD ("LL") == ND)
			UpdateStdStake (line_cnt, LL_SPECIAL);
		else
			UpdateStake (line_cnt);
		
		TotalQty += ((float) (TotalLineQty));
	}
	if (AUDIT)
		TailAudit (TotalQty);
}

void
HeadAudit (void)
{
	dsp_screen (" Printing Stock Take Input Audit ", 
									comm_rec.co_no, comm_rec.co_name);

	if ((fout = popen ("pformat", "w")) == 0)
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);
		
	fprintf (fout, ".START%s<%s>\n", DateToString (comm_rec.inv_date), PNAME);
	fprintf (fout, ".SO\n");
	fprintf (fout, ".LP%d\n", local_rec.printerNo);
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
	if (MULT_LOC)
		fprintf (fout, "===========");
	fprintf (fout, "=====");
	if (SK_BATCH_CONT)
		fprintf (fout, "=========");
	fprintf (fout, "============");
	fprintf (fout, "================\n");

	fprintf (fout, "   =================");
	fprintf (fout, "=========================================");
	if (MULT_LOC)
		fprintf (fout, "===========");
	fprintf (fout, "=====");
	if (SK_BATCH_CONT)
		fprintf (fout, "=========");
	fprintf (fout, "============");
	fprintf (fout, "================\n");

	fprintf (fout, "   |  ITEM  NUMBER  ");
	fprintf (fout, "|          D E S C R I P T I O N         ");
	if (MULT_LOC)
		fprintf (fout, "| LOCATION ");
	fprintf (fout, "| UOM");
	if (SK_BATCH_CONT)
		fprintf (fout, "| LOT NO.");
	fprintf (fout, "|   COUNTED    ");
	fprintf (fout, "|EXPIRY DATE");
	fprintf (fout, "|\n");

	fprintf (fout, "   |----------------");
	fprintf (fout, "|----------------------------------------");
	if (MULT_LOC)
		fprintf (fout, "|----------");
	fprintf (fout, "|----");
	if (SK_BATCH_CONT)
		fprintf (fout, "|--------");
	fprintf (fout, "|--------------");
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
	if (MULT_LOC)
		fprintf (fout, "|----------");
	fprintf (fout, "|----");
	if (SK_BATCH_CONT)
		fprintf (fout, "|--------");
	fprintf (fout, "|--------------");
	fprintf (fout, "|-----------");
	fprintf (fout, "|\n");

	fprintf (fout, "   | TOTAL          ");
	fprintf (fout, "|                                        ");
	if (MULT_LOC)
		fprintf (fout, "|          ");
	fprintf (fout, "|    ");
	if (SK_BATCH_CONT)
		fprintf (fout, "|        ");
	sprintf (err_str, local_rec.rep_qty, tot_qty);
	fprintf (fout, "|%14s", err_str);
	fprintf (fout, "|           ");
	fprintf (fout, "|\n");

	fprintf (fout, ".EOF\n");
	pclose (fout);
}

int
heading (
 int scn)
{
	int		scnWidth;

	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);

		if (scn == 1)
		{
			snorm ();
			scnWidth = 80;
		}
		else
		{
			swide ();
			scnWidth = 132;
		}

		clear ();

		rv_pr (ML (mlSkMess214), (int) ((scnWidth - 18) / 2), 0, 1);
		move (0, 1);

		if (scn == 1)
		{
			box (0, 3, scnWidth, 7);
			line_at (1,0,  scnWidth);
			line_at (8,1,  scnWidth - 1);
			line_at (19,0, scnWidth - 1);
			PrintCoStuff ();
		}
		else
		{
			line_at (1,0, scnWidth -1);
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
	line (132);
	strcpy (err_str,ML (mlStdMess038));
	print_at (20,0,err_str,comm_rec.co_no, comm_rec.co_name);
	strcpy (err_str,ML (mlStdMess039));
	print_at (21,0,err_str, comm_rec.est_no, comm_rec.est_name);
	strcpy (err_str,ML (mlStdMess099));
	print_at (22,0,err_str,comm_rec.cc_no, comm_rec.cc_name);
}

void
UpdateStake (
 int	lineCnt)
{
	int		i;
	int		PrintItemLine	=	TRUE;

	TotalLineQty	=	0.00;


	abc_selfield (sttf, "sttf_id_no3");

	PrintItemLine	=	TRUE;
	for (i = 0; i < MAX_LOTS; i++)
	{
		if (!GetHHWH (lineCnt,i))
			break;

		if (GetQty (lineCnt, i) ==  0.00 &&
			!strncmp (GetLoc (lineCnt, i), "          ", 10))
			continue;

		/*---------------------------------------------
		|  Everything else gets written to sttf ???   |
		|  Should it be only those that were changed? |
		|  ???????????????????????????????????????    |
		---------------------------------------------*/
		if (envVarStockTakeUpdate == ZERO_COUNT ||
			envVarStockTakeUpdate == ZERO_ALL)
		{
			if (GetQty (lineCnt, i) == GetAvailQty (lineCnt, i))
				continue;
		}

		sttf_rec.hhwh_hash = GetHHWH (lineCnt, i);
		sttf_rec.hhum_hash = GetHHUM (lineCnt, i);
		strcpy (sttf_rec.location, 	GetLoc (lineCnt, i));
		strcpy (sttf_rec.lot_no, 	GetLotNo (lineCnt, i));
		cc = find_rec (sttf, &sttf_rec, EQUAL, "u");
		if (cc)
		{
			sttf_rec.hhwh_hash 	= 		GetHHWH 	 (lineCnt,i);
			sttf_rec.hhum_hash 	= 		GetHHUM 	 (lineCnt,i);
			strcpy (sttf_rec.location, 	GetLoc  	 (lineCnt,i));
			strcpy (sttf_rec.lot_no,   	GetLotNo	 (lineCnt,i));
			strcpy (sttf_rec.slot_no,  	GetSLotNo	 (lineCnt, i));
			sttf_rec.exp_date	=		StringToDate (GetExpiry (lineCnt, i));
			sttf_rec.qty 		= (float) GetBaseQty (lineCnt, i);
			sttf_rec.page_no 	= (lineCnt + 1) % 60;
			strcpy (sttf_rec.stat_flag, insc_rec.stake_code);
			cc = abc_add (sttf, &sttf_rec);
			if (cc)
				file_err (cc, (char *)sttf, "DBADD");
		}
		else
		{
			sttf_rec.exp_date	=	StringToDate (GetExpiry (lineCnt, i));
			strcpy (sttf_rec.stat_flag, insc_rec.stake_code);
			sttf_rec.hhum_hash 	= 	GetHHUM (lineCnt, i);
			sttf_rec.qty += (float) GetBaseQty (lineCnt, i);
			cc = abc_update (sttf, &sttf_rec);
			if (cc)
				file_err (cc, (char *)sttf, "DBUPDATE");
			abc_unlock (sttf);
		}
		if (AUDIT)
		{
			dsp_process (" Item : ", inmr_rec.item_no);

			if (PrintItemLine)
			{
				fprintf (fout, "   |%-16.16s", inmr_rec.item_no);
				fprintf (fout, "|%-40.40s", inmr_rec.description);
			}
			else
			{
				fprintf (fout, "   |%16.16s", " ");
				fprintf (fout, "|%40.40s", 	  " ");
			}
			PrintItemLine	=	FALSE;

			if (MULT_LOC)
				fprintf (fout, "|%-10.10s", GetLoc (lineCnt, i));
			fprintf (fout, "|%-4.4s", GetLUOM (lineCnt, i));
			fprintf (fout, "|%-8.8s", GetLotNo  (lineCnt, i));
			sprintf (err_str, local_rec.rep_qty, GetBaseQty (lineCnt, i));
			fprintf (fout, "|%14s", err_str);
			fprintf (fout, "|%10.10s ", GetExpiry (lineCnt, i));
			fprintf (fout, "|\n");

			TotalLineQty += GetBaseQty (lineCnt, i);
		}
	}
}

void
UpdateStdStake (
	int		line_cnt,
	int		LLSpecial)
{
	TotalLineQty	=	0.00;

	abc_selfield (sttf, "sttf_id_no3");

	sttf_rec.hhwh_hash = SK_HHWH_HASH;
	sttf_rec.hhum_hash = SK_HHUM_HASH;
	if (LLSpecial)
	{
		strcpy (sttf_rec.location, local_rec.location);
		strcpy (sttf_rec.lot_no,  "  N/A  ");
		strcpy (sttf_rec.slot_no, "  N/A  ");
	}
	else
	{
		strcpy (sttf_rec.location, "          ");
		strcpy (sttf_rec.lot_no,   "       ");
		strcpy (sttf_rec.slot_no,  "       ");
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
			strcpy (sttf_rec.location, local_rec.location);
			strcpy (sttf_rec.lot_no,  "  N/A  ");
			strcpy (sttf_rec.slot_no, "  N/A  ");
		}
		else
		{
			strcpy (sttf_rec.location, "          ");
			strcpy (sttf_rec.lot_no,   "       ");
		}
		sttf_rec.exp_date 	= 0L;
		sttf_rec.qty 		= (float) SK_QUANTITY * SK_CNV_FCT;
		sttf_rec.page_no 	= 0;
		strcpy (sttf_rec.stat_flag, insc_rec.stake_code);
		cc = abc_add (sttf, &sttf_rec);
		if (cc)
			file_err (cc, (char *)sttf, "DBADD");
	}
	else
	{
		putchar ('U');
		fflush (stdout);
		sttf_rec.exp_date 	= 0L;
		strcpy (sttf_rec.stat_flag, insc_rec.stake_code);
		sttf_rec.hhum_hash 	= 	SK_HHUM_HASH;
		sttf_rec.qty += (float)	SK_QUANTITY * SK_CNV_FCT;
		cc = abc_update (sttf, &sttf_rec);
		if (cc)
			file_err (cc, (char *)sttf, "DBUPDATE");
	}
	if (AUDIT)
	{
		dsp_process (" Item : ", inmr_rec.item_no);

		fprintf (fout, "   |%-16.16s", inmr_rec.item_no);
		fprintf (fout, "|%-40.40s", inmr_rec.description);
		if (LLSpecial)
			fprintf (fout, "|%-10.10s", local_rec.location);
		fprintf (fout, "|%-4.4s", SK_UOM);
		sprintf (err_str, local_rec.rep_qty, SK_QUANTITY * SK_CNV_FCT);
		fprintf (fout, "|%14s", err_str);
		fprintf (fout, "|           ");
		fprintf (fout, "|\n");

		TotalLineQty += SK_QUANTITY * SK_CNV_FCT;
	}
}

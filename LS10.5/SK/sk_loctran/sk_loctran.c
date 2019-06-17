/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: sk_loctran.c,v 5.7 2002/07/24 08:39:14 scott Exp $
|  Program Name  : (sk_loctran.c    )                                 |
|  Program Desc  : (Multiple Location Transfers.                )     |
|---------------------------------------------------------------------|
|  Author        : Bee Chwee Lim.  | Date Written  : 05/01/89         |
|---------------------------------------------------------------------|
| $Log: sk_loctran.c,v $
| Revision 5.7  2002/07/24 08:39:14  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.6  2002/06/26 05:48:49  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.5  2002/06/20 07:11:02  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.4  2002/02/27 03:28:28  scott
| S/C - 00876
| Environment: Linux-Informix Server / HP-Oracle Server / Windows Client
| Program Description = SKMR2 - Warehouse Stock Status Display
| Priority = High
| Worksheet = Standard / CITIC
| Description of the Issue: Trans-All display shows Stock Transfers in HD UOM as if EA UOM when all other displays are correct.  (e.g., A 1 HD transfer is reflected as a 0.01 HD transfer in Trans-All)
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: sk_loctran.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/SK/sk_loctran/sk_loctran.c,v 5.7 2002/07/24 08:39:14 scott Exp $";

#define	NOHDLINES	4 

#define MAXWIDTH	150
#define	MAXLINES	50
#define	SR			store [line_cnt]

#include <pslscr.h>
#include <twodec.h>
#include <ml_std_mess.h>
#include <ml_sk_mess.h>

	FILE	*fout;

	int		printerNumber 	 = 1, 
			lpsw 			 = FALSE,
			lotBatchTransfer = TRUE;
	
#include	"schema"

struct commRecord	comm_rec;
struct ccmrRecord	ccmr_rec;
struct inmrRecord	inmr_rec;
struct inccRecord	incc_rec;
struct inwuRecord	inwu_rec;
struct inumRecord	inum_rec;
struct inumRecord	inum2_rec;
struct inloRecord	inlo_rec;

#include	<MoveRec.h>

	char	*data 	= "data",
			*inum2 	= "inum2";

	int		envVarSkLocTrans = 0;
	int		envSkGrinNoPlate = 0;


struct storeRec {
	long	hhbrHash;
	long	hhwhHash;
	long	hhumHash;
	char	inmrClass 	[2];
	char	category 	[12];
	char	lotCtrl 	[2];
	char	uom 		[5];
	char	uomGroup 	[21];
	double  qty;
	float	convFct;
	float	stdCnvFct;
	int		decPt;
} store [MAXLINES];

/*
 * Local & Screen Structures 
 */
struct {
	char	dummy 			[11];
	char	item_no 		[17];
	char	item_desc 		[41];
	char	uom 			[5];
	char	locationFrom 	[11];
	char	locationTo 		[11];
	char	LL 				[2];
	char	uomPrompt 		[31];
	char	uomHelp	  		[81];
	char	batchString 	[8];
	float	qty;
} local_rec;

static	struct	var	vars [] =
{
	{1, TAB, "itemNo", 	MAXLINES, 1, CHARTYPE, 
		"UUUUUUUUUUUUUUUU", "          ", 
		" ", "", "     Item no      ", " ", 
		 NE, NO, JUSTLEFT, "", "", local_rec.item_no}, 
	{1, TAB, "itemDesc", 	 0, 1, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "             Item Description             ", " ", 
		 NA, NO, JUSTLEFT, "", "", local_rec.item_desc}, 
	{1, TAB, "UOM",	 0, 2, CHARTYPE,
		"AAAA", "          ",
		" ", " ", local_rec.uomPrompt, local_rec.uomHelp,
		YES, NO,  JUSTLEFT, "", "", local_rec.uom},
	{1, TAB, "locationFrom",	 0, 0, CHARTYPE,
		"AAAAAAAAAA", "          ",
		" ", " ", "Location From", "NOTE : Use standard search for existing locations OR alternate search #1 for all available locations.",
		YES, NO,  JUSTLEFT, "", "", local_rec.locationFrom},
	{1, TAB, "qty",	 0, 0, FLOATTYPE,
		"NNNNNN.NN", "          ",
		" ", " ", "Quantity ", "Enter the quantity",
		YES, NO,  JUSTRIGHT, "", "", (char *) &local_rec.qty},
	{1, TAB, "locationTo",	 0, 0, CHARTYPE,
		"AAAAAAAAAA", "          ",
		" ", " ", "Location to", "NOTE : Use standard search for existing locations OR alternate search #1 for all available locations.",
		YES, NO,  JUSTLEFT, "", "", local_rec.locationTo},
	{1, TAB, "LL", 	 0, 0, CHARTYPE, 
		"U", "          ", 
		" ", "N", "L", "Lot / Location Selection. <return> ", 
		 YES, NO,  JUSTLEFT, "", "", local_rec.LL}, 
	{1, TAB, "hhwh_hash", 	 0, 1, LONGTYPE, 
		"NNNNNNN", "          ", 
		" ", "", "hhwh_hash", " ", 
		 ND, NO, JUSTLEFT, "", "", (char *)&incc_rec.hhwh_hash}, 
	{0, LIN, "", 	 0, 0, INTTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}, 
};

#include	<LocHeader.h>
#include	<MoveAdd.h>

/*
 * Function Declarations 
 */
float	ToStdUom	 	(float);
int  	heading 		(int);
int  	spec_valid 		(int);
void 	CloseAudit 		(void);
void 	CloseDB 		(void);
void 	OpenAudit 		(void);
void 	OpenDB 			(void);
void 	PrintCo 		(void);
void 	ReadCcmr 		(void);
void 	SrchInlo		(long, long, char *);
void 	SrchInum 		(char *, int);
void 	Update 			(void);
void 	shutdown_prog 	(void);

/*
 * Main Processing Routine. 
 */
int
main (
 int argc, 
 char * argv [])
{
	char	*sptr;

	/*
	 * Setup required parameters. 
	 */
	if (argc < 2)	
	{
		print_at (0,0,mlStdMess036, argv [0]);
		return (EXIT_FAILURE);
	}
	sptr = strrchr (argv [0], '/');
	if (sptr)
		sptr++;
	else
		sptr = argv [0];

	if (!strcmp (sptr, "sk_loctran"))
		lotBatchTransfer = FALSE;
	else
		lotBatchTransfer = TRUE;

	/*
	 * Check for Number plates.   
	 */
	sptr = chk_env ("SK_GRIN_NOPLATE");
	envSkGrinNoPlate = ((sptr == (char *)0)) ? 0 : atoi (sptr);
	if (envSkGrinNoPlate)
		lotBatchTransfer = TRUE;

	init_scr ();			/*  sets terminal from termcap	*/

	SETUP_SCR (vars);


	set_tty ();             /*  get into raw mode		*/
	set_masks ();			/*  setup print using masks	*/
/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (1, store, sizeof (struct storeRec));
#endif
	init_vars (1);			/*  set default values		*/

	printerNumber = atoi (argv [1]);

	OpenDB ();

	if (!MULT_LOC)
	{
		no_option ("MULT_LOC (Multi Bin Locations)");
		shutdown_prog ();
        return (EXIT_FAILURE);
	}

	if (SK_BATCH_CONT)
		lotBatchTransfer	=	TRUE;

	FLD	("LL")				=	(lotBatchTransfer) ? YES 	: ND;
	FLD	("locationFrom")	=	(lotBatchTransfer) ? ND 	: YES;
	FLD	("locationTo")		=	(lotBatchTransfer) ? ND 	: YES;
	FLD	("qty")				=	(lotBatchTransfer) ? ND 	: YES;
	if (lotBatchTransfer)
	{
		strcpy (local_rec.uomPrompt, "Conv.To UOM");
		strcpy (local_rec.uomHelp, 	 "Enter the Conversion to UOM. Default = standard UOM ");
	}
	else
	{
		strcpy (local_rec.uomPrompt, "  UOM  ");
		strcpy (local_rec.uomHelp, 	 "Enter the UOM. Default = standard UOM ");
	}
	swide ();

	/*
	 * Validate if transactions for location transfer required. 
	 */
	sptr = chk_env ("SK_LOC_TRANS");
	envVarSkLocTrans = (sptr == (char *)0) ? FALSE : atoi (sptr);


	tab_row = 2;
	tab_col = 10;
	strcpy (local_rec.batchString, "  N/A  ");
	while (prog_exit == 0) 
	{
		entry_exit	= FALSE;
		restart		= FALSE;
		search_ok	= TRUE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		lcount [1]	= FALSE;
		init_vars (1);

		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		/* enter edit mode */
		heading (1);
		scn_display (1);
		edit (1);
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
shutdown_prog (void)
{
	CloseAudit ();
	CloseDB (); 
	FinishProgram ();
}

/*
 * Open Data Dase Files
 */
void
OpenDB (void)
{
	MoveOpen	=	TRUE;
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);
	ReadCcmr ();

	abc_alias (inum2, inum);

	open_rec ("move", move_list, MOVE_NO_FIELDS, "move_move_hash");
	open_rec (inmr, inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (inwu, inwu_list, INWU_NO_FIELDS, "inwu_id_no");
	open_rec (incc, incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (inum,  inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	open_rec (inum2, inum_list, INUM_NO_FIELDS, "inum_id_no2");
	open_rec (inlo,  inlo_list, INLO_NO_FIELDS, "inlo_mst_id");

	OpenLocation (ccmr_rec.hhcc_hash);
	IgnoreAvailChk	=	TRUE;
	strcpy (llctAutoAll, "N");
	strcpy (llctAllLocs, "Y");
	LL_EditLoc		=		(MULT_LOC) ? TRUE : FALSE;
	LL_EditLot		=		(SK_BATCH_CONT) ? TRUE : FALSE;
	LL_EditSLot		=		(SK_BATCH_CONT) ? TRUE : FALSE;
	LL_EditDate		=		(SK_BATCH_CONT) ? TRUE : FALSE;
}

/*
 * Close data base files. 
 */
void
CloseDB (void)
{
	abc_fclose (inmr);
	abc_fclose (incc);
	abc_fclose (inwu);
	abc_fclose (inum);
	abc_fclose (inum2);
	abc_fclose (inlo);
	abc_fclose ("move");
	CloseLocation ();

	SearchFindClose ();
	abc_dbclose (data);
}

/*
 * Get common info from commom database file. 
 */
void
ReadCcmr (void)
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

/*
 * Primary validation and file access here. 
 */
int
spec_valid (
 int field)
{
	int		TempLine;
	int		DspLines	=	0;
	char	locationType [2];

	/*
	 * Validate Item Number. 
	 */
	if (LCHECK ("itemNo"))
	{
		if (last_char == EOI) 
		{
			prog_exit = TRUE; 
			return (EXIT_SUCCESS);
		}
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
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		SuperSynonymError ();

		strcpy (local_rec.item_no, inmr_rec.item_no);
		DSP_FLD ("itemNo");
		DSP_FLD ("itemDesc");

		/*
		 * Look up to see if item is on Cost Centre 
		 */
		incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
		incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
		cc = find_rec (incc, &incc_rec, COMPARISON, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess001));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}
		SR.hhwhHash = incc_rec.hhwh_hash;
		SR.hhbrHash = incc_rec.hhbr_hash;
		strcpy (SR.inmrClass, inmr_rec.inmr_class);
		strcpy (SR.category, inmr_rec.category);
		sprintf (local_rec.item_desc, "%-28.28s", inmr_rec.description);
		DSP_FLD ("itemDesc");

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
			print_mess (ML (mlSkMess621));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		strcpy (SR.uom, inum_rec.uom);
		strcpy (SR.uomGroup, inum_rec.uom_group);
		strcpy (SR.lotCtrl, inmr_rec.lot_ctrl);
		SR.stdCnvFct 		= inum_rec.cnv_fct;
		SR.decPt			= inmr_rec.dec_pt;
		return (EXIT_SUCCESS);
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
			SrchInum (temp_str, line_cnt);
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

		if (ValidItemUom (inmr_rec.hhbr_hash, inum2_rec.hhum_hash))
		{
			print_mess (ML (mlStdMess028));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
			

		strcpy (local_rec.uom, inum2_rec.uom);
		SR.hhumHash = inum2_rec.hhum_hash;
		strcpy (SR.uom, inum2_rec.uom);
        if (inum2_rec.cnv_fct == 0.00)
             inum2_rec.cnv_fct = 1.00;

        SR.convFct = inum2_rec.cnv_fct / SR.stdCnvFct;
		strcpy (SR.uom, inum2_rec.uom);
		strcpy (SR.uomGroup, inum2_rec.uom_group);

		DSP_FLD ("UOM");
		return (EXIT_SUCCESS);
	}

	/*
	 * Qty validation lookup. 
	 */
	if (LCHECK ("qty"))
	{
		local_rec.qty = (float) (n_dec (atof (temp_str), SR.decPt));
		DSP_FLD ("qty");

		if (local_rec.qty < 0.00)
		{
			errmess (ML (mlStdMess190));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		SR.qty = (float) local_rec.qty;

		DSP_FLD ("qty");
		return (EXIT_SUCCESS);
	}

    /*
     * Validate Location.
     */
    if (LCHECK ("locationFrom"))
    {                                                                           
		if (FLD ("locationFrom") == ND)
			return (EXIT_SUCCESS);

        if (SRCH_KEY)
        {                                                                       
			if (search_key	!=	SEARCH)
				SearchLomr (ccmr_rec.hhcc_hash, temp_str);
			else
            	SrchInlo (SR.hhwhHash, SR.hhumHash, temp_str);
            return (EXIT_SUCCESS);                                                          
        }                                                                       
		cc = 	FindLocation 
				(
					SR.hhwhHash,
					SR.hhumHash,
					local_rec.locationFrom,
					ValidLocations,
					&inlo_rec.inlo_hash
				);
		if (cc)
		{
			cc	= 	CheckLocation 
				  	(
						ccmr_rec.hhcc_hash, 
						local_rec.locationFrom,
						locationType
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

    /*
     * Validate Location
     */
    if (LCHECK ("locationTo"))
    {                                                                           
		if (FLD ("locationTo") == ND)
			return (EXIT_SUCCESS);

        if (SRCH_KEY)
        {                                                                       
			if (search_key	!=	SEARCH)
				SearchLomr (ccmr_rec.hhcc_hash, temp_str);
			else
            	SrchInlo (SR.hhwhHash, SR.hhumHash, temp_str);
            return (EXIT_SUCCESS);                                                          
        }                                                                       
		cc = 	FindLocation 
				(
					SR.hhwhHash,
					SR.hhumHash,
					local_rec.locationTo,
					ValidLocations,
					&inlo_rec.inlo_hash
				);
		if (cc)
		{
			cc	= 	CheckLocation 
					(
						ccmr_rec.hhcc_hash, 
						local_rec.locationTo,
						locationType
					);
			if (cc)
			{
            	print_mess (ML (mlStdMess209));
            	sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}
        if (!strcmp (local_rec.locationFrom, local_rec.locationTo))
        {
            errmess (ML ("Location from cannot be the same as location to."));
			sleep (sleepTime);
			clear_mess ();
            return (EXIT_FAILURE);
        }
		return (EXIT_SUCCESS);
	}
	/*
	 * Validate lots and locations. 
	 */
	if (LCHECK ("LL"))
	{
		if (FLD ("LL") == ND)
			return (EXIT_SUCCESS);

		if (SR.lotCtrl [0] != 'Y' && SR.lotCtrl [0] != 'y')
		{
			LL_EditLot		=		FALSE;
			LL_EditSLot		=		FALSE;
			LL_EditDate		=		FALSE;
		}
		else
		{
			LL_EditLot		=		(SK_BATCH_CONT) ? TRUE : FALSE;
			LL_EditSLot		=		(SK_BATCH_CONT) ? TRUE : FALSE;
			LL_EditDate		=		(SK_BATCH_CONT) ? TRUE : FALSE;
		}
		TempLine	=	lcount [1];
		DspLines	=	tab_row + (line_cnt % TABLINES) + 3;
		cc = DisplayLL
			(								/*----------------------*/
				line_cnt,					/*	Line number.		*/
				DspLines,					/*  Row for window		*/
				tab_col + 22,				/*  Col for window		*/
				8,							/*  length for window	*/
				SR.hhwhHash, 				/*	Warehouse hash.		*/
				SR.hhumHash,				/*	UOM hash			*/
				ccmr_rec.hhcc_hash,			/*	CC hash.			*/
				SR.uom,						/* UOM					*/
				0.00,						/* Quantity.			*/
				SR.convFct,					/* Conversion factor.	*/
				TodaysDate (),				/* Expiry Date.			*/
				FALSE,						/* Silent mode			*/
				(local_rec.LL [0] == 'Y'),	/* Input Mode.			*/
				SR.lotCtrl					/* Lot controled item. 	*/
											/*----------------------*/
			);
		/*
		 * Redraw screens. 
		 */
		strcpy (local_rec.LL, "Y");
		putval (line_cnt);

		lcount [1] = (line_cnt + 1 > lcount [1]) ? line_cnt + 1 : lcount [1];
		scn_write (1);
		scn_display (1);
		lcount [1] = TempLine;
		PrintCo ();
			
		if (cc)
			return (EXIT_FAILURE);
		
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);
}

/*
 * Update inlo. 
 */
void
Update (void)
{
	float	ChkQty;
	int		i;
	int		FirstTime	=	TRUE;

	clear ();
	print_at (0,0,ML (mlStdMess035));
	fflush (stdout);

	/*
	 * Update inventory location record (file inlo).
	 */
	for (line_cnt = 0; line_cnt < lcount [1]; line_cnt++) 
	{
		/*
		 * Set switch is something is printed 
		 */
		if (lpsw == FALSE) 
		{ 
			OpenAudit ();
			lpsw = TRUE;
		}
		/*
		 * Get Tabular line. 
		 */
		getval (line_cnt);

		if (!lotBatchTransfer)
		{
			strcpy (err_str, DateToString (0L));
			InLotLocation
			(
				SR.hhwhHash,
				ccmr_rec.hhcc_hash,
				SR.hhumHash,
				SR.uom,
				local_rec.batchString,
				local_rec.batchString,
				local_rec.locationTo,
				" ",
				err_str,
				(float) ToStdUom (SR.qty),
				SR.convFct,
				"A", 
				0.00, 
				0.00, 
				0.00, 
				0.00,
				0L
			);
			if (envVarSkLocTrans)
			{
				/*
				 * Log inventory movements. 
				 */
				MoveAdd
				(
					comm_rec.co_no,
					comm_rec.est_no,
					comm_rec.cc_no,
					SR.hhbrHash,
					ccmr_rec.hhcc_hash,
					SR.hhumHash,
					TodaysDate (),
					2,
					inlo_rec.lot_no,
					SR.inmrClass,		
					SR.category,		
					"LOC TRN TO",
					local_rec.locationTo,
					(float) ToStdUom (SR.qty),
					0.0,
					0.0
				);
			}
			OutLotLocation
			(
				SR.hhwhHash,
				ccmr_rec.hhcc_hash,
				SR.hhumHash,
				SR.uom,
				local_rec.batchString,
				local_rec.batchString,
				local_rec.locationFrom,
				" ",
				err_str,
				(float) ToStdUom (SR.qty),
				SR.convFct,
				"A", 
				0.00, 
				0.00, 
				0.00, 
				0.00,
				0L
			);
			if (envVarSkLocTrans)
			{
				/*
				 * Log inventory movements. 
				 */
				MoveAdd
				(
					comm_rec.co_no,
					comm_rec.est_no,
					comm_rec.cc_no,
					SR.hhbrHash,
					ccmr_rec.hhcc_hash,
					SR.hhumHash,
					TodaysDate (),
					3,
					local_rec.batchString,
					SR.inmrClass,		
					SR.category,		
					"LOC TRN FR",
					local_rec.locationFrom,
					(float) ToStdUom (SR.qty),
					0.0,
					0.0
				);
			}
			fprintf (fout, "|  %-16.16s  ",	local_rec.item_no);
			fprintf (fout, "|  %-40.40s  ",	local_rec.item_desc);
			fprintf (fout, "| %14.2f ",		local_rec.qty);
			fprintf (fout, "| %s ",			local_rec.uom);
			fprintf (fout, "| %10.10s ",	local_rec.locationFrom);
			fprintf (fout, "| %10.10s |\n",	local_rec.locationTo);
		}
		else
		{
			UpdateLotLocation (line_cnt, FALSE);

			FirstTime = TRUE;
			for (i = 0; i < MAX_LOTS; i++)
			{
				if (!LL_Valid (line_cnt, i))
					break;

				ChkQty	=	GetBaseQty (line_cnt,i);
				if (ChkQty != 0.00)
				{
					if (envVarSkLocTrans)
					{
						/*
						 * Log inventory movements. 
						 */
						MoveAdd
						(
							comm_rec.co_no,
							comm_rec.est_no,
							comm_rec.cc_no,
							SR.hhbrHash,
							ccmr_rec.hhcc_hash,
							SR.hhumHash,
							TodaysDate (),
							(ChkQty < 0) ? 3 : 2,
							GetLotNo 	(line_cnt, i),
							SR.inmrClass,		
							SR.category,		
							(ChkQty < 0) ? "LOC TRN FR" : "LOC TRN TO",
							GetLoc (line_cnt, i),
							(ChkQty < 0) ? ChkQty * -1 : ChkQty,
							0.0,
							0.0
						);
					}
					if (FirstTime)
					{
						fprintf (fout, "|  %-16.16s  ",	local_rec.item_no);
						fprintf (fout, "|  %-40.40s  ",	local_rec.item_desc);
					}
					else
					{
						fprintf (fout, "|  %16.16s  ", 	" ");
						fprintf (fout, "|  %40.40s  ", 	" ");
					}
					FirstTime = FALSE;
					fprintf (fout, "| %14.2f ", 	GetQty 		(line_cnt, i));
					fprintf (fout, "| %-4.4s ", 	GetUOM 		(line_cnt, i));
					fprintf (fout, "| %-10.10s ", 	GetLoc 		(line_cnt, i));
					fprintf (fout, "| %-7.7s |\n", 	GetLotNo 	(line_cnt, i));
				
					inwu_rec.hhwh_hash	=	GetHHWH (line_cnt, i);
					inwu_rec.hhum_hash	=	GetHHUM (line_cnt, i);
					cc = find_rec (inwu, &inwu_rec, COMPARISON, "u");
					if (cc)
					{
						memset (&inwu_rec, 0, sizeof (inwu_rec));
						inwu_rec.hhwh_hash	=	GetHHWH (line_cnt, i);
						inwu_rec.hhum_hash	=	GetHHUM (line_cnt, i);
						cc = abc_add (inwu, &inwu_rec);
						if (cc)
							file_err (cc, inwu, "DBADD");

						inwu_rec.hhwh_hash	=	GetHHWH (line_cnt, i);
						inwu_rec.hhum_hash	=	GetHHUM (line_cnt, i);
						cc = find_rec (inwu, &inwu_rec, COMPARISON,"u");
						if (cc)
							file_err (cc, inwu, "DBFIND");
					}
					/*
					 * update inventory cost centre stock record (file incc)
					 */
					inwu_rec.adj 		+= (float) GetBaseQty (line_cnt, i);

					inwu_rec.closing_stock = inwu_rec.opening_stock +
											 inwu_rec.pur +
											 inwu_rec.receipts +
											 inwu_rec.adj -
											 inwu_rec.issues -
											 inwu_rec.sales;

					cc = abc_update (inwu,&inwu_rec);
					if (cc)
						file_err (cc, inwu, "DBUPDATE");
				}
			}
		}
	} 
}

/*
 * Search on UOM 
 */
void
SrchInum (
 char 	*key_val,
 int	line_cnt)
{
	work_open ();
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

		if (!ValidItemUom (inmr_rec.hhbr_hash, inum2_rec.hhum_hash))
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
		file_err (cc, "inum2", "DBFIND");
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
		rv_pr (ML (mlSkMess041), 51, 0, 1);
		line_at (1,0,130);

		PrintCo ();

		line_cnt = 0;
		scn_write (scn);
	}
    return (EXIT_SUCCESS);
}

void
PrintCo (void)
{
	line_at (20,0,130);

	print_at (21,0, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_name);
	print_at (22,0, ML (mlStdMess039), comm_rec.est_no, comm_rec.est_name);
}

/*
 * These are functions for the audit printing.                             
 *                                                                         
 * Routine to open output pipe to standard print to provide an audit trail 
 * of events. This also sends the output straight to the spooler.          
 */
void
OpenAudit (void)
{
	if ((fout = popen ("pformat", "w")) == 0) 
		sys_err ("Error in pformat During (POPEN)", errno, PNAME);

	fprintf (fout, ".START%s<%s>\n", DateToString (comm_rec.inv_date), PNAME);
	fprintf (fout, ".SO\n");
	fprintf (fout, ".LP%d\n", printerNumber);
	fprintf (fout, ".12\n");
	fprintf (fout, ".L138\n");
	fprintf (fout, ".EINVENTORY LOCATION TRANSFERS\n");
	fprintf (fout, ".B1\n");
	fprintf (fout, ".ECO : %s - %s\n", comm_rec.co_no, clip (comm_rec.co_name));
	fprintf (fout, ".EBR : %s - %s\n", comm_rec.est_no, clip (comm_rec.est_name));
	fprintf (fout, ".EAS AT %s\n", SystemTime ());
	fprintf (fout, ".B1\n");

	fprintf (fout, ".R=====================");
	fprintf (fout, "=============================================");
	fprintf (fout, "=================");
	fprintf (fout, "=======");
	if (lotBatchTransfer)
		fprintf (fout, "========================\n");
	else
		fprintf (fout, "===========================\n");

	fprintf (fout, "=====================");
	fprintf (fout, "=============================================");
	fprintf (fout, "=================");
	fprintf (fout, "=======");
	if (lotBatchTransfer)
		fprintf (fout, "========================\n");
	else
		fprintf (fout, "===========================\n");
		
	fprintf (fout, "|    ITEM NUMBER     ");
	fprintf (fout, "|              ITEM DESCRIPTION              ");
	fprintf (fout, "|   QUANTITY     ");
	fprintf (fout, "| UOM. ");
	if (lotBatchTransfer)
		fprintf (fout, "|  LOCATION  | LOT NO. |\n");
	else
		fprintf (fout, "|  LOCATION  |  LOCATION  |\n");

	fprintf (fout, "|                    ");
	fprintf (fout, "|                                            ");
	fprintf (fout, "|   TRANSFERED   ");
	fprintf (fout, "|      ");
	if (lotBatchTransfer)
		fprintf (fout, "|  NUMBER    |         |\n");
	else
		fprintf (fout, "|    FROM    |     TO     |\n");

	fprintf (fout, "|--------------------");
	fprintf (fout, "|--------------------------------------------");
	fprintf (fout, "|----------------");
	fprintf (fout, "|------");
	if (lotBatchTransfer)
		fprintf (fout, "|------------|---------|\n");
	else
		fprintf (fout, "|------------|------------|\n");
	
	fprintf (fout, ".PI12\n");
}

/*
 * Routine to close the audit trail output file. 
 */
void
CloseAudit (void)
{
	if (lpsw == TRUE) 	
	{
		fprintf (fout, ".EOF\n");
		pclose (fout);
	}
}

float	
ToStdUom (
	float	lclQty)
{
	float	cnvQty;

	if (F_HIDE (label ("UOM")))
		return (lclQty);

	if (SR.convFct == 0.00 || lclQty == 0.00)
		return (0.00);

	cnvQty = lclQty * SR.convFct;

	return (cnvQty);
}


/*
 * Search for valid Location. 
 */
void
SrchInlo
 (
 	long	hhwhHash,
 	long	hhumHash,
	char	*KeyValue
)
{
	char	DispUom [100];
	char	tempWorkStr [31];
	float	AllocQty;

	abc_selfield ("inlo", "inlo_id_loc");
	_work_open (25,10,20);
	inlo_rec.hhwh_hash 	= 	hhwhHash;
	strcpy (inlo_rec.loc_type, " ");
	sprintf (inlo_rec.location,"%-10.10s", "          ");
	cc = save_rec ("#  LOCATION  - LOT NO. ","# UOM. |TYPE| QUANTITY ");
	cc = find_rec ("inlo", &inlo_rec, GTEQ,"r");
	while (!cc && (inlo_rec.hhwh_hash == hhwhHash || hhwhHash == 0L))
	{
		if (strncmp (inlo_rec.location,KeyValue,strlen (KeyValue)))
		{
			cc = find_rec ("inlo", &inlo_rec, NEXT,"r");
			continue;		
		}
		if (inlo_rec.cnv_fct == 0.00)
			inlo_rec.cnv_fct	=	1.00;

		if (inlo_rec.hhum_hash != hhumHash && hhumHash != 0L)
		{
			cc = find_rec ("inlo", &inlo_rec, NEXT,"r");
			continue;		
		}
		sprintf (DispUom, "%-10.10s %-7.7s %010ld %s",
							inlo_rec.location,
							inlo_rec.lot_no,
							inlo_rec.hhum_hash,
							inlo_rec.loc_type);
		AllocQty	=	inlo_rec.qty - CalcAlloc (inlo_rec.inlo_hash,line_cnt);

		sprintf (tempWorkStr, LLQtyMask, AllocQty / inlo_rec.cnv_fct);
		sprintf (err_str," %s | %s  |%11.11s",	inlo_rec.uom, 
												inlo_rec.loc_type,
									    		tempWorkStr);
		cc = save_rec (DispUom,err_str);
		if (cc)
			break;

		cc = find_rec ("inlo",&inlo_rec,NEXT,"r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	abc_selfield ("inlo", "inlo_mst_id");

	inlo_rec.hhwh_hash 	= 	hhwhHash;
	inlo_rec.hhum_hash	=	atol (temp_str + 20);
	sprintf (inlo_rec.location, "%-10.10s",	temp_str);
	sprintf (inlo_rec.lot_no,   "%-7.7s",	temp_str + 11);
	sprintf (inlo_rec.loc_type, "%-1.1s", temp_str + 30);

	cc = find_rec ("inlo", &inlo_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, "inlo", "DBFIND");
}

/*=====================================================================
|  Copyright (C) 1998 - 2002 LogisticSoftware                         |
|=====================================================================|
| $Id: qc_manual.c,v 5.4 2002/07/24 08:39:08 scott Exp $
|  Program Name  : (qc_manual.c   )                                   |
|  Program Desc  : (QC Manual Entry.                            )     |
|---------------------------------------------------------------------|
|  Date Written  : 30/08/94        | Author       : Aroha Merrilees.  |
|---------------------------------------------------------------------|
| $Log: qc_manual.c,v $
| Revision 5.4  2002/07/24 08:39:08  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.3  2002/07/03 04:26:10  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: qc_manual.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/QC/qc_manual/qc_manual.c,v 5.4 2002/07/24 08:39:08 scott Exp $";

#define	SR			store [line_cnt]

#define	MAXLINES	2000
#define	TABLINES	10

#include	<pslscr.h>
#include	<twodec.h>
#include	<ml_std_mess.h>
#include	<ml_qc_mess.h>
#include	<search_utils.h>
#include	<tabdisp.h>

#include	"schema"

struct commRecord	comm_rec;
struct ccmrRecord	ccmr_rec;
struct inmrRecord	inmr_rec;
struct inccRecord	incc_rec;
struct inccRecord	incc2_rec;
struct qcmrRecord	qcmr_rec;
struct qchrRecord	qchr_rec;
struct insfRecord	insf_rec;
struct inumRecord	inum_rec;
struct inumRecord	inum2_rec;
struct inloRecord	inlo_rec;
struct lomrRecord	lomr_rec;

	char	*data	= "data",
			*inum2 	= "inum2",
			*incc2	= "incc2";

	int		envSoFwdAvl;
	FILE	*fout;

struct storeRec {
	long	hhbrHash;
	long	hhccHash;
	long	hhwhHash;
	long	hhumHash;
	char	lotControl [2];
	char	serialItem [2];
	double  quantity;
	char	uom [5];
	char	uomGroup [21];
	float	cnvFct;
	float	stdCnvFct;
	int		decPt;
} store [MAXLINES];

/*
 * Local & Screen Structures. 
 */
struct {
	char	dummy [11];
	char	sysDate [11];
	long	lsysDate;
	int		printerNumber;

	char	itemNo [17];
	char	itemDesc [41];
	float	qcQty;
	char	serNo [26];
	char	qcCentre [5];
	char	uom [5];
	char	LL [2];
} local_rec;

static	struct	var	vars [] =
{
	{1, TAB, "itemNo", 		MAXLINES, 0, CHARTYPE, 
		"UUUUUUUUUUUUUUUU", "          ", 
		" ", " ", "  Item Number   ", "QC Item Number", 
		YES, NO, JUSTLEFT, "", "", local_rec.itemNo}, 
	{1, TAB, "itemDesc",		0, 0, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "           Item  Description           ", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.itemDesc}, 
	{1, TAB, "UOM",	 0, 1, CHARTYPE,
		"AAAA", "          ",
		" ", " ", " UOM. ", "Enter the Conversion to UOM. Default = standard UOM ",
		YES, NO,  JUSTLEFT, "", "", local_rec.uom},
	{1, TAB, "qcQty",	0, 0, FLOATTYPE, 
		"NNNNNNN.NNN", "          ", 
		" ", "1.00", "  QC Qty  ", "Quantity To Be Re-QC-Checked", 
		YES, NO, JUSTRIGHT, "0.000", "999999.999", (char *)&local_rec.qcQty}, 
	{1, TAB, "LL", 	 0, 0, CHARTYPE, 
		"U", "          ", 
		" ", "N", "L", "Lot / Location Selection. <return> ", 
		 YES, NO,  JUSTLEFT, "", "", local_rec.LL}, 
	{1, TAB, "serNo", 	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", "", "      Serial Number      ", "Serial Number",
		YES, NO, JUSTLEFT, "", "", local_rec.serNo}, 
	{1, TAB, "qcCentre",	 0, 0, CHARTYPE, 
		"UUUU", "          ", 
		" ", " ", "Cntr", "QC Centre", 
		 YES, NO,  JUSTLEFT, "", "", local_rec.qcCentre}, 
	{0, LIN, "", 	 0, 0, CHARTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}, 
};

#include	<LocHeader.h>

/*
 * Local function prototypes 
 */
void	OpenDB				(void);
void	CloseDB			 	(void);
void	PrintCo				(void);
int		LookForQCLocation	(long);
void	SrchInum			(char *, int);
int		DeleteItem			(void);
int		ValidQuantity		(double, int);
void	Update				(void);
void	SrchQcmr			(char *);
void	SrchInsf			(char *);
void	SrchInmr			(char *);
void	PrintHeading		(void);
int		heading (int);

extern	int	qualityControlSrch;


/*
 * Main Processing Routine . |
 */
int
main (
 int	argc,
 char *	argv [])
{
	char *	sptr;
	int		QC_APPLY;

	qualityControlSrch	=	TRUE;

	SETUP_SCR (vars);


	QC_APPLY = (sptr = chk_env ("QC_APPLY")) ? atoi (sptr) : 0;
	if (!QC_APPLY)
	{
		/*QC_APPLY not set for this program to be used\n\n*/
		print_at (0, 0, ML (mlQcMess001));
		return (EXIT_FAILURE);
	}

	if (argc != 2)
	{
		print_at (0, 0, ML (mlStdMess036), argv [0]);
		return (EXIT_FAILURE);
	}
	local_rec.printerNumber = atoi (argv [1]);

	envSoFwdAvl = atoi (get_env ("SO_FWD_AVL"));

	strcpy (local_rec.sysDate, DateToString (TodaysDate ()));
	local_rec.lsysDate = TodaysDate ();

	OpenDB ();

	FLD ("LL") = ND;
	if (SK_BATCH_CONT || MULT_LOC)
		FLD ("LL") = YES;

	/*
	 * Setup required parameters. 
	 */
	init_scr ();
	set_tty ();
	set_masks ();
/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (1, store, sizeof (struct storeRec));
#endif

	/* Open print file. */
	if ((fout = popen ("pformat", "w")) == 0)
		sys_err ("Error in pformat during (POPEN)", errno, PNAME);
	PrintHeading ();

	/*
	 * Beginning of input control loop 
	 */
	while (!prog_exit)
	{
		/*
		 * Reset control flags 
		 */
		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		search_ok	= TRUE;

		lcount [1]	= 0;
		init_vars (1);

		/*
		 * Enter screen 1 linear input 
		 */
		heading (1);
		entry (1);
		if (prog_exit || restart)
			continue;

		line_cnt = 0;
		heading (1);
		scn_display (1);
		edit (1);
		if (restart)
			continue;

		Update ();
	}
	/* Close print file */
	fprintf (fout, ".EOF\n");
	pclose (fout);

	CloseDB (); 
	FinishProgram ();
	return (EXIT_SUCCESS);
}

/*
 * Open data base files 
 */
void
OpenDB (void)
{
	abc_dbopen (data);

	read_comm (comm_list, COMM_NO_FIELDS, (char *) &comm_rec);

	abc_alias (incc2, incc);
	abc_alias (inum2, inum);

	open_rec (inmr,  inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (incc,  incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (incc2, incc_list, INCC_NO_FIELDS, "incc_hhwh_hash");
	open_rec (qcmr,  qcmr_list, QCMR_NO_FIELDS, "qcmr_id_no");
	open_rec (qchr,  qchr_list, QCHR_NO_FIELDS, "qchr_id_no");
	open_rec (insf,  insf_list, INSF_NO_FIELDS, "insf_id_no");
	open_rec (ccmr,  ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	open_rec (inum,  inum_list, INUM_NO_FIELDS, "inum_hhum_hash");
	open_rec (inum2, inum_list, INUM_NO_FIELDS, "inum_id_no2");
	open_rec (inlo,  inlo_list, INLO_NO_FIELDS, "inlo_inlo_hash");
	open_rec (lomr,  lomr_list, LOMR_NO_FIELDS, "lomr_id_no");

	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, comm_rec.est_no);
	strcpy (ccmr_rec.cc_no, comm_rec.cc_no);
	cc = find_rec (ccmr, &ccmr_rec, EQUAL, "r");
	if (cc)
		file_err (cc, (char *)ccmr, "DBFIND");

	abc_fclose (ccmr);

	OpenLocation (ccmr_rec.hhcc_hash);
}

/*
 * Close data base files 
 */
void
CloseDB (void)
{
	abc_fclose (inmr);
	abc_fclose (incc);
	abc_fclose (incc2);
	abc_fclose (qcmr);
	abc_fclose (qchr);
	abc_fclose (insf);
	abc_fclose (inum);
	abc_fclose (inum2);
	abc_fclose (lomr);
	abc_fclose (inlo);
	CloseLocation ();

	SearchFindClose ();
	abc_dbclose (data);
}

int
heading (
 int	scn)
{
	if (!restart) 
	{
		if (scn != cur_screen)
			scn_set (scn);

		swide ();
		clear ();

		strcpy (err_str, ML (mlQcMess024));
		rv_pr (err_str, (132 - strlen (err_str)) / 2, 0, 1);
		move (0, 1);
		line (132);

		PrintCo ();

		/*  reset this variable for new screen NOT page	*/
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}

void
PrintCo (
 void)
{
	move (0, 21);
	line (132);

	print_at (22, 0, ML (mlStdMess038),
		comm_rec.co_no, comm_rec.co_short);

	print_at (22, 45, ML (mlStdMess039),
		comm_rec.est_no, comm_rec.est_short);

	print_at (22, 60,ML (mlStdMess099),
		comm_rec.cc_no, comm_rec.cc_short);
}
int
spec_valid (
 int	field)
{
	int		TempLine;
	long	workInloHash	=	0L;

	if (LCHECK ("itemNo"))
	{
		if (last_char == DELLINE)
			return (DeleteItem ());

		if (SRCH_KEY)
		{
			InmrSearch (comm_rec.co_no, temp_str, 0L, "N");
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
			return (EXIT_FAILURE);

		cc = FindInmr (comm_rec.co_no, local_rec.itemNo, 0L, "N");
		if (!cc)
		{
			strcpy (inmr_rec.co_no, comm_rec.co_no);
			strcpy (inmr_rec.item_no, local_rec.itemNo);
			cc = find_rec (inmr, &inmr_rec, COMPARISON, "r");
		}
		if (cc)
		{
			print_mess (ML (mlStdMess001));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();

		if (inmr_rec.qc_reqd [0] != 'Y')
		{
			print_mess (ML (mlQcMess003));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
		incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
		cc = find_rec (incc, &incc_rec, EQUAL, "r");
		if (cc)
		{
			print_mess (ML (mlStdMess192));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		strcpy (local_rec.itemNo, inmr_rec.item_no);
		sprintf (local_rec.itemDesc, "%-40.40s", inmr_rec.description);

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
			print_mess (ML (mlQcMess027));
			sleep (sleepTime);
			return (EXIT_FAILURE);
		}

		strcpy (SR.uom, inum_rec.uom);
		strcpy (SR.uomGroup, inum_rec.uom_group);
		strcpy (SR.lotControl, inmr_rec.lot_ctrl);
		strcpy (SR.serialItem, inmr_rec.serial_item);
		SR.stdCnvFct 	= inum_rec.cnv_fct;
		SR.hhumHash 	= inum_rec.hhum_hash;
		SR.hhbrHash 	= incc_rec.hhbr_hash;
		SR.hhwhHash 	= incc_rec.hhwh_hash;
		SR.hhccHash 	= incc_rec.hhcc_hash;
		SR.decPt		= inmr_rec.dec_pt;
		
		FLD ("serNo") 	= (SR.serialItem [0] == 'Y') ? YES : NA;

		DSP_FLD ("itemNo");
		DSP_FLD ("itemDesc");

		cc =	FindLocation 
				(
					SR.hhwhHash, 
					SR.hhumHash, 
					NULL, 
					"Q", 
					&workInloHash
				);
		if (cc)
		{
			cc = LookForQCLocation (SR.hhccHash);
			if (cc)
			{
				print_mess (ML (mlQcMess026));
				sleep (sleepTime);
				return (EXIT_FAILURE);
			}
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("qcQty"))
	{
		float	available;

		local_rec.qcQty = (float) n_dec (local_rec.qcQty, SR.decPt);
		DSP_FLD ("qcQty");
		if (!ValidQuantity (local_rec.qcQty, SR.decPt))
			return (EXIT_FAILURE);

		cc = find_hash (incc2, &incc_rec, EQUAL, "r", SR.hhwhHash);
		if (cc)
		{
			print_mess (ML (mlStdMess192));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (SR.serialItem [0] == 'Y' && local_rec.qcQty > 1.00)
		{
			local_rec.qcQty = 1.00;
			DSP_FLD ("qcQty");
		}

		available = incc_rec.closing_stock -
					incc_rec.committed -
					incc_rec.backorder -
					incc_rec.qc_qty;
		if (envSoFwdAvl)
			available -= incc_rec.forward;
		if ((available < local_rec.qcQty) && local_rec.qcQty != 0.00)
		{
			/*sprintf (err_str, "%cOnly %f Available (%f QC Qty), Trying To Issue %f ", 
					n_dec (available, SR.decPt),
					n_dec (incc_rec.qc_qty, SR.decPt),
					local_rec.qcQty);*/

			sprintf (err_str,
					 ML (mlQcMess025),  
					 local_rec.qcQty,
					 n_dec (available,
					 SR.decPt));
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		SR.quantity	=	local_rec.qcQty;
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("serNo"))
	{
		if (F_NOKEY (label ("serNo")))
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			SrchInsf (temp_str);
			return (EXIT_SUCCESS);
		}

		insf_rec.hhwh_hash = SR.hhwhHash;
		strcpy (insf_rec.status, "F");
		strcpy (insf_rec.serial_no, local_rec.serNo);
		if (find_rec (insf, &insf_rec, EQUAL, "r"))
		{
			print_mess (ML (mlStdMess201));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("qcCentre"))
	{
		if (SRCH_KEY)
		{
			SrchQcmr (temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
		{
			/* Read default from incc record. */
			if (find_hash (incc2, &incc_rec, EQUAL, "r", SR.hhwhHash))
				return (EXIT_FAILURE);

			if (!strcmp (incc_rec.qc_centre, "    "))
				return (EXIT_FAILURE);
			
			strcpy (local_rec.qcCentre, incc_rec.qc_centre);
			DSP_FLD ("qcCentre");
			return (EXIT_SUCCESS);
		}

		strcpy (qcmr_rec.co_no, comm_rec.co_no);
		strcpy (qcmr_rec.br_no, comm_rec.est_no);
		strcpy (qcmr_rec.centre, local_rec.qcCentre);
		if (find_rec (qcmr, &qcmr_rec, EQUAL, "r"))
		{
			print_mess (ML (mlStdMess131));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
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
	
		if (ValidItemUom (SR.hhbrHash, inum2_rec.hhum_hash))
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

        SR.cnvFct = inum2_rec.cnv_fct / SR.stdCnvFct;
		strcpy (SR.uom, inum2_rec.uom);
		strcpy (SR.uomGroup, inum2_rec.uom_group);

		DSP_FLD ("UOM");
		return (EXIT_SUCCESS);
	}
	/*
	 * Validate lots and locations. 
	 */
	if (LCHECK ("LL"))
	{
		if (FLD ("LL") == ND)
			return (EXIT_SUCCESS);

		TempLine = lcount [1];
		cc = DisplayLL
			(										/*----------------------*/
				line_cnt,							/*	Line number.		*/
				tab_row + 3,						/*  Row for window		*/
				tab_col + 3,						/*  Col for window		*/
				4,									/*  length for window	*/
				SR.hhwhHash, 						/*	Warehouse hash.		*/
				SR.hhumHash,						/*	UOM hash			*/
				SR.hhccHash,						/*	CC hash.			*/
				SR.uom,								/* UOM					*/
				(float) SR.quantity,				/* Quantity.			*/
				SR.cnvFct,							/* Conversion factor.	*/
				TodaysDate (),						/* Expiry Date.			*/
				FALSE,								/* Silent mode			*/
				(local_rec.LL [0] == 'Y'),			/* Input Mode.			*/
				SR.lotControl						/* Lot controled item. 	*/
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

int
LookForQCLocation (
 long	hhccHash)
{
	lomr_rec.hhcc_hash	=	hhccHash;
	strcpy (lomr_rec.location, "          ");
	cc = find_rec (lomr, &lomr_rec, GTEQ, "r");
	while (!cc && lomr_rec.hhcc_hash == hhccHash)
	{
		if (lomr_rec.loc_type [0] == 'Q')
			return (EXIT_SUCCESS);

		cc = find_rec (lomr, &lomr_rec, NEXT, "r");
	}
	return (EXIT_FAILURE);
}

/*
 * Search on UOM 
 */
void
SrchInum (
	char 	*keyValue,
	int		line_cnt)
{
	_work_open (4,0,40);
	save_rec ("#UOM","#Description");

	strcpy (inum2_rec.uom_group, SR.uomGroup);
	strcpy (inum2_rec.uom, keyValue);
	cc = find_rec (inum2, &inum2_rec, GTEQ, "r");
	while (!cc &&
		   !strcmp (inum2_rec.uom_group, SR.uomGroup))
	{
		if (strncmp (inum2_rec.uom, keyValue, strlen (keyValue)))
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
	strcpy (inum2_rec.uom, keyValue);
	cc = find_rec (inum2, &inum2_rec, COMPARISON, "r");
	if (cc)
		file_err (cc, inum2, "DBFIND");
}

int
DeleteItem (
 void)
{
	int	cnt,
		this_page = line_cnt / TABLINES;

	if (prog_status == ENTRY)
	{
		print_mess (ML (mlStdMess005));
		return (EXIT_FAILURE);
	}

	lcount [1]--;
	for (cnt = line_cnt; line_cnt < lcount [1]; line_cnt++)
	{
		getval (line_cnt + 1);
		putval (line_cnt);
		if (this_page == line_cnt / TABLINES)
			line_display ();
	}
	if (this_page == line_cnt / TABLINES)
		blank_display ();
	line_cnt = cnt;
	getval (line_cnt);

	return (EXIT_SUCCESS);
}

/*
 * Checks if the quantity entered by the user valid quantity that can be 
 * saved to a float variable without any problems of losing figures after 
 * the decimal point. eg. if _dec_pt is 2 then the greatest quantity the 
 * user can enter is 99999.99  
 */
int
ValidQuantity (
	double	_qty,
	int	_dec_pt)
{
	/*
	 * Quantities to be compared with with the user has entered.   
	 */
	double	compare [7];
	
	compare [0] = 9999999;
	compare [1] = 999999.9;
	compare [2] = 99999.99;
	compare [3] = 9999.999;
	compare [4] = 999.9999;
	compare [5] = 99.99999;
	compare [6] = 9.999999;

	if (_qty > compare [_dec_pt])
	{
		sprintf (err_str, 
				 ML (mlQcMess019),
				 _qty,
				 compare [_dec_pt]);
		print_mess (err_str);
		sleep (sleepTime);
		clear_mess ();
		return (FALSE);
	}
	return (TRUE);
}

void
tab_other (
 int	lineNo)
{
	if (cur_screen != 2)
		return;
}

/*
 * Update pcat records from TAB screen 
 */
void
Update (void)
{
	float	qcDays;
	char	QC_Loc [11];
	char	workLotNo [8];
	int		i;

	abc_selfield (inmr, "inmr_hhbr_hash");
	scn_set (1);
	for (line_cnt = 0; line_cnt < lcount [1]; line_cnt++)
	{
		getval (line_cnt);

		if (MULT_LOC || SK_BATCH_CONT)
		{
			UpdateLotLocation (line_cnt, TRUE);

			for (i = 0; i < MAX_LOTS; i++)
			{
				if (!LL_Valid (line_cnt, i))
					break;
				
				if (GetQty (line_cnt, i) == 0.00)
					continue;

				cc = 	FindLocation 
						(
							SR.hhwhHash, 
							SR.hhumHash, 
							NULL, 
							"Q",
							&inlo_rec.inlo_hash
						);
				if (cc)
				{
					cc = LookForQCLocation (SR.hhccHash);
					if (cc)
						file_err (cc, (char *)lomr, "DBFIND");

					strcpy (QC_Loc, lomr_rec.location);
				}
				else
				{
					cc = find_rec (inlo, &inlo_rec, COMPARISON, "r");
					if (cc)
						file_err (cc, (char *)inlo, "DBFIND");
						
					strcpy (QC_Loc, inlo_rec.location);
				}


				strcpy (workLotNo, GetLotNo (line_cnt, i));
				InLotLocation
				(
					SR.hhwhHash,
					SR.hhccHash,
					SR.hhumHash,
					GetUOM 		(line_cnt, i),
					workLotNo,
					GetSLotNo 	(line_cnt, i),
					QC_Loc,
					"Q",
					GetExpiry   (line_cnt, i),
					GetBaseQty 	(line_cnt, i),
					GetCnvFct	(line_cnt, i),
					GetLocStat	(line_cnt, i),
					GetPackQty	(line_cnt, i),
					GetChgWgt	(line_cnt, i),
					GetGrossWgt	(line_cnt, i),
					GetCuMetres	(line_cnt, i),
					GetSKND	 	(line_cnt, i)
				);
				if (find_hash (incc2, &incc_rec, EQUAL, "r", SR.hhwhHash))
					incc_rec.qc_time = 0.00;

				qcDays = incc_rec.qc_time * 7; /* get qc_time in days */

				cc = 	FindLocation 
						(
							SR.hhwhHash, 
							SR.hhumHash, 
							QC_Loc, 
							"Q",
							&inlo_rec.inlo_hash
						);
				if (cc)
					file_err (cc, (char *)inlo, "DBFIND");

				cc = find_rec (inlo, &inlo_rec, EQUAL, "r");
				if (cc)
					file_err (cc, (char *)inlo, "DBFIND");
				
				strcpy (qchr_rec.co_no,		comm_rec.co_no);
				strcpy (qchr_rec.br_no,		comm_rec.est_no);
				strcpy (qchr_rec.wh_no,		comm_rec.cc_no);
				strcpy (qchr_rec.qc_centre,	local_rec.qcCentre);
				qchr_rec.hhbr_hash			= SR.hhbrHash;
				qchr_rec.hhum_hash			= SR.hhumHash;
				qchr_rec.origin_qty			= GetBaseQty (line_cnt, i),
				qchr_rec.receipt_dt			= local_rec.lsysDate;
				qchr_rec.exp_rel_dt			= local_rec.lsysDate + (long)qcDays;
				qchr_rec.rel_qty			= 0.00;
				qchr_rec.rej_qty			= 0.00;
				qchr_rec.inlo_hash			= inlo_rec.inlo_hash;
				sprintf (qchr_rec.serial_no,"%-25.25s", local_rec.serNo);
				sprintf (qchr_rec.ref_1,	"%-10.10s", "Manual");
				sprintf (qchr_rec.ref_2,	"%-10.10s", " ");
				qchr_rec.next_seq			= 0;
				strcpy (qchr_rec.source_type,"M");

				if ((cc = abc_add (qchr, &qchr_rec)))
					file_err (cc, (char *)qchr, "DBFIND");

				fprintf (fout, "|%-10.10s",	DateToString (local_rec.lsysDate));
				fprintf (fout, "|%-4.4s",	local_rec.qcCentre);
				fprintf (fout, "|%-16.16s",	local_rec.itemNo);
				fprintf (fout, "|%-40.40s",	local_rec.itemDesc);
				fprintf (fout, "|%-4.4s",	GetUOM 		(line_cnt, i));
				fprintf (fout, "|%11.3f",n_dec (local_rec.qcQty, SR.decPt));
				fprintf (fout, "|%-7.7s",	GetLotNo 	(line_cnt, i));
				fprintf (fout, "|%-7.7s",	GetSLotNo 	(line_cnt, i));
				fprintf (fout, "|%-10.10s",	GetExpiry   (line_cnt, i));
				fprintf (fout, "|%-10.10s",	QC_Loc);
				fprintf (fout, "|%-25.25s|\n",	local_rec.serNo);
			}
		}
		else
		{
			if (find_hash (incc2, &incc_rec, EQUAL, "r", SR.hhwhHash))
				incc_rec.qc_time = 0.00;

			qcDays = incc_rec.qc_time * 7; /* get qc_time in days */

			strcpy (qchr_rec.co_no,		comm_rec.co_no);
			strcpy (qchr_rec.br_no,		comm_rec.est_no);
			strcpy (qchr_rec.wh_no,		comm_rec.cc_no);
			strcpy (qchr_rec.qc_centre,	local_rec.qcCentre);
			qchr_rec.hhbr_hash			= SR.hhbrHash;
			qchr_rec.hhum_hash			= SR.hhumHash;
			qchr_rec.inlo_hash			= 0L;
			qchr_rec.origin_qty			= local_rec.qcQty;
			qchr_rec.receipt_dt			= local_rec.lsysDate;
			qchr_rec.exp_rel_dt			= local_rec.lsysDate + (long) qcDays;
			qchr_rec.rel_qty			= 0.00;
			qchr_rec.rej_qty			= 0.00;
			sprintf (qchr_rec.serial_no,"%-25.25s", local_rec.serNo);
			sprintf (qchr_rec.ref_1,	"%-10.10s", "Manual");
			sprintf (qchr_rec.ref_2,	"%-10.10s", " ");
			qchr_rec.next_seq			= 0;
			strcpy (qchr_rec.source_type,"M");

			if ((cc = abc_add (qchr, &qchr_rec)))
				file_err (cc, (char *)qchr, "DBFIND");

			fprintf (fout, "|%-10.10s",	DateToString (local_rec.lsysDate));
			fprintf (fout, "|%-4.4s",	local_rec.qcCentre);
			fprintf (fout, "|%-16.16s",	local_rec.itemNo);
			fprintf (fout, "|%-40.40s",	local_rec.itemDesc);
			fprintf (fout, "|%-4.4s",	GetUOM 		(line_cnt, 0));
			fprintf (fout, "|%11.3f",	n_dec (local_rec.qcQty, SR.decPt));
			fprintf (fout, "|%-7.7s",	"  N/A  ");
			fprintf (fout, "|%-7.7s",	"  N/A  ");
			fprintf (fout, "|%-10.10s",	"   N/A    ");
			fprintf (fout, "|%-10.10s",	"   N/A    ");
			fprintf (fout, "|%-25.25s|\n",	local_rec.serNo);
		}

		/* Update company qc quantity. */
		if ((cc = find_hash (inmr, &inmr_rec, EQUAL, "u", SR.hhbrHash)))
			file_err (cc, (char *)inmr, "DBFIND");

		inmr_rec.qc_qty += local_rec.qcQty;
		if ((cc = abc_update (inmr, &inmr_rec)))
			file_err (cc, (char *)inmr, "DBUPDATE");

		/* Update warehouse qc quantity. */
		if ((cc = find_hash (incc2, &incc_rec, EQUAL, "u", SR.hhwhHash)))
			file_err (cc, (char *)incc, "DBFIND");

		incc_rec.qc_qty += local_rec.qcQty;
		if ((cc = abc_update (incc2, &incc_rec)))
			file_err (cc, (char *)incc, "DBUPDATE");

		/* If a serial item, update insf record as committed. */
		if (SR.serialItem [0] == 'Y')
		{
			insf_rec.hhwh_hash = SR.hhwhHash;
			strcpy (insf_rec.status, "F");
			sprintf (insf_rec.serial_no, "%-25.25s", local_rec.serNo);
			if (find_rec (insf, &insf_rec, EQUAL, "u"))
				file_err (cc, (char *)insf, "DBFIND");

			strcpy (insf_rec.status, "C");
			if (abc_update (insf, &insf_rec))
				file_err (cc, (char *)insf, "DBUPDATE");
		}
	}
	abc_selfield (inmr, "inmr_id_no");
}

/*
 * Search for QC Centre 
 */
void
SrchQcmr (
 char *	keyValue)
{
	_work_open (4,0,40);
	save_rec ("#Cntr", "#Description");

	strcpy (qcmr_rec.co_no, comm_rec.co_no);
	strcpy (qcmr_rec.br_no, comm_rec.est_no);
	sprintf (qcmr_rec.centre, "%-4.4s", keyValue);
	cc = find_rec (qcmr, &qcmr_rec, GTEQ, "r");
	while (!cc &&
		!strcmp (qcmr_rec.co_no, comm_rec.co_no) &&
		!strcmp (qcmr_rec.br_no, comm_rec.est_no) &&
		!strncmp (qcmr_rec.centre, keyValue, strlen (keyValue)))
	{
		if (save_rec (qcmr_rec.centre, qcmr_rec.description))
			break;

		cc = find_rec (qcmr, &qcmr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (qcmr_rec.co_no, comm_rec.co_no);
	strcpy (qcmr_rec.br_no, comm_rec.est_no);
	sprintf (qcmr_rec.centre, "%-4.4s", temp_str);
	if ((cc = find_rec (qcmr, &qcmr_rec, EQUAL, "r")))
		file_err (cc, (char *)qcmr, "DBFIND");
}

/*
 * Search for Serial No 
 */
void
SrchInsf (
 char *	keyValue)
{
	_work_open (25,0,2);
	save_rec ("#Serial Number", "#Status");

	insf_rec.hhwh_hash = SR.hhwhHash;
	strcpy (insf_rec.status, "F");
	sprintf (insf_rec.serial_no, "%-25.25s", keyValue);
	cc = find_rec (insf, &insf_rec, GTEQ, "r");
	while (!cc &&
		insf_rec.hhwh_hash == SR.hhwhHash &&
		!strcmp (insf_rec.status, "F") &&
		!strncmp (insf_rec.serial_no, keyValue, strlen (keyValue)))
	{
		if (save_rec (insf_rec.serial_no, insf_rec.status))
			break;

		cc = find_rec (insf, &insf_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	insf_rec.hhwh_hash = SR.hhwhHash;
	strcpy (insf_rec.status, "F");
	sprintf (insf_rec.serial_no, "%-25.25s", temp_str);
	if ((cc = find_rec (insf, &insf_rec, EQUAL, "r")))
		file_err (cc, (char *)insf, "DBFIND");
}

void
PrintHeading (
 void)
{
	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (fout, ".SO\n");
	fprintf (fout, ".LP%d\n", local_rec.printerNumber);
	fprintf (fout, ".10\n");
	fprintf (fout, ".L152\n");
	fprintf (fout, ".E QC MANUAL RECEIPT AUDIT TRIAL \n");
	fprintf (fout, ".E COMPANY : %s %s \n",
			clip (comm_rec.co_no), clip (comm_rec.co_name));
	fprintf (fout, ".E BRANCH : %s %s \n",
			clip (comm_rec.est_no), clip (comm_rec.est_name));
	fprintf (fout, ".E WAREHOUSE : %s %s \n",
			clip (comm_rec.cc_no), clip (comm_rec.cc_name));
	fprintf (fout, ".B1\n");
	
	fprintf (fout, ".R===========");
	fprintf (fout, "=====");
	fprintf (fout, "=================");
	fprintf (fout, "=========================================");
	fprintf (fout, "=====");
	fprintf (fout, "============");
	fprintf (fout, "========");
	fprintf (fout, "========");
	fprintf (fout, "===========");
	fprintf (fout, "===========");

	fprintf (fout, "===========================\n");
	fprintf (fout, "===========");
	fprintf (fout, "=====");
	fprintf (fout, "=================");
	fprintf (fout, "=========================================");
	fprintf (fout, "=====");
	fprintf (fout, "============");
	fprintf (fout, "========");
	fprintf (fout, "========");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "===========================\n");

	fprintf (fout, "| RECEIPT  ");
	fprintf (fout, "| QC ");
	fprintf (fout, "|  ITEM  NUMBER  ");
	fprintf (fout, "|            ITEM DESCRIPTION            ");
	fprintf (fout, "|UOM.");
	fprintf (fout, "|    QC     ");
	fprintf (fout, "|LOT  NO");
	fprintf (fout, "|SLOT NO");
	fprintf (fout, "|  EXPIRY  ");
	fprintf (fout, "|  QC LOC  ");
	fprintf (fout, "|      SERIAL NUMBER      |\n");

	fprintf (fout, "|   DATE   ");
	fprintf (fout, "|CNTR");
	fprintf (fout, "|                ");
	fprintf (fout, "|                                        ");
	fprintf (fout, "|UOM.");
	fprintf (fout, "| QUANTITY  ");
	fprintf (fout, "|       ");
	fprintf (fout, "|       ");
	fprintf (fout, "|   DATE   ");
	fprintf (fout, "|          ");
	fprintf (fout, "|                         |\n");

	fprintf (fout, "|----------");
	fprintf (fout, "|----");
	fprintf (fout, "|----------------");
	fprintf (fout, "|----------------------------------------");
	fprintf (fout, "|----");
	fprintf (fout, "|-----------");
	fprintf (fout, "|-------");
	fprintf (fout, "|-------");
	fprintf (fout, "|----------");
	fprintf (fout, "|----------");
	fprintf (fout, "|-------------------------|\n");
}

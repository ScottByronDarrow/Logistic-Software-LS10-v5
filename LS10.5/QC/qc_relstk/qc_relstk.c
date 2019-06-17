/*=====================================================================
|  Copyright (C) 1999 - 2000 LogisticSoftware                         |
|=====================================================================|
| $Id: qc_relstk.c,v 5.4 2002/07/24 08:39:09 scott Exp $
|  Program Name  : (qc_relstk.c )                                   |
|  Program Desc  : (QC Release Of Stock Program.              )     |
|---------------------------------------------------------------------|
|  Date Written  : 22/07/94        | Author       : Aroha Merrilees.  |
|---------------------------------------------------------------------|
| $Log: qc_relstk.c,v $
| Revision 5.4  2002/07/24 08:39:09  scott
| Updated to ensure SetSortArray () is after set_masks
|
| Revision 5.3  2002/07/03 04:26:11  scott
| Updated to add SetSortArray () for new LS10-GUI screen features
|
| Revision 5.2  2001/08/09 09:16:28  scott
| Updated to add FinishProgram () function
|
| Revision 5.1  2001/08/06 23:37:55  scott
| RELEASE 5.0
|
=====================================================================*/
#define	CCMAIN
char	*PNAME = "$RCSfile: qc_relstk.c,v $",
		*PROG_VERSION = "@(#) - $Header: /usr/LS10/REPOSITORY/LS10.5/QC/qc_relstk/qc_relstk.c,v 5.4 2002/07/24 08:39:09 scott Exp $";

#define	MAXLINES	2000
#define MAXWIDTH	400
#define	TABLINES	8

#include	<pslscr.h>
#include	<std_decs.h>
#include	<GlUtils.h>
#include	<twodec.h>
#include	<hot_keys.h>
#include	<minimenu.h>
#include	<ml_qc_mess.h>
#include	<ml_std_mess.h>
#include	<tabdisp.h>
#include	<Costing.h>

#define		SR			store [line_cnt]
#define		RELEASE		(local_rec.progType [0] == 'R')
	
	float	ToStdUom (float);
	float	ToLclUom (float);

#include	"schema"

struct commRecord	comm_rec;
struct comrRecord	comr_rec;
struct prmrRecord	prmr_rec;
struct qcmrRecord	qcmr_rec;
struct inmrRecord	inmr_rec;
struct qchrRecord	qchr_rec;
struct qchrRecord	qchr2_rec;
struct qclnRecord	qcln_rec;
struct ccmrRecord	ccmr_rec;
struct inccRecord	incc_rec;
struct inwuRecord	inwu_rec;
struct exwoRecord	exwo_rec;
struct sumrRecord	sumr_rec;
struct inumRecord	inum_rec;
struct inumRecord	inum2_rec;
struct inloRecord	inlo_rec;

	char	*data	= "data",
			*inum2	= "inum2",
			*qchr2	= "qchr2",
			*spaces9 = "         ",
			*zeros9  = "000000000";

	long	prevHhqcHash;

	FILE	*fout;

struct storeRec {
	long	hhbrHash;
	long	hhccHash;
	long	hhwhHash;
	long	hhumHash;
	char	uom [5];
	char	lotControl [2];
	int		decPt;
	float	quantity;
	float	cnvFct;
	float	stdCnvFct;
	long	hhqcHash;
	long	releaseDate;
	int		lineNo;
} store [MAXLINES];

static	int	DetailFunc	(int, KEY_TAB *);
static	int	LineFunc 	(int, KEY_TAB *);
static	int	ExitFunc 	(int, KEY_TAB *);
static	KEY_TAB	detailKeys [] =
{
	{ "",	'\r',	DetailFunc,
	  "Select Record",	"E" },
	{ "",	FN1,	ExitFunc,
	  "",				"E" },
	{ "",	FN16,	ExitFunc,
	  "",				"E" },
	END_KEYS
};
static	KEY_TAB	lineKeys [] =
{
	{ "",	'\r',	LineFunc,
	  "Select Record",	"E" },
	{ "",	FN1,	ExitFunc,
	  "",				"E" },
	{ "",	FN16,	ExitFunc,
	  "",				"E" },
	END_KEYS
};

/*
 * Local & Screen Structures. 
 */
struct {
	char	dummy [11];
	char	loc_curr [4];
	char	sysDate [11];
	long	lsysDate;
	int		printerNumber;
	char	progType [2];

	long	relDate;

	char	itemNo [17];
	float	relQty;
	float	rejQty;
	char	reason [3];
	char	coa [2];
	long	reassayDt;
	char	remarks [31];
	long	releaseDate;

	char	itemDesc [41];
	char	supName [41];
	char	reasonDesc [21];
	char	reasonWofAcc [sizeof glmrRec.acc_no];
	char	sourceType [15];
	long	recptDt;
	long	expDt;
	float	originQty;
	float	orgRelQty;
	float	orgRejQty;

	long	hhqcHash;
	char	stkWofAcc [sizeof glmrRec.acc_no];
	double	cost;
	int		lineNo;
	int		expiryPrd;
	char	UOM [5];
	char	LL [2];
} local_rec;

enum {SCN_HEADER = 1, SCN_DETAILS};
static	struct	var	vars [] =
{
	{SCN_HEADER, LIN, "relDate", 	 2, 15, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", local_rec.sysDate, " Release Date :", "QC Release Date - Defaults To Todays Date", 
		 NE, NO, JUSTRIGHT, "", "", (char *)&local_rec.relDate}, 
	{SCN_HEADER, LIN, "empCode", 	 3, 15, CHARTYPE, 
		"UUUUUUUU", "          ", 
		" ", "", " Employee     :", "Employee Code - Search available", 
		 NE, NO,  JUSTLEFT, "", "", prmr_rec.code}, 
	{SCN_HEADER, LIN, "empName", 	 3, 35, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", " ", "", " ", 
		 NA, NO,  JUSTLEFT, "", "", prmr_rec.name}, 
	{SCN_HEADER, LIN, "qcCentre",	 4, 15, CHARTYPE, 
		"UUUU", "          ", 
		" ", "", " QC Centre    :", "QC Centre", 
		 NE, NO,  JUSTLEFT, "", "", qcmr_rec.centre}, 

	{SCN_DETAILS, TAB, "itemNo", 		MAXLINES, 0, CHARTYPE, 
		"UUUUUUUUUUUUUUUU", "          ", 
		" ", " ", "  Item Number   ", "QC Item Number", 
		YES, NO, JUSTLEFT, "", "", local_rec.itemNo}, 
	{SCN_DETAILS, TAB, "UOM", 	 0, 0, CHARTYPE, 
		"AAAA", "          ", 
		" ", "", "UOM.", "", 
		NA, NO, JUSTLEFT, "", "", local_rec.UOM}, 
	{SCN_DETAILS, TAB, "relQty",	0, 0, FLOATTYPE, 
		"NNNNNNN.NNN", "          ", 
		" ", " ", "Rel. Quantity", "Released Quantity - Defaults To Original Receipted Quantity", 
		YES, NO, JUSTRIGHT, "0.000", "999999.999", (char *)&local_rec.relQty}, 
	{SCN_DETAILS, TAB, "rejQty",	0, 0, FLOATTYPE, 
		"NNNNNNN.NNN", "          ", 
		" ", "0.00", "Rej. Quantity", "Rejected Quantity - Defaults To The Balance Between Original and Released Quantities", 
		YES, NO, JUSTRIGHT, "0.000", "999999.999", (char *)&local_rec.rejQty},
	{SCN_DETAILS, TAB, "LL", 	 0, 0, CHARTYPE, 
		"U", "          ", 
		" ", " ", "L", "Lot / Location Selection. <return> ", 
		 YES, NO,  JUSTLEFT, "", "", local_rec.LL}, 
	{SCN_DETAILS, TAB, "reason",	0, 2, CHARTYPE, 
		"UU", "          ", 
		" ", "", "Reason", "Reason Code For Rejection", 
		YES, NO, JUSTLEFT, "", "", local_rec.reason}, 
	{SCN_DETAILS, TAB, "coa",	0, 2, CHARTYPE, 
		"U", "          ", 
		" ", "N", "Cert Anal.", "Certificate Of Analysis - Defaults To No", 
		YES, NO, JUSTLEFT, "NY", "", local_rec.coa}, 
	{SCN_DETAILS, TAB, "reassayDt", 	 0, 1, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", " ", "Next Rel Date", "Next Reassay Date - Default To Release Date Plus First Expiry Period", 
		YES, NO, JUSTRIGHT, "", "", (char *)&local_rec.reassayDt}, 
	{SCN_DETAILS, TAB, "remarks", 	 0, 0, CHARTYPE,
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ",
		" ", " ", "            Remarks           ", "Release Remarks",
		YES, NO, JUSTLEFT, "", "", local_rec.remarks}, 
	{SCN_DETAILS, TAB, "releaseDate", 	 0, 2, EDATETYPE,
		"DD/DD/DD", "          ",
		" ", " ", "Release Date", "Release Date",
		ND, NO, JUSTRIGHT, "", "", (char *)&local_rec.releaseDate}, 
	{SCN_DETAILS, TAB, "itemDesc",		0, 0, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		ND, NO, JUSTLEFT, "", "", local_rec.itemDesc}, 
	{SCN_DETAILS, TAB, "supName", 	 0, 0, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		ND, NO, JUSTLEFT, "", "", local_rec.supName},
	{SCN_DETAILS, TAB, "reasonDesc",	0, 0, CHARTYPE, 
		"AAAAAAAAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		ND, NO, JUSTLEFT, "", "", local_rec.reasonDesc}, 
	{SCN_DETAILS, TAB, "reasonWofAcc",	0, 0, CHARTYPE, 
		GlMask, "          ", 
		"0", "", "", "", 
		ND, NO, JUSTLEFT, "", "", local_rec.reasonWofAcc}, 
	{SCN_DETAILS, TAB, "sourceType", 	 0, 0, CHARTYPE, 
		"AAAAAAAAAAAAAA", "          ", 
		" ", "", "", "", 
		ND, NO, JUSTLEFT, "", "", local_rec.sourceType}, 
	{SCN_DETAILS, TAB, "recptDt", 	 0, 0, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", "", "", "", 
		ND, NO, JUSTRIGHT, "", "", (char *)&local_rec.recptDt}, 
	{SCN_DETAILS, TAB, "expDt", 	 0, 0, EDATETYPE, 
		"DD/DD/DD", "          ", 
		" ", "", "", "", 
		ND, NO, JUSTRIGHT, "", "", (char *)&local_rec.expDt}, 
	{SCN_DETAILS, TAB, "originQty",	0, 0, FLOATTYPE, 
		"NNNNNNN.NNN", "          ", 
		" ", "", "", "", 
		ND, NO, JUSTRIGHT, "", "", (char *)&local_rec.originQty}, 
	{SCN_DETAILS, TAB, "orgRelQty",	0, 0, FLOATTYPE, 
		"NNNNNNN.NNN", "          ", 
		" ", "", "", "", 
		ND, NO, JUSTRIGHT, "", "", (char *)&local_rec.orgRelQty}, 
	{SCN_DETAILS, TAB, "orgRejQty",	0, 0, FLOATTYPE, 
		"NNNNNNN.NNN", "          ", 
		" ", "", "", "", 
		ND, NO, JUSTRIGHT, "", "", (char *)&local_rec.orgRejQty}, 
	{SCN_DETAILS, TAB, "hhqcHash", 	0, 0, LONGTYPE, 
		"NNNNNNNNNN", "          ", 
		" ", "", "", "", 
		ND, NO, JUSTRIGHT, "", "", (char *)&local_rec.hhqcHash}, 
	{SCN_DETAILS, TAB, "stkWofAcc",	0, 0, CHARTYPE, 
		GlMask, "          ", 
		"0", "", "", "", 
		ND, NO, JUSTLEFT, "", "", local_rec.stkWofAcc}, 
	{SCN_DETAILS, TAB, "cost",	0, 0, MONEYTYPE, 
		"NNNNNNN.NN", "          ", 
		"", "", "", "", 
		ND, NO, JUSTRIGHT, "", "", (char *)&local_rec.cost}, 
	{SCN_DETAILS, TAB, "lineNo", 	0, 0, INTTYPE, 
		"NNN", "          ", 
		" ", "", "", "", 
		ND, NO, JUSTRIGHT, "", "", (char *)&local_rec.lineNo}, 
	{SCN_DETAILS, TAB, "expiryPrd", 	0, 0, INTTYPE, 
		"NNN", "          ", 
		" ", "", "", "", 
		ND, NO, JUSTRIGHT, "", "", (char *)&local_rec.expiryPrd}, 

	{0, LIN, "", 	 0, 0, CHARTYPE, 
		"A", "          ", 
		" ", "", "dummy", " ", 
		YES, NO, JUSTRIGHT, "", "", local_rec.dummy}, 
};

void	OpenDB 			(void);
void	CloseDB 		(void);
int		heading 		(int);
void	PrintCoStuff 	(void);
int		spec_valid 		(int);
int		DeleteItem 		(void);
int		GetAccounts 	(void);
int		LoadDetails 	(void);
int		FindRecords 	(int);
int		CheckStore 		(int);
int		GetLineDetails 	(void);
int		DetailFunc 		(int, KEY_TAB *);
int		LineFunc 		(int, KEY_TAB *);
int		ExitFunc 		(int, KEY_TAB *);
int		ValidQuantity 	(double, int);
void	tab_other 		(int);
void	DisplayDetails 	(int);
void	Update 			(void);
void	SrchPrmr 		(char *);
void	SrchQcmr 		(char *);
void	SrchExwo 		(char *);
void	SrchInum 		(char *);
void	PrintHeading 	(void);
void	PrintDetails 	(void);
float	ToStdUom 		(float);
float	ToLclUom 		(float);
char	location [10];

#include	<LocHeader.h>

/*
 * Main Processing Routine 
 */
int
main (
 int	argc,
 char *	argv [])
{
	char *	sptr;
	int		QC_APPLY;

	QC_APPLY = (sptr = chk_env ("QC_APPLY")) ? atoi (sptr) : 0;
	if (!QC_APPLY)
	{
		/*
		 * QC_APPLY not set for this program to be used 
		 */
		print_at (0,0,"%s", ML (mlQcMess001));
		return (EXIT_SUCCESS);
	}

	if (argc != 3)
	{
		print_at (0,0, ML (mlQcMess700), argv [0]);
		return (argc);
	}
	local_rec.printerNumber = atoi (argv [1]);
	strcpy (local_rec.progType, argv [2]);
	if (local_rec.progType [0] != 'M' &&
		local_rec.progType [0] != 'R')
	{
		print_at (0,0, ML (mlQcMess700), argv [0]);
		return (argc);
	}
	OpenDB ();

	/*
	 * Setup required parameters. 
	 */
	SETUP_SCR (vars);

	init_scr ();
	set_tty ();
	set_masks ();
/* 
 * Required to ensure store structure is sorted when lines sorted.
 */
#ifdef GVISION
	SetSortArray (SCN_DETAILS, store, sizeof (struct storeRec));
#endif

	FLD ("LL") = ND;
	if (SK_BATCH_CONT || MULT_LOC)
		FLD ("LL") = YES;

	strcpy (local_rec.sysDate, DateToString (TodaysDate ()));
	local_rec.lsysDate = TodaysDate ();

	if (!RELEASE)
	{
		int		cnt;

		for (cnt = label ("relQty"); cnt < label ("itemDesc"); cnt ++)
			if (vars [cnt].required != ND)
				vars [cnt].required = NI;

		FLD ("releaseDate") = NI;
		vars [label ("relDate")].prmpt 	= strdup (ML ("Trans Date   "));
		vars [label ("relDate")].comment = strdup (ML ("QC Trans Date - Defaults To Todays Date"));
	}

	/* Open print file. */
	if ((fout = popen ("pformat", "w")) == 0)
		sys_err ("Error in pformat during (POPEN)", errno, PNAME);

	PrintHeading ();

	/*
	 * Beginning of input control loop . 
	 */
	while (!prog_exit)
	{
		/*
		 * Reset control flags . 
		 */
		entry_exit	= FALSE;
		edit_exit	= FALSE;
		prog_exit	= FALSE;
		restart		= FALSE;
		search_ok	= TRUE;
		lcount [SCN_DETAILS] = 0;
		init_vars (SCN_HEADER);
		init_vars (SCN_DETAILS);

		heading (SCN_HEADER);
		entry (SCN_HEADER);
		if (prog_exit || restart)
			continue;

		line_cnt = 0;
		heading (SCN_DETAILS);
		entry (SCN_DETAILS);
		if (restart)
			continue;

		heading (SCN_DETAILS);
		scn_display (SCN_DETAILS);
		edit (SCN_DETAILS);
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

#include	<MoveRec.h>
#include	<MoveAdd.h>

/*
 * Open data base files 
 */
void
OpenDB (void)
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
		sprintf (local_rec.loc_curr, "%-3.3s", get_env ("CURR_CODE"));
	else
		sprintf (local_rec.loc_curr, "%-3.3s", comr_rec.base_curr);
	abc_fclose (comr);

	abc_alias (qchr2, qchr);
	abc_alias (inum2, inum);

	open_rec (prmr,  prmr_list, PRMR_NO_FIELDS, "prmr_id_no");
	open_rec (qcmr,  qcmr_list, QCMR_NO_FIELDS, "qcmr_id_no");
	open_rec (inmr,  inmr_list, INMR_NO_FIELDS, "inmr_id_no");
	open_rec (inum,  inum_list, INUM_NO_FIELDS, "inum_uom");
	open_rec (inum2, inum_list, INUM_NO_FIELDS, "inum_id_no2");
	open_rec (qchr,  qchr_list, QCHR_NO_FIELDS, "qchr_id_no");
	open_rec (qchr2, qchr_list, QCHR_NO_FIELDS, "qchr_hhqc_hash");
	open_rec (qcln,  qcln_list, QCLN_NO_FIELDS, "qcln_id_no");
	open_rec (ccmr,  ccmr_list, CCMR_NO_FIELDS, "ccmr_id_no");
	open_rec (incc,  incc_list, INCC_NO_FIELDS, "incc_id_no");
	open_rec (exwo,  exwo_list, EXWO_NO_FIELDS, "exwo_id_no");
	open_rec (sumr,  sumr_list, SUMR_NO_FIELDS, "sumr_hhsu_hash");
	open_rec (inlo,  inlo_list, INLO_NO_FIELDS, "inlo_inlo_hash");
	open_rec ("move",move_list, MOVE_NO_FIELDS, "move_move_hash");

	strcpy (ccmr_rec.co_no, comm_rec.co_no);
	strcpy (ccmr_rec.est_no, comm_rec.est_no);
	strcpy (ccmr_rec.cc_no, comm_rec.cc_no);
	if ((cc = find_rec (ccmr, &ccmr_rec, EQUAL, "r")))
		file_err (cc, (char *)ccmr, "DBFIND");

	OpenLocation (ccmr_rec.hhcc_hash);
	IgnoreAvailChk	=	TRUE;
	LL_EditLoc	=	TRUE;
	LL_EditLot	=	TRUE;
	LL_EditDate	=	TRUE;
	LL_EditSLot	=	TRUE;
	strcpy (llctAutoAll, "N");

	LLInputClear	=	FALSE;

	OpenGlmr ();
	OpenGlwk ();
	GL_OpenBatch (comm_rec.co_no, comm_rec.est_no);
}

/*
 * Close data base files 
 */
void
CloseDB (void)
{
	abc_fclose (prmr);
	abc_fclose (qcmr);
	abc_fclose (inmr);
	abc_fclose (inum);
	abc_fclose (inum2);
	abc_fclose (qchr);
	abc_fclose (qchr2);
	abc_fclose (qcln);
	abc_fclose (ccmr);
	abc_fclose (incc);
	abc_fclose (exwo);
	abc_fclose (sumr);
	abc_fclose (inlo);
	abc_fclose ("move");
	CloseLocation ();
	GL_CloseBatch (local_rec.printerNumber);
	GL_Close ();

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

		/*
		 * QC Stock Release 
		 */
		if (RELEASE)
			sprintf (err_str, " %s ", ML (mlQcMess015));
		else
		{
			/*
			 * Maintenance Of QC Released Stock 
			 */
			sprintf (err_str, " %s ", ML (mlQcMess016));
		}
		rv_pr (err_str, (132 - strlen (err_str)) / 2, 0, 1);
		move (0, 1); line (132);

		if (scn == SCN_HEADER)
			box (0, 1, 132, 3);

		if (scn == SCN_DETAILS)
		{
			heading (SCN_HEADER);
			scn_display (SCN_HEADER);

			tab_col = 0;
			tab_row = 10;
			box (0, 5, 132, 14);
			move (0, 5); PGCHAR (10);
			move (131, 5); PGCHAR (11);
/*
			if (!RELEASE)
			{
				move (0, 11); PGCHAR (10);
			}
			if (MULT_LOC && !RELEASE)
			{
				move (132, 11); PGCHAR (11);
			}
*/
			DisplayDetails (line_cnt);
		}
		PrintCoStuff ();

		

		/*  reset this variable for new screen NOT page	*/
		scn_write (scn);
	}
	return (EXIT_SUCCESS);
}

void
PrintCoStuff (void)
{
	move (0, 21); line (132);
	sprintf (err_str, ML (mlStdMess038), comm_rec.co_no, comm_rec.co_short);
	print_at (22, 0, "%s", err_str);
	print_at (22, 31,ML (mlStdMess039), comm_rec.est_no, comm_rec.est_short); 
	print_at (22, 60,ML (mlStdMess099), comm_rec.cc_no, comm_rec.cc_short);

}

int
spec_valid (
 int	field)
{
	int		TempLine;

	if (LCHECK ("empCode"))
	{
		if (last_char == FN16)
		{
			prog_exit = TRUE;
			return (EXIT_SUCCESS);
		}

		if (SRCH_KEY)
		{
			SrchPrmr (temp_str);
			return (EXIT_SUCCESS);
		}

		strcpy (prmr_rec.co_no, comm_rec.co_no);
		strcpy (prmr_rec.br_no, comm_rec.est_no);
		cc = find_rec (prmr, &prmr_rec, EQUAL, "r");
		if (cc)
		{
			/*
			 * Employee Not found. 
			 */
			print_mess (ML (mlStdMess053));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		DSP_FLD ("empName");
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
			return (EXIT_FAILURE);

		strcpy (qcmr_rec.co_no, comm_rec.co_no);
		strcpy (qcmr_rec.br_no, comm_rec.est_no);
		if (find_rec (qcmr, &qcmr_rec, EQUAL, "r"))
		{
			/*
			 * QC Centre Not found. 
			 */
			print_mess (ML (mlStdMess131));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (location, qcmr_rec.centre);
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("itemNo"))
	{

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
			/*
			 * Item Not found. 
			 */
			print_mess (ML (mlStdMess001));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		SuperSynonymError ();

		if (inmr_rec.qc_reqd [0] != 'Y')
		{
			/* 
			 * QC Not Required 
			 */
			print_mess (ML (mlQcMess003));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		cc = FindInei (inmr_rec.hhbr_hash, comm_rec.est_no, "r");
		if (cc)
		{
			/*
			 * Item not found. 
			 */
			print_mess (ML (mlStdMess001));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		SR.hhbrHash	=	inmr_rec.hhbr_hash;
		strcpy (inum_rec.uom, inmr_rec.sale_unit);
		cc = find_rec (inum, &inum_rec, COMPARISON, "r");
		if (cc)
			file_err (cc, (char *)inum, "DBFIND");

		SR.stdCnvFct	=	inum_rec.cnv_fct;
		if (SR.stdCnvFct == 0.00)
			SR.stdCnvFct = 1.00;

		if (!LoadDetails ())
		{
			/*
			 * No QC Records For Release 
			 */
			if (RELEASE)
				sprintf (err_str, ML (mlQcMess007));
			else
			{
				/*
				 * No QC Records To Maintain 
				 */
				sprintf (err_str, ML (mlQcMess008));
			}
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		if (restart)
			return (EXIT_SUCCESS);

		if (!GetAccounts ())
			return (EXIT_FAILURE);

		if (!RELEASE)
		{
			DSP_FLD ("itemNo");
			DSP_FLD ("relQty");
			DSP_FLD ("rejQty");
			DSP_FLD ("reason");
			DSP_FLD ("coa");
			DSP_FLD ("reassayDt");
			DSP_FLD ("remarks");
			DSP_FLD ("releaseDate");
		}
		if (local_rec.expiryPrd)
		{
			long	reassayDt;
			int		mth, cnt,
					mntNoDays [] = {31, 28, 31, 30, 31, 30,
									31, 31, 30, 31, 30, 31, 0};

			DateToDMY (local_rec.relDate, NULL, &mth, NULL);
			reassayDt = local_rec.relDate;
			for (cnt = 0; cnt < local_rec.expiryPrd; cnt++)
			{
				mth = (mth + cnt) % 12;
				reassayDt += (long) (mntNoDays [mth]);
			}
			local_rec.reassayDt	= reassayDt;
		}

		tab_other (line_cnt);
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("relQty"))
	{
		if (dflt_used)
			local_rec.relQty = 	local_rec.originQty -
								local_rec.orgRelQty -
								local_rec.orgRejQty;

		if (prog_status != ENTRY &&
			local_rec.relQty == 0.00 &&
			local_rec.rejQty == 0.00)
		{
			int		i;

			DSP_FLD ("relQty");

			i = prmptmsg (ML (mlStdMess151), "YyNn", 10, 23);
			if (i == 'Y' || i == 'y') 
				return (DeleteItem ());
		}

		local_rec.relQty = local_rec.relQty;
		DSP_FLD ("relQty");
		if (!ValidQuantity (local_rec.relQty, SR.decPt))
			return (EXIT_FAILURE);

		if (local_rec.relQty > local_rec.originQty)
		{
			/*
			 * Release Qty %f Greater Then Original Qty %f 
			 */
			sprintf (err_str, ML (mlQcMess018),local_rec.relQty, 
											  local_rec.originQty);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (RELEASE)
		{
			float	releaseQty;

			releaseQty = local_rec.relQty +
						 local_rec.orgRelQty +
						 local_rec.orgRejQty;
			if (releaseQty > local_rec.originQty)
			{
				sprintf (err_str, ML (mlQcMess009), releaseQty, 
												local_rec.originQty);
				print_mess (err_str);
				sleep (sleepTime);
				clear_mess ();
				return (EXIT_FAILURE);
			}
		}
		SR.quantity	=	local_rec.relQty + local_rec.rejQty;

		return (EXIT_SUCCESS);
	}

	if (LCHECK ("rejQty"))
	{
		float	releaseQty;

		if (FLD ("rejQty") == NA)
			return (EXIT_SUCCESS);

		if (prog_status != ENTRY &&
			local_rec.relQty == 0.00 &&
			local_rec.rejQty == 0.00)
		{
			int		i;

			DSP_FLD ("rejQty");

/*" Are You sure You Want To Delete This Line ? (Yes/No) ");*/
			i = prmptmsg (ML (mlStdMess151), "YyNn", 10, 23);
			if (i == 'Y' || i == 'y') 
				return (DeleteItem ());
		}

		if (dflt_used)
		{
			strcpy (local_rec.reason,		" ");
			strcpy (local_rec.reasonDesc,	" ");
			strcpy (local_rec.reasonWofAcc,	" ");
			local_rec.cost = 0.00;
			FLD ("reason") = NA;
			DSP_FLD ("reason");
			DisplayDetails (line_cnt);

			return (EXIT_SUCCESS);
		}

		local_rec.rejQty = local_rec.rejQty;
		DSP_FLD ("rejQty");
		if (!ValidQuantity (local_rec.rejQty, SR.decPt))
			return (EXIT_FAILURE);

		releaseQty = local_rec.relQty + local_rec.rejQty;
		if (releaseQty > local_rec.originQty)
		{
			sprintf (err_str, ML (mlQcMess010),  releaseQty, 
												local_rec.originQty);
			print_mess (err_str);
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}

		if (local_rec.rejQty > 0.00)
		{
			if (RELEASE)
			{
				releaseQty = local_rec.rejQty + 
							 local_rec.relQty +
							 local_rec.orgRelQty +
							 local_rec.orgRejQty;

				if (releaseQty > local_rec.originQty)
				{
					sprintf (err_str, ML (mlQcMess009), releaseQty, 
												local_rec.originQty);
					print_mess (err_str);
					sleep (sleepTime);
					clear_mess ();
					return (EXIT_FAILURE);
				}
			}

			if (RELEASE)
				FLD ("reason") = YES;
			else
				FLD ("reason") = NI;
			if (prog_status != ENTRY)
			{
				/* prompt for reason and wof account */
				do {
					get_entry (label ("reason"));
					cc = spec_valid (label ("reason"));
					DSP_FLD ("reason");
				} while (cc && !restart);
				if (restart)
					return (EXIT_SUCCESS);
			}

			switch (inmr_rec.costing_flag [0])
			{
				case	'A':
				case	'L':
				case	'P':
				case	'T':
					local_rec.cost	=	FindIneiCosts
										(
											inmr_rec.costing_flag,
											comm_rec.est_no,
											inmr_rec.hhbr_hash
										);
				break;
										
				case	'F':
					local_rec.cost = FindIncfCost 
									(
										SR.hhwhHash,
										incc_rec.closing_stock,
										local_rec.rejQty,
										TRUE,
										SR.decPt
									);
					if (local_rec.cost <= 0.00)
					{
						local_rec.cost	=	FindIneiCosts
											(
												"L",
												comm_rec.est_no,
												inmr_rec.hhbr_hash
											);
					}
				break;
			case	'I':
					local_rec.cost = FindIncfCost 
									(
										SR.hhwhHash,
										incc_rec.closing_stock,
										local_rec.rejQty,
										FALSE,
										SR.decPt
									);
					if (local_rec.cost <= 0.00)
					{
						local_rec.cost	=	FindIneiCosts
											(
												"L",
												comm_rec.est_no,
												inmr_rec.hhbr_hash
											);
					}
				break;
			case	'S':
				local_rec.cost	=	FindInsfValue (SR.hhwhHash, TRUE);
			}
		}
		else
		{
			/*
			 * Rejection quantity equals zero, therefore no need to enter reason
			 */
			strcpy (local_rec.reason,		" ");
			strcpy (local_rec.reasonDesc,	" ");
			strcpy (local_rec.reasonWofAcc,	" ");
			local_rec.cost = 0.00;
			FLD ("reason") = NA;
			DSP_FLD ("reason");
		}
		SR.quantity	=	local_rec.relQty + local_rec.rejQty;
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("reason"))
	{
		if (FLD ("reason") == NA)
			return (EXIT_SUCCESS);

		if (SRCH_KEY)
		{
			SrchExwo (temp_str);
			return (EXIT_SUCCESS);
		}

		if (dflt_used)
			return (EXIT_FAILURE);

		strcpy (exwo_rec.co_no, comm_rec.co_no);
		strcpy (exwo_rec.code, local_rec.reason);
		if (find_rec (exwo, &exwo_rec, EQUAL, "r"))
		{
			/*-------------------------
			| Reason Code Not found. |
			-------------------------*/
			print_mess (ML (mlStdMess163));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		strcpy (local_rec.reasonDesc, exwo_rec.description);

		if (find_hash (glmr, &glmrRec, EQUAL, "r", exwo_rec.hhmr_hash))
		{
			/*--------------------
			| Account not found. |
			--------------------*/
			print_mess (ML (mlStdMess186));
			sleep (sleepTime);
			clear_mess ();
			return (EXIT_FAILURE);
		}
		sprintf (local_rec.reasonWofAcc, "%-*.*s", 
								MAXLEVEL,MAXLEVEL,glmrRec.acc_no);
		
		tab_other (line_cnt);
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("reassayDt"))
	{
		if (dflt_used)
		{
			if (local_rec.expiryPrd)
			{
				long	reassayDt;
				int		mth, cnt,
						mntNoDays [] = {31, 28, 31, 30, 31, 30,
										31, 31, 30, 31, 30, 31, 0};

				DateToDMY (local_rec.relDate, NULL, &mth, NULL);
				reassayDt = local_rec.relDate;
				for (cnt = 0; cnt < local_rec.expiryPrd; cnt++)
				{
					mth = (mth + cnt) % 12;
					reassayDt += (long) (mntNoDays [mth]);
				}
				local_rec.reassayDt	= reassayDt;
			}
			else
				local_rec.reassayDt	= 0L;
		}
		return (EXIT_SUCCESS);
	}

	if (LCHECK ("expiryPrd")) /* Last field in the TAB screen. */
	{
		store [line_cnt].hhqcHash		= local_rec.hhqcHash;
		if (RELEASE)
		{
			store [line_cnt].releaseDate	= 0L;
			store [line_cnt].lineNo			= 0;
		}
		else
		{
			store [line_cnt].releaseDate	= local_rec.releaseDate;
			store [line_cnt].lineNo			= local_rec.lineNo;
		}
		if (prog_status == ENTRY)
			lcount [SCN_DETAILS] ++;
		putval (line_cnt);

		return (EXIT_SUCCESS);
	}

	/*
	 * Validate lots and locations. 
	 */
	if (LCHECK ("LL"))
	{
		char	tempDateStr [11];

		if (FLD ("LL") == ND)
			return (EXIT_SUCCESS);

		if (prog_status	== ENTRY)
		{
			inlo_rec.inlo_hash	=	qchr_rec.inlo_hash;
			cc = find_rec (inlo, &inlo_rec, COMPARISON, "r");
			if (!cc)
			{
				ClearLotsInfo (line_cnt);
		
				strcpy (tempDateStr, DateToString (inlo_rec.expiry_date));
				LLAddList
			 	(
					inlo_rec.hhwh_hash,
					inlo_rec.hhum_hash,
					incc_rec.hhcc_hash,
					inlo_rec.inlo_hash,
					inlo_rec.location,
					inlo_rec.lot_no,
					inlo_rec.loc_type,
					inlo_rec.slot_no,
					tempDateStr,
					inlo_rec.uom,
					inlo_rec.qty / inlo_rec.cnv_fct,
					inum_rec.uom,
					SR.quantity,
					inlo_rec.cnv_fct,
					SR.cnvFct,
					inlo_rec.date_create,
					inlo_rec.loc_status,
					inlo_rec.pack_qty,
					inlo_rec.chg_wgt,
					inlo_rec.gross_wgt,
					inlo_rec.cu_metre,
					inlo_rec.sknd_hash
				);
				qtyRequired	=	SR.quantity;
				ProcList (line_cnt, (float) 0.00);
			}
		}
		TempLine	=	lcount [SCN_DETAILS];
		cc = DisplayLL
			(										/*----------------------*/
				line_cnt,							/*	Line number.		*/
				tab_row + 3 + (line_cnt % TABLINES),/*  Row for window		*/
				tab_col + 22,						/*  Col for window		*/
				4,									/*  length for window	*/
				SR.hhwhHash, 						/*	Warehouse hash.		*/
				SR.hhumHash,						/*	UOM hash			*/
				SR.hhccHash,						/*	CC hash.			*/
				SR.uom,							/* UOM					*/
				SR.quantity,							/* Quantity.			*/
				SR.cnvFct,						/* Conversion factor.	*/
				TodaysDate (),						/* Expiry Date.			*/
				FALSE,								/* Silent mode			*/
				FALSE,								/* Input Mode.			*/
				SR.lotControl						/* Lot controled item. 	*/
													/*----------------------*/
			);
		/*-----------------
		| Redraw screens. |
		-----------------*/
		putval (line_cnt);

		lcount [SCN_DETAILS] = (line_cnt + 1 > lcount [SCN_DETAILS]) 
								? line_cnt + 1 : lcount [SCN_DETAILS];
		scn_write (SCN_DETAILS);
		scn_display (SCN_DETAILS);
		lcount [SCN_DETAILS] = TempLine;
		box (0, 5, 132, 14);
		PrintCoStuff ();
		if (cc)
			return (EXIT_FAILURE);
		
		return (EXIT_SUCCESS);
	}
	return (EXIT_SUCCESS);             
}

int
DeleteItem (void)
{
	int		cnt,
			this_page = line_cnt / TABLINES;

	if (prog_status == ENTRY)
	{
		/*
		 * Cannot Delete Lines on Entry 
		 */
		print_mess (ML (mlStdMess005));
		return (EXIT_FAILURE);
	}

	lcount [SCN_DETAILS]--;
	for (cnt = line_cnt; line_cnt < lcount [SCN_DETAILS]; line_cnt++)
	{
		getval (line_cnt + 1);
		store [line_cnt].hhqcHash		= local_rec.hhqcHash;
		store [line_cnt].releaseDate	= local_rec.releaseDate;
		store [line_cnt].lineNo			= local_rec.lineNo;
		putval (line_cnt);
		if (this_page == line_cnt / TABLINES)
			line_display ();
	}
	if (this_page == line_cnt / TABLINES)
	{
		blank_display ();
		store [line_cnt].hhqcHash		= 0L;
		store [line_cnt].releaseDate	= 0L;
		store [line_cnt].lineNo			= 0;
	}
	line_cnt = cnt;
	getval (line_cnt);

	return (EXIT_SUCCESS);
}

/*
 * Get control posting accounts.
 */
int
GetAccounts (void)
{

	GL_GLI 
	(
		comm_rec.co_no,
		comm_rec.est_no,
		comm_rec.cc_no,
		"COSTSALE C",
		" ",
		inmr_rec.category
	);
	strcpy (local_rec.stkWofAcc, glmrRec.acc_no);
	return (TRUE);
}

int
LoadDetails (void)
{
	int		recCount = 0;
	int		TempLine;

	abc_selfield ("inum","inum_hhum_hash");

	/*
	 * Check if more than one record. 
	 */
	recCount = FindRecords (TRUE);

	if (recCount > 1)
	{
		tab_open ("qchrTab", detailKeys, 11, 1, 5, FALSE);
		tab_add ("qchrTab",
				"# %-16.16s %4.4s %-11.11s %-11.11s %-11.11s %-10.10s %-10.10s %-7.7s %-10.10s %-7.7s %-3.3s",
					"  Item  Number  ", 
					"UOM.",
					"Origin. Qty",
					"Releas. Qty",		
					"Reject. Qty",
					"Recpt Date",		
					"Exp R Date",
					"Lot  No",	
					"ExpiryDate",		
					"Ref One",
					"Src");

		FindRecords (FALSE);

		tab_scan ("qchrTab");
		tab_close ("qchrTab",TRUE);
		if (restart)
			return (TRUE);

		TempLine	=	lcount [SCN_DETAILS];

		/*-----------------
		| Redraw screens. |
		-----------------*/
		putval (line_cnt);

		lcount [SCN_DETAILS] = (line_cnt + 1 > lcount [SCN_DETAILS]) 
									? line_cnt + 1 : lcount [SCN_DETAILS];
		scn_write (SCN_DETAILS);
		scn_display (SCN_DETAILS);
		lcount [SCN_DETAILS] = TempLine;
		box (0, 5, 132, 14);
		PrintCoStuff ();
	}
	else
	{
		if (recCount == 1)
		{
			if (find_hash (qchr2, &qchr_rec, GTEQ, "r", prevHhqcHash))
				return (FALSE);
		}
		else
			return (FALSE);
	}

	inum_rec.hhum_hash	=	qchr_rec.hhum_hash;
	cc = find_rec (inum, &inum_rec, EQUAL, "r");
	if (cc)
		file_err (cc, (char *)inum, "DBFIND");

	SR.cnvFct	= inum_rec.cnv_fct / SR.stdCnvFct;
	SR.hhumHash		= inum_rec.hhum_hash;
	strcpy (SR.uom, inum_rec.uom);
	strcpy (local_rec.UOM, inum_rec.uom);

	DSP_FLD ("UOM");

	if (!RELEASE)
	{
		if (!GetLineDetails ())
			return (FALSE);
	}

	incc_rec.hhcc_hash = ccmr_rec.hhcc_hash;
	incc_rec.hhbr_hash = inmr_rec.hhbr_hash;
	if ((cc = find_rec (incc, &incc_rec, EQUAL, "r")))
		file_err (cc, (char *)incc, "DBFIND");

	SR.hhwhHash	=	incc_rec.hhwh_hash;
	SR.hhccHash	=	incc_rec.hhcc_hash;
	if (qchr_rec.source_type [0] == 'P')
	{
		if ((find_hash (sumr, &sumr_rec, EQUAL, "r", qchr_rec.hhsu_hash)))
			strcpy (sumr_rec.crd_name, " ");
	}
	else
	{
		strcpy (sumr_rec.crd_name, " ");
	}

	SR.hhumHash				=	qchr_rec.hhum_hash;
	strcpy (local_rec.itemNo,		inmr_rec.item_no);
	if (RELEASE)
	{
		local_rec.relQty			= 0.00;
		SR.quantity						= 0.00;
		local_rec.rejQty			= 0.00;
		sprintf (local_rec.reason,	"%-2.2s", " ");
		sprintf (local_rec.coa,		"%-1.1s", " ");
		local_rec.reassayDt			= 0L;
		sprintf (local_rec.remarks,	"%-30.30s", " ");
	}
	else
	{
		local_rec.relQty			= ToLclUom (qcln_rec.rel_qty);
		SR.quantity					= ToLclUom (qcln_rec.rel_qty);
		local_rec.rejQty			= ToLclUom (qcln_rec.rej_qty);
		sprintf (local_rec.reason,	"%-2.2s", qcln_rec.reason);
		sprintf (local_rec.coa,		"%-1.1s", qcln_rec.coa);
		local_rec.reassayDt			= qcln_rec.reassay_date;
		sprintf (local_rec.remarks,	"%-30.30s", qcln_rec.remarks);
		local_rec.releaseDate		= qcln_rec.release_dt;
	}

	sprintf (local_rec.itemDesc,		"%-40.40s", inmr_rec.description);
	sprintf (local_rec.supName,			"%-40.40s", sumr_rec.crd_name);
	if (RELEASE)
	{
		sprintf (local_rec.reasonDesc,		"%-20.20s", " ");
		sprintf (local_rec.reasonWofAcc,	"%*.*s",MAXLEVEL,MAXLEVEL," ");
	}
	else
	{
		strcpy (exwo_rec.co_no, comm_rec.co_no);
		sprintf (exwo_rec.code, "%-2.2s", qcln_rec.reason);
		if ((cc = find_rec (exwo, &exwo_rec, COMPARISON, "r")))
			strcpy (exwo_rec.description, " ");

		sprintf (local_rec.reasonDesc,		"%-20.20s", exwo_rec.description);
		sprintf (local_rec.reasonWofAcc,	"%-*.*s",	
								MAXLEVEL, MAXLEVEL, qcln_rec.wof_acc);
	}
	sprintf (local_rec.sourceType,	"%-14.14s", "Manual Entry");

	if (qchr_rec.source_type [0] == 'P')
		sprintf (local_rec.sourceType,	"%-14.14s", "Purchase Order");

	if (qchr_rec.source_type [0] == 'W')
		sprintf (local_rec.sourceType,	"%-14.14s", "Works Order");

	local_rec.recptDt					= qchr_rec.receipt_dt;
	local_rec.expDt						= qchr_rec.exp_rel_dt;
	local_rec.originQty					= ToLclUom (qchr_rec.origin_qty);
	local_rec.orgRelQty					= ToLclUom (qchr_rec.rel_qty);
	local_rec.orgRejQty					= ToLclUom (qchr_rec.rej_qty);

	SR.hhbrHash		= inmr_rec.hhbr_hash;
	SR.hhwhHash		= incc_rec.hhwh_hash;
	SR.hhccHash		= incc_rec.hhcc_hash;

	local_rec.hhqcHash					= qchr_rec.hhqc_hash;
	if (RELEASE)
	{
		sprintf (local_rec.stkWofAcc,	"%*.*s",MAXLEVEL,MAXLEVEL," ");
		local_rec.cost					= 0.00;
	}
	else
	{
		sprintf (local_rec.stkWofAcc,	"%-*.*s",
								MAXLEVEL,MAXLEVEL,qcln_rec.stk_wof_acc);
		local_rec.cost					= qcln_rec.cost;
	}
	sprintf (SR.lotControl,	"%-1.1s", 	inmr_rec.lot_ctrl);
	SR.decPt			= inmr_rec.dec_pt;
	local_rec.lineNo 	= (RELEASE) ? 0 : qcln_rec.line_no;
	local_rec.expiryPrd	= ineiRec.expiry_prd1;

	return (TRUE);
}

int
FindRecords (
 int	flag)
{
	char	recptDate [11],
			expRelDate [11];
	int		recCount = 0,
			addRecord;
	float	releasedQty;

	strcpy (qchr_rec.co_no, comm_rec.co_no);
	strcpy (qchr_rec.br_no, comm_rec.est_no);
	strcpy (qchr_rec.wh_no, comm_rec.cc_no);
	strcpy (qchr_rec.qc_centre, location);
	qchr_rec.hhbr_hash = inmr_rec.hhbr_hash;
	qchr_rec.exp_rel_dt = 0L;
	cc = find_rec (qchr, &qchr_rec, GTEQ, "r");
	while (!cc &&
		!strcmp (qchr_rec.co_no, comm_rec.co_no) &&
		!strcmp (qchr_rec.br_no, comm_rec.est_no) &&
		!strcmp (qchr_rec.wh_no, comm_rec.cc_no) &&
		!strcmp (qchr_rec.qc_centre, location) &&
		qchr_rec.hhbr_hash == inmr_rec.hhbr_hash)
	{
		addRecord = FALSE;

		inum_rec.hhum_hash	=	qchr_rec.hhum_hash;
		cc = find_rec (inum, &inum_rec, EQUAL, "r");
		if (cc)
			file_err (cc, (char *)inum, "DBFIND");

		SR.cnvFct	= inum_rec.cnv_fct / SR.stdCnvFct;
		SR.hhumHash		= inum_rec.hhum_hash;
		strcpy (SR.uom, inum_rec.uom);
		strcpy (local_rec.UOM, inum_rec.uom);

		/*
		 * Do not display records that have been completed for RELEASE only
		 */
		releasedQty = ToLclUom (qchr_rec.rel_qty + qchr_rec.rej_qty);
		if (RELEASE && releasedQty >= ToLclUom (qchr_rec.origin_qty)) 
		{
			cc = find_rec (qchr, &qchr_rec, NEXT, "r");
			continue;
		}
		/*
		 * Do not display records that have not been released, not for RELEASE.
		 */
		if (!RELEASE && qchr_rec.next_seq == 0)
		{
			cc = find_rec (qchr, &qchr_rec, NEXT, "r");
			continue;
		}

		if (RELEASE)
		{
			if (CheckStore (TRUE))
				addRecord = TRUE;
		}
		else
		{
			qcln_rec.hhqc_hash = qchr_rec.hhqc_hash;
			qcln_rec.release_dt = 0L;
			qcln_rec.line_no = 0;
			cc = find_rec (qcln, &qcln_rec, GTEQ, "r");
			while (!cc && qcln_rec.hhqc_hash == qchr_rec.hhqc_hash)
			{
				if (!strcmp (qcln_rec.emp_code, prmr_rec.code))
				{
					if (CheckStore (FALSE))
					{
						addRecord = TRUE;
						break;
					}
				}
				cc = find_rec (qcln, &qcln_rec, NEXT, "r");
			}
		}

		if (addRecord)
		{
			if (flag)
			{
				prevHhqcHash = qchr_rec.hhqc_hash;
				recCount ++;
			}
			else
			{
				/* add record to list */
				strcpy (recptDate, DateToString (qchr_rec.receipt_dt));
				strcpy (expRelDate,DateToString (qchr_rec.exp_rel_dt));
				inlo_rec.inlo_hash	=	qchr_rec.inlo_hash;
				cc = find_rec (inlo, &inlo_rec, COMPARISON, "r");
				if (cc)
					memset (&inlo_rec, 0, sizeof (inlo_rec));
		
				tab_add ("qchrTab",
						" %-16.16s %4.4s %11.3f %11.3f %11.3f %-10.10s %-10.10s %-7.7s %-10.10s %-7.7s  %-1.1s     %10ld ",
						inmr_rec.item_no,
						inlo_rec.uom,
						ToLclUom (qchr_rec.origin_qty),
						ToLclUom (qchr_rec.rel_qty),
						ToLclUom (qchr_rec.rej_qty),
						recptDate,
						expRelDate,
						inlo_rec.lot_no,
						DateToString (inlo_rec.expiry_date),
						qchr_rec.ref_1,
						qchr_rec.source_type,
						qchr_rec.hhqc_hash);
			}
		}
		/*
		 * There is more than one record so display in tab screen.
		 */
		if (flag)
		{
			if (recCount > 1)
				break;
		}

		cc = find_rec (qchr, &qchr_rec, NEXT, "r");
	}

	return (recCount);
}

int
CheckStore (
 int	flag)
{
	int		cnt;

	for (cnt = 0; cnt < lcount [SCN_DETAILS]; cnt ++)
	{
		if (flag)
		{
			if (store [cnt].hhqcHash == qchr_rec.hhqc_hash &&
				cnt != line_cnt)
				return (FALSE);
		}
		else
		{
			if (store [cnt].hhqcHash	== qcln_rec.hhqc_hash &&
				store [cnt].releaseDate	== qcln_rec.release_dt &&
				store [cnt].lineNo		== qcln_rec.line_no &&
				cnt != line_cnt)
				return (FALSE);
		}
	}

	return (TRUE);
}

int
GetLineDetails (void)
{
	int		flag = FALSE;
	char	relDate [11];
	int		TempLine;

	tab_open ("qclnTab", lineKeys, 11, 1, 5, FALSE);
	tab_add ("qclnTab",
			"# %-10.10s %-4.4s %-8.8s %-11.11s %-11.11s %-7.7s %-7.7s %-10.10s ",
			"Rel. Date ",		"Line",
			"Emp Code",		"  Rel Qty  ",
			"  Rej Qty  ",	"Lot  No",
			"Slot No",		"ExpiryDate");

	qcln_rec.hhqc_hash = qchr_rec.hhqc_hash;
	qcln_rec.release_dt = 0L;
	qcln_rec.line_no = 0;
	cc = find_rec (qcln, &qcln_rec, GTEQ, "r");
	while (!cc &&
		qcln_rec.hhqc_hash == qchr_rec.hhqc_hash)
	{
		if (!strcmp (qcln_rec.emp_code, prmr_rec.code))
		{
			if (CheckStore (FALSE))
			{
				if (!flag)
					flag = TRUE;
	
				strcpy (relDate, DateToString (qcln_rec.release_dt));
				inlo_rec.inlo_hash	=	qcln_rec.inlo_hash;
				cc = find_rec (inlo, &inlo_rec, COMPARISON, "r");
				if (cc)
					memset (&inlo_rec, 0, sizeof (inlo_rec));

				tab_add ("qclnTab",
						" %-10.10s  %3d %-8.8s %11.3f %11.3f %-7.7s %-7.7s %-10.10s ",
						relDate,
						qcln_rec.line_no,
						qcln_rec.emp_code,
						ToLclUom (qcln_rec.rel_qty),
						ToLclUom (qcln_rec.rej_qty),
						inlo_rec.lot_no,
						inlo_rec.slot_no,
						DateToString (inlo_rec.expiry_date));
			}
		}

		cc = find_rec (qcln, &qcln_rec, NEXT, "r");
	}
	if (!flag)
	{
		tab_add ("qclnTab", ML ("No Lines Found For This Item"));
		tab_display ("qclnTab", TRUE);
		sleep (sleepTime);
	}
	else
		tab_scan ("qclnTab");

	tab_close ("qclnTab", TRUE);
	if (restart)
		return (TRUE);

	TempLine	=	lcount [SCN_DETAILS];

	/*-----------------
	| Redraw screens. |
	-----------------*/
	putval (line_cnt);

	lcount [SCN_DETAILS] = (line_cnt + 1 > lcount [SCN_DETAILS]) 
							? line_cnt + 1 : lcount [SCN_DETAILS];
	scn_write (SCN_DETAILS);
	scn_display (SCN_DETAILS);
	lcount [SCN_DETAILS] = TempLine;
	box (0, 5, 132, 14);
	PrintCoStuff ();
	heading (SCN_DETAILS);
	scn_display (SCN_DETAILS);

	return (flag);
}

static int
DetailFunc (
 int		c,
 KEY_PTR	psUnused)
{
	char	getLine [200];
	long	hhqcHash;

	tab_get ("qchrTab", getLine, CURRENT, 0);

	hhqcHash = atol (getLine + 116);
	if ((cc = find_hash (qchr2, &qchr_rec, EQUAL, "r", hhqcHash)))
		file_err (cc, qchr2, "DBFIND");

	c = FN16;
	return (c);
}

static int
LineFunc (
 int		c,
 KEY_PTR	psUnused)
{
	char	getLine [200];
	char	relDate [11];

	tab_get ("qclnTab", getLine, CURRENT, 0);

	sprintf (relDate, "%-10.10s", getLine + 1);
	qcln_rec.hhqc_hash = qchr_rec.hhqc_hash;
	qcln_rec.release_dt = StringToDate (relDate);
	qcln_rec.line_no = atoi (getLine + 12);
	if ((cc = find_rec (qcln, &qcln_rec, EQUAL, "r")))
		file_err (cc, (char *)qcln, "DBFIND");

	c = FN16;
	return (c);
}

static int
ExitFunc (
 int		c,
 KEY_PTR	psUnused)
{
	if (c == FN1)
		restart = TRUE;

	return (c);
}

/*
 * Checks if the quantity entered by the user valid quantity that can be saved 
 * to a float variable without any problems of losing figures after the 
 * decimal point.  eg. if _dec_pt is 2 then the greatest quantity the user 
 * can enter is 99999.99   
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
		/*
		 * Quantity %14.6f Greater Than Allowed Quantity %f 
		 */
		sprintf (err_str, ML (mlQcMess019), _qty, compare [_dec_pt]);
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
	if (cur_screen != SCN_DETAILS)
		return;

	if (prog_status != ENTRY && lineNo >= lcount [SCN_DETAILS])
		FLD ("itemNo") = YES;
	else
		FLD ("itemNo") = NE;

	if (local_rec.rejQty > 0.00)
	{
		if (RELEASE)
			FLD ("reason")	= YES;
		else
			FLD ("reason")	= NI;
	}
	else
		FLD ("reason")		= NA;

	DisplayDetails (lineNo);
}

void
DisplayDetails (
 int	lineNo)
{
	char	DspExpPrd [4];
	char	DspMoney [3][12];

	if (prog_status != ENTRY && lineNo >= lcount [SCN_DETAILS])
	{
		print_at (6, 2,	ML (mlQcMess020), " ");
		print_at (7, 2,	ML (mlQcMess021), " ");
		print_at (8, 2,	ML (mlQcMess022), " ");
		print_at (9, 2,	ML (mlQcMess023), " ");
		print_at (6, 92, ML (mlQcMess011), " "); 
		print_at (7, 92, ML (mlQcMess012), " ");
		print_at (8, 92, ML (mlQcMess013), " ");
		print_at (9, 92, ML (mlQcMess014), " ");
		sprintf (err_str, ML (mlQcMess017), " ", " ", " ");
		rv_pr (err_str, (132 - strlen (err_str)) / 2, 20, 1);
	}
	else
	{
		print_at (6, 2, ML (mlQcMess020), local_rec.itemDesc);
		print_at (7, 2, ML (mlQcMess021), local_rec.supName);
		print_at (8, 2, ML (mlQcMess022), local_rec.reasonDesc);
		print_at (9, 2, ML (mlQcMess023), local_rec.reasonWofAcc);

		print_at (6, 92, ML (mlQcMess011), local_rec.sourceType);
		print_at (7, 92, ML (mlQcMess012), DateToString (local_rec.recptDt));
		print_at (8, 92, ML (mlQcMess013), DateToString (local_rec.expDt));
		sprintf (DspExpPrd, "%03d", local_rec.expiryPrd);
		print_at (9, 92, ML (mlQcMess014), DspExpPrd);

		sprintf (DspMoney [0], "%11.3f",  local_rec.originQty);
		sprintf (DspMoney [1], "%11.3f",  local_rec.orgRelQty);
		sprintf (DspMoney [2], "%11.3f",  local_rec.orgRejQty);
		sprintf (err_str, ML (mlQcMess017),DspMoney [0],DspMoney [1], DspMoney [2]);
		rv_pr (err_str, (132 - strlen (err_str)) / 2, 20, 1);
	}
}

/*
 * Update pcat records from TAB screen 
 */
void
Update (void)
{
	float	prevRelQty	=	0.00,
			prevRejQty	=	0.00;

	int		monthPeriod;

	abc_selfield (inmr, "inmr_hhbr_hash");
	abc_selfield (incc, "incc_hhwh_hash");
	abc_selfield (glmr, "glmr_id_no");

	/*
	 * Update qchr records. 
	 */
	scn_set (SCN_DETAILS);
	for (line_cnt = 0; line_cnt < lcount [SCN_DETAILS]; line_cnt++)
	{
		getval (line_cnt);

		if (RELEASE && local_rec.relQty == 0.00 && local_rec.rejQty == 0.00)
			continue;

		qchr_rec.hhqc_hash	=	local_rec.hhqcHash;
		cc = find_rec (qchr2, &qchr_rec, EQUAL, "u");
		if (cc)
			continue;
		
		inmr_rec.hhbr_hash	=	qchr_rec.hhbr_hash;
		cc = find_rec (inmr, &inmr_rec, EQUAL, "u");
		if (cc)
			file_err (cc, (char *)inmr, "DBFIND");
			
		incc_rec.hhwh_hash	=	SR.hhwhHash;
		cc = find_rec (incc, &incc_rec, EQUAL, "u");
		if (cc)
			file_err (cc, (char *)incc, "DBFIND");

		/*
		 * Read line details record if Not RELEASE. 
		 */
		if (!RELEASE)
		{
			qcln_rec.hhqc_hash	= qchr_rec.hhqc_hash;
			qcln_rec.release_dt	= SR.releaseDate;
			qcln_rec.line_no	= SR.lineNo;
			if (find_rec (qcln, &qcln_rec, EQUAL, "u"))
				continue;
		}

		if (MULT_LOC || SK_BATCH_CONT)
		{
			/*
			 * Update location details if this is a new    
			 * release or details have been modified.     
			 */
			if (RELEASE ||
				(!RELEASE &&
				(local_rec.relQty 	!= ToLclUom (qcln_rec.rel_qty) ||
				 local_rec.rejQty 	!= ToLclUom (qcln_rec.rej_qty) ||	
				 qchr_rec.inlo_hash != qcln_rec.inlo_hash)))
			{
				/*
				 * Reverse old transactions.                
				 */
				if (!RELEASE)
				{
					/*
					 * Increase original location, by total     
					 * previous release total (including rej.) 
					 */
					inlo_rec.inlo_hash	=	qchr_rec.inlo_hash;
					cc = find_rec (inlo, &inlo_rec, COMPARISON, "u");
					if (cc)
						file_err (cc, (char *)inlo, "DBFIND");

					inlo_rec.qty += (qcln_rec.rel_qty + qcln_rec.rej_qty);

					cc = abc_update (inlo, &inlo_rec);
					if (cc)
						file_err (cc, (char *)inlo, "DBUPDATE");

					inlo_rec.inlo_hash	=	qcln_rec.inlo_hash;
					cc = find_rec (inlo, &inlo_rec, COMPARISON, "u");
					if (cc)
						file_err (cc, (char *)inlo, "DBFIND");

					inlo_rec.qty -= qcln_rec.rel_qty;

					cc = abc_update (inlo, &inlo_rec);
					if (cc)
						file_err (cc, (char *)inlo, "DBUPDATE");
				}
				/*
				 * Reduce original location by total released 
				 * quantity (rejected included).             
				 */
				inlo_rec.inlo_hash	=	qchr_rec.inlo_hash;
				cc = find_rec (inlo, &inlo_rec, COMPARISON, "u");
				if (cc)
					file_err (cc, (char *)inlo, "DBFIND");
				
				inlo_rec.qty -= ToStdUom (local_rec.relQty);
				inlo_rec.qty -= ToStdUom (local_rec.rejQty);

				if ((cc = abc_update (inlo, &inlo_rec)))
					file_err (cc, (char *)inlo, "DBUPDATE");

				/*
				| Increase new location by release qty.    |
				------------------------------------------*/
				inlo_rec.inlo_hash	=	GetINLO (line_cnt, 0);
				cc = find_rec (inlo, &inlo_rec, COMPARISON, "u");
				if (cc)
					file_err (cc, (char *)inlo, "DBFIND");

				inlo_rec.qty += ToStdUom (local_rec.relQty);

				if ((cc = abc_update (inlo, &inlo_rec)))
					file_err (cc, (char *)inlo, "DBUPDATE");
			}

			/*------------------------------------------
			| Update WOF G/L account if required.      |
			------------------------------------------*/
			if ((RELEASE && local_rec.rejQty > 0.00) ||
				(!RELEASE &&
				(qcln_rec.rej_qty > 0.00 ||
				local_rec.rejQty > 0.00)))
			{
				/*---------------------------------------------
				| Update G/L details if this is a new release |
				| or details have been modified.              |
				---------------------------------------------*/
				if (RELEASE ||
					(!RELEASE &&
					(local_rec.rejQty != qcln_rec.rej_qty ||
					strcmp (local_rec.reason, qcln_rec.reason))))
				{
					/*------------------------------------------
					| Reverse the old account posting, if need |
					| be.                                      |
					------------------------------------------*/
					if (!RELEASE && qcln_rec.rej_qty > 0.00)
					{
						double	value =
							out_cost (qcln_rec.cost, inmr_rec.outer_size);

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
							SR.hhumHash,
							local_rec.relDate,
							11,
							(SK_BATCH_CONT) ? GetLotNo (line_cnt,0)
											: location,
							inmr_rec.inmr_class,
							inmr_rec.category,
							location,
							"WOF RTN",
							0 - qcln_rec.rej_qty,
							0.00,
							CENTS (value)
						);

						strcpy (glmrRec.co_no,	comm_rec.co_no);
						strcpy (glmrRec.acc_no,	qcln_rec.wof_acc);
						if ((cc = find_rec (glmr, &glmrRec, EQUAL, "r")))
							file_err (cc, glmr, "DBFIND");

						strcpy (glwkRec.acc_no,			glmrRec.acc_no);
						strcpy (glwkRec.co_no,			comm_rec.co_no);
						glwkRec.hhgl_hash				= glmrRec.hhmr_hash;
						strcpy (glwkRec.tran_type,		"12");
						sprintf (glwkRec.sys_ref,"%010ld",(long) comm_rec.term);
						glwkRec.tran_date				= local_rec.relDate;
						DateToDMY (comm_rec.inv_date, NULL,&monthPeriod,NULL);
						sprintf (glwkRec.period_no,	"%02d",	monthPeriod);
						glwkRec.post_date				= local_rec.relDate;
						sprintf (glwkRec.narrative,	"Return W/H : %s", comm_rec.cc_no);
						sprintf (glwkRec.alt_desc1, "%-20.20s", qcln_rec.remarks);
						sprintf (glwkRec.alt_desc2, "%20.20s", " ");
						sprintf (glwkRec.alt_desc3, "%20.20s", " ");
						sprintf (glwkRec.batch_no,  "%10.10s", " ");
	 					sprintf 
						(
							glwkRec.user_ref, 
							"%-4.4s - %-8.8s", 
							qcmr_rec.centre, 
							prmr_rec.code
						);
						glwkRec.amount	= CENTS (qcln_rec.rej_qty * value);
						strcpy (glwkRec.jnl_type,		"2");
						strcpy (glwkRec.stat_flag,		"2");
						glwkRec.loc_amount = glwkRec.amount;
						glwkRec.exch_rate = 1.00;
						strcpy (glwkRec.currency, local_rec.loc_curr);
						GL_AddBatch ();

						strcpy (glmrRec.co_no,		comm_rec.co_no);
						strcpy (glmrRec.acc_no,		qcln_rec.stk_wof_acc);
						if ((cc = find_rec (glmr, &glmrRec, EQUAL, "r")))
							file_err (cc, glmr, "DBFIND");

						strcpy (glwkRec.acc_no,glmrRec.acc_no);
						glwkRec.hhgl_hash				= glmrRec.hhmr_hash;
						strcpy (glwkRec.jnl_type,		"1");
						GL_AddBatch ();
					}

					if (local_rec.rejQty > 0.00)
					{
						float	WkQty	=	0.00;
						double	value =
								out_cost (local_rec.cost, inmr_rec.outer_size);
						WkQty	=	ToStdUom (local_rec.rejQty);

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
							SR.hhumHash,
							local_rec.relDate,
							11,
							(SK_BATCH_CONT) ? GetLotNo (line_cnt,0)
											: location,
							inmr_rec.inmr_class,
							inmr_rec.category,
							location,
							"WOF ISS",
							WkQty,
							0.00,
							CENTS (value)
						);

						strcpy (glmrRec.co_no,		comm_rec.co_no);
						strcpy (glmrRec.acc_no,		local_rec.reasonWofAcc);
						if ((cc = find_rec (glmr, &glmrRec, EQUAL, "r")))
							file_err (cc, glmr, "DBFIND");

						strcpy (glwkRec.acc_no,	glmrRec.acc_no);
						strcpy (glwkRec.co_no,			comm_rec.co_no);
						glwkRec.hhgl_hash				= glmrRec.hhmr_hash;
						strcpy (glwkRec.tran_type,		"12");
						sprintf (glwkRec.sys_ref,"%010ld",(long) comm_rec.term);
						glwkRec.tran_date				= local_rec.relDate;

						DateToDMY (comm_rec.inv_date, NULL,&monthPeriod,NULL);
						sprintf (glwkRec.period_no,	"%02d",	monthPeriod);
						glwkRec.post_date				= local_rec.relDate;
						sprintf (glwkRec.narrative,	"Issue W/H : %s", comm_rec.cc_no);
						sprintf (glwkRec.alt_desc1, "%-20.20s", qcln_rec.remarks);
						sprintf (glwkRec.alt_desc2, "%20.20s", " ");
						sprintf (glwkRec.alt_desc3, "%20.20s", " ");
						sprintf (glwkRec.batch_no,  "%10.10s", " ");
	 					sprintf 
						(
							glwkRec.user_ref, 
							"%-4.4s - %-8.8s", 
							qcmr_rec.centre, 
							prmr_rec.code
						);
						glwkRec.amount					= CENTS (qcln_rec.rej_qty * value);
						glwkRec.amount	= CENTS (WkQty * value);
						strcpy (glwkRec.jnl_type,		"1");
						strcpy (glwkRec.stat_flag,		"2");
						glwkRec.loc_amount = glwkRec.amount;
						glwkRec.exch_rate = 1.00;
						strcpy (glwkRec.currency, local_rec.loc_curr);
						GL_AddBatch ();

						strcpy (glmrRec.co_no,	comm_rec.co_no);
						strcpy (glmrRec.acc_no,	local_rec.stkWofAcc);
						if ((cc = find_rec (glmr, &glmrRec, EQUAL, "r")))
							file_err (cc, glmr, "DBFIND");

						strcpy (glwkRec.acc_no, glmrRec.acc_no);
						glwkRec.hhgl_hash		= glmrRec.hhmr_hash;
						strcpy (glwkRec.jnl_type,		"2");
						GL_AddBatch ();
					}
				}
			}
			else
				strcpy (local_rec.stkWofAcc,	" ");

			/*-----------------------------------
			| Update serial item record (insf). |
			-----------------------------------*/
			if (inmr_rec.serial_item [0] == 'Y')
			{
				insfRec.hhwh_hash = SR.hhwhHash;
				strcpy (insfRec.status, "C"); /* COMMITTED */
				if (!RELEASE)
				{
					if (qcln_rec.rej_qty > 0.00)
						strcpy (insfRec.status, "S"); /* SOLD */
					else
						strcpy (insfRec.status, "F"); /* FREE */
				}
				strcpy (insfRec.serial_no, qchr_rec.serial_no);
				if (!find_rec (insf, &insfRec, EQUAL, "u"))
				{
					if (local_rec.rejQty > 0.00)
						strcpy (insfRec.status, "S"); /* SOLD */
					else
						strcpy (insfRec.status, "F"); /* FREE */

					if (!RELEASE &&
						local_rec.relQty == 0.00 &&
						local_rec.rejQty == 0.00)
						strcpy (insfRec.status, "C"); /* COMMITTED */

					if ((cc = abc_update (insf, &insfRec)))
						file_err (cc, (char *)insf, "DBUPDATE");
				}
			}

			/*------------------------------------------
			| Add or Update qcln record.               |
			------------------------------------------*/
			if (!RELEASE)
			{
				prevRelQty					= qcln_rec.rel_qty;
				prevRejQty					= qcln_rec.rej_qty;
			}

			strcpy (qcln_rec.emp_code,		prmr_rec.code);
			qcln_rec.rel_qty				= ToStdUom (local_rec.relQty);
			qcln_rec.rej_qty				= ToStdUom (local_rec.rejQty);
			strcpy (qcln_rec.wof_acc,		local_rec.reasonWofAcc);
			strcpy (qcln_rec.stk_wof_acc,	local_rec.stkWofAcc);
			qcln_rec.cost					= local_rec.cost;
			strcpy (qcln_rec.reason,		local_rec.reason);

			qcln_rec.inlo_hash				=	GetINLO (line_cnt, 0);
			strcpy (qcln_rec.remarks,		local_rec.remarks);
			strcpy (qcln_rec.coa,			local_rec.coa);
			qcln_rec.reassay_date			= local_rec.reassayDt;
			if (RELEASE)
			{
				qcln_rec.hhqc_hash			= qchr_rec.hhqc_hash;
				qcln_rec.release_dt			= local_rec.relDate;
				qcln_rec.line_no			= qchr_rec.next_seq;

				if ((cc = abc_add (qcln, &qcln_rec)))
					file_err (cc, (char *)qcln, "DBADD");
			}
			else
			{
				if ((cc = abc_update (qcln, &qcln_rec)))
					file_err (cc, (char *)qcln, "DBUPDATE");
			}

			/*------------------------------------------
			| Update qchr record.                      |
			------------------------------------------*/
			if (RELEASE)
				qchr_rec.next_seq	++;
			else
			{
				qchr_rec.rel_qty	-= prevRelQty;
				qchr_rec.rej_qty	-= prevRejQty;
			}
			qchr_rec.rel_qty		+= ToStdUom (local_rec.relQty);
			qchr_rec.rej_qty		+= ToStdUom (local_rec.rejQty);

			if ((cc = abc_update (qchr2, &qchr_rec)))
				file_err (cc, qchr2, "DBUPDATE");

			/*------------------------------------------
			| Update inmr and incc qc_qty.             |
			| Less what has been already been released.|
			------------------------------------------*/
			if (!RELEASE)
			{
				inmr_rec.qc_qty	+= prevRelQty;
				inmr_rec.qc_qty	+= prevRejQty;
				incc_rec.qc_qty		+= prevRelQty;
				incc_rec.qc_qty		+= prevRejQty;
			}
			inmr_rec.qc_qty		-= qcln_rec.rel_qty;
			inmr_rec.qc_qty		-= qcln_rec.rej_qty;
			incc_rec.qc_qty			-= qcln_rec.rel_qty;
			incc_rec.qc_qty			-= qcln_rec.rej_qty;

			if (prevRejQty > 0.00)
			{
				inmr_rec.on_hand	+= prevRejQty;
				incc_rec.issues		-= prevRejQty;
				incc_rec.ytd_issues	-= prevRejQty;
			}

			if (qcln_rec.rej_qty > 0.00)
			{
				inmr_rec.on_hand	-= qcln_rec.rej_qty;
				incc_rec.issues		+= qcln_rec.rej_qty;
				incc_rec.ytd_issues	+= qcln_rec.rej_qty;
			}

			incc_rec.closing_stock	= 	incc_rec.opening_stock +
										incc_rec.pur +
										incc_rec.receipts +
										incc_rec.adj -
										incc_rec.issues -
										incc_rec.sales;

			if ((cc = abc_update (inmr, &inmr_rec)))
				file_err (cc, (char *)inmr, "DBUPDATE");

			if ((cc = abc_update (incc, &incc_rec)))
				file_err (cc, (char *)incc, "DBUPDATE");

			open_rec (inwu, inwu_list, INWU_NO_FIELDS, "inwu_id_no");

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
					file_err (cc, (char *)inwu, "DBADD");

				inwu_rec.hhwh_hash	=	incc_rec.hhwh_hash;
				inwu_rec.hhum_hash	=	inmr_rec.std_uom;
				cc = find_rec (inwu, &inwu_rec, COMPARISON,"u");
				if (cc)
					file_err (cc, (char *)inwu, "DBFIND");
			}
			if (prevRejQty > 0.00)
				inwu_rec.issues		-= prevRejQty;

			if (qcln_rec.rej_qty > 0.00)
				inwu_rec.issues		+= qcln_rec.rej_qty;

			inwu_rec.closing_stock = inwu_rec.opening_stock +
									 inwu_rec.pur +
									 inwu_rec.receipts +
									 inwu_rec.adj -
									 inwu_rec.issues -
									 inwu_rec.sales;

			cc = abc_update (inwu,&inwu_rec);
			if (cc)
				file_err (cc, (char *)inwu, "DBUPDATE");

			abc_fclose (inwu);
			PrintDetails ();
		}
	}

	abc_selfield (inmr, "inmr_id_no");
	abc_selfield (incc, "incc_id_no");
	abc_selfield (glmr, "glmr_hhmr_hash");
}

/*
 * Search for Employee 
 */
void
SrchPrmr (
 char *	keyValue)
{
	_work_open (8,0,40);
	save_rec ("#Code", "#Name");

	strcpy (prmr_rec.co_no, comm_rec.co_no);
	strcpy (prmr_rec.br_no, comm_rec.est_no);
	sprintf (prmr_rec.code, "%-8.8s", keyValue);
	cc = find_rec (prmr, &prmr_rec, GTEQ, "r");
	while (!cc &&
		!strcmp (prmr_rec.co_no, comm_rec.co_no) &&
		!strcmp (prmr_rec.br_no, comm_rec.est_no) &&
		!strncmp (prmr_rec.code, keyValue, strlen (keyValue)))
	{
		if (save_rec (prmr_rec.code, prmr_rec.name))
			break;

		cc = find_rec (prmr, &prmr_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (prmr_rec.co_no, comm_rec.co_no);
	strcpy (prmr_rec.br_no, comm_rec.est_no);
	sprintf (prmr_rec.code, "%-8.8s", temp_str);
	if ((cc = find_rec (prmr, &prmr_rec, COMPARISON, "r")))
		file_err (cc, (char *)prmr, "DBFIND");
}

/*
 * Search for QC Centre 
 */
void
SrchQcmr (
	char	*keyValue)
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
 * Search for Reason 
 */
void
SrchExwo (
 char *	keyValue)
{
	_work_open (2,0,40);
	save_rec ("#No", "#Description");

	strcpy (exwo_rec.co_no, comm_rec.co_no);
	sprintf (exwo_rec.code, "%-2.2s", keyValue);
	cc = find_rec (exwo, &exwo_rec, GTEQ, "r");
	while (!cc &&
		!strcmp (exwo_rec.co_no, comm_rec.co_no) &&
		!strncmp (exwo_rec.code, keyValue, strlen (keyValue)))
	{
		if (save_rec (exwo_rec.code, exwo_rec.description))
			break;

		cc = find_rec (exwo, &exwo_rec, NEXT, "r");
	}
	cc = disp_srch ();
	work_close ();
	if (cc)
		return;

	strcpy (exwo_rec.co_no, comm_rec.co_no);
	sprintf (exwo_rec.code, "%-2.2s", temp_str);
	if ((cc = find_rec (exwo, &exwo_rec, COMPARISON, "r")))
		file_err (cc, (char *)exwo, "DBFIND");
}

/*
 * Search on UOM 
 */
void
SrchInum (
 char *	keyValue)
{
	_work_open (4,0,40);
	save_rec ("#UOM","#Description");

	strcpy (inum2_rec.uom_group, inum_rec.uom_group);
	strcpy (inum2_rec.uom, keyValue);
	cc = find_rec (inum2, &inum2_rec, GTEQ, "r");
	while (!cc &&
		   !strcmp (inum2_rec.uom_group, inum_rec.uom_group))
	{
		if (strncmp (inum2_rec.uom, keyValue, strlen (keyValue)))
		{
			cc = find_rec (inum2, &inum2_rec, NEXT, "r");
			continue;
		}

		cc = save_rec (inum2_rec.uom, inum2_rec.desc);
		if (cc)
			break;

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

void
PrintHeading (void)
{

	fprintf (fout, ".START%s<%s>\n", DateToString (TodaysDate ()), PNAME);
	fprintf (fout, ".SO\n");
	fprintf (fout, ".LP%d\n", local_rec.printerNumber);
	fprintf (fout, ".9\n");
	fprintf (fout, ".PI12\n");
	fprintf (fout, ".L158\n");
	if (RELEASE)
		fprintf (fout, ".E QC RELEASE AUDIT TRIAL \n");
	else
		fprintf (fout, ".E QC MAINTAIN RELEASE RECORDS AUDIT TRIAL \n");
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
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "========");
	fprintf (fout, "========");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "=====\n");

	fprintf (fout, "===========");
	fprintf (fout, "=====");
	fprintf (fout, "=================");
	fprintf (fout, "=========================================");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "========");
	fprintf (fout, "========");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "===========");
	fprintf (fout, "=====\n");

	fprintf (fout, "| RELEASE  ");
	fprintf (fout, "| QC ");
	fprintf (fout, "|  ITEM  NUMBER  ");
	fprintf (fout, "|            ITEM DESCRIPTION            ");
	fprintf (fout, "| RECEIPT  ");
	fprintf (fout, "| EXPT REL ");
	fprintf (fout, "|LOT  NO");
	fprintf (fout, "|SLOT NO");
	fprintf (fout, "|  EXPIRY  ");
	fprintf (fout, "| LOCATION ");
	fprintf (fout, "|NEXT R DT.");
	fprintf (fout, "|COA");
	fprintf (fout, "|\n");
}

void
PrintDetails (void)
{
	fprintf (fout, "|----------");
	fprintf (fout, "-----");
	fprintf (fout, "-----------------");
	fprintf (fout, "-----------------------------------------");
	fprintf (fout, "-----------");
	fprintf (fout, "-----------");
	fprintf (fout, "--------");
	fprintf (fout, "--------");
	fprintf (fout, "-----------");
	fprintf (fout, "-----------");
	fprintf (fout, "-----------");
	fprintf (fout, "----");
	fprintf (fout, "|\n");

	fprintf (fout, "|%-10.10s",		DateToString (qcln_rec.release_dt));
	fprintf (fout, "|%-4.4s",		qchr_rec.qc_centre);
	fprintf (fout, "|%-16.16s",		inmr_rec.item_no);
	fprintf (fout, "|%-40.40s",		local_rec.itemDesc);
	fprintf (fout, "|%-10.10s",		DateToString (qchr_rec.receipt_dt));
	fprintf (fout, "|%-10.10s",		DateToString (qchr_rec.exp_rel_dt));
	fprintf (fout, "|%-7.7s",		GetLotNo (line_cnt, 0));
	fprintf (fout, "|%-7.7s",		GetSLotNo (line_cnt, 0));
	fprintf (fout, "|%-10.10s",		GetExpiry (line_cnt, 0));
	fprintf (fout, "|%-10.10s",		GetLoc (line_cnt, 0));
	fprintf (fout, "|%-10.10s",		DateToString (qcln_rec.reassay_date));
	fprintf (fout, "| %-1.1s ",		qcln_rec.coa);
	fprintf (fout, "|\n");

	fprintf (fout, "| SOURCE TYPE : %14.14s ",	local_rec.sourceType);
	fprintf (fout, "EMPLOYEE : %-40.40s ",		prmr_rec.name);
	fprintf (fout, "SUPPLIER : %-54.54s ",	local_rec.supName);
	fprintf (fout, "|\n");

	fprintf (fout, "| UOM : %4.4s   ORIGINAL : %11.3f ",	
									SR.uom,
									ToLclUom (qchr_rec.origin_qty));
	fprintf (fout, "RELEASED : %11.3f ",	ToLclUom (qcln_rec.rel_qty));
	fprintf (fout, "REJECTED : %11.3f ",	ToLclUom (qcln_rec.rej_qty));
	fprintf (fout, "REMARKS : %-30.30s ",	qcln_rec.remarks);
	fprintf (fout, "              ");
	fprintf (fout, "          |\n");

	if (local_rec.rejQty > 0.00)
	{
		double	value = qcln_rec.cost * qcln_rec.rej_qty;

		fprintf (fout, "| REASON : %-2.2s %-20.20s ",
				qcln_rec.reason, local_rec.reasonDesc);
		fprintf (fout, "DBT :%-16.16s ",	qcln_rec.wof_acc);
		fprintf (fout, "CRD :%-16.16s  ",	qcln_rec.stk_wof_acc);
		fprintf (fout, "COST/UNIT : %10.2f ",	DOLLARS (qcln_rec.cost));
		fprintf (fout, "TOTAL COST : %10.2f  ",	DOLLARS (value));
		fprintf (fout, "           ");
		fprintf (fout, "          |\n");
	}
}

float	
ToStdUom (
	float	lclQty)
{
	float	cnvQty;

	if (F_HIDE (label ("UOM")))
		return (lclQty);

	if (SR.cnvFct == 0.00 || lclQty == 0.00)
		return (0.00);

	cnvQty = lclQty * SR.cnvFct;

	return (cnvQty);
}

float	
ToLclUom (
	float	lclQty)
{
	float	cnvQty;

	if (F_HIDE (label ("UOM")))
		return (lclQty);

	if (SR.cnvFct == 0.00 || lclQty == 0.00)
		return (0.00);

	cnvQty = lclQty / SR.cnvFct;

	return (cnvQty);
}
